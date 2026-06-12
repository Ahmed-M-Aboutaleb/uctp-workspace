#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <thread>
#include <mutex>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <filesystem>

#include "problem.h"
#include "individual.h"
#include "hgats.h"
#include "gsga.h"
#include "local_search.h"

using namespace std;
namespace fs = filesystem;

// ═══════════════════════════════════════════════════════════════════════
// STRUCTURES
// ═══════════════════════════════════════════════════════════════════════

struct Result {
    string name;
    int df;
    int scp;
    double time;
    int fitness;
};

struct InstanceStats {
    string name;
    int numRuns;
    
    int bestDf, worstDf;
    double avgDf;
    double stdDevDf;
    
    int bestScp, worstScp;
    double avgScp;
    double stdDevScp;
    
    int bestFitness, worstFitness;
    double avgFitness;
    
    double avgTime, totalTime;
    
    Individual bestIndividual;
    Problem bestProblem;
    
    vector<Result> allResults;
};

// ═══════════════════════════════════════════════════════════════════════
// GLOBAL THREAD-SAFE STRUCTURES
// ═══════════════════════════════════════════════════════════════════════

mutex resultMutex;
mutex consoleMutex;
vector<InstanceStats> globalStats;
string resultsDir = "results";

// ═══════════════════════════════════════════════════════════════════════
// DIRECTORY SETUP
// ═══════════════════════════════════════════════════════════════════════

void setupResultsDirectory() {
    try {
        if (!fs::exists(resultsDir)) {
            fs::create_directory(resultsDir);
            cout << "[CREATED] " << resultsDir << "\n";
        }

        fs::create_directory(resultsDir + "/timetables");
        fs::create_directory(resultsDir + "/statistics");
        fs::create_directory(resultsDir + "/best_timetables");
        
        // تحقق من الكتابة
        string testFile = resultsDir + "/.test";
        ofstream test(testFile);
        if (!test.is_open()) {
            cerr << "[FATAL] Cannot write to " << resultsDir << "\n";
            exit(1);
        }
        test.close();
        fs::remove(testFile);
        
        cout << "[OK] All directories ready and writable\n\n";
    } catch (const exception& e) {
        cerr << "[FATAL] " << e.what() << "\n";
        exit(1);
    }
}

// ═══════════════════════════════════════════════════════════════════════
// HELPER FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════

HGATS::Params buildParams(int timeLimit) {
    HGATS::Params p;
    p.gsgaParams.populationSize = 50;
    p.gsgaParams.timeLimitSec   = timeLimit;
    p.tsParams.timeLimitSec     = timeLimit;
    return p;
}

Result runInstance(const string& path, int timeLimit, Individual& bestIndividual) {
    Problem prob;
    Result r;

    if (!prob.loadFromFile(path)) {
        r.df = -1;
        r.scp = -1;
        r.time = 0;
        return r;
    }

    auto start = chrono::steady_clock::now();

    HGATS solver(prob, buildParams(timeLimit));
    Individual best = solver.solve();

    auto end = chrono::steady_clock::now();

    r.name = prob.name;
    r.df   = best.distanceToFeasibility;
    r.scp  = best.softConstraintPenalty;
    r.time = chrono::duration<double>(end - start).count();
    r.fitness = r.df * 1000000 + r.scp;

    bestIndividual = best;

    return r;
}

double calculateStdDev(const vector<double>& values, double mean) {
    if (values.size() <= 1) return 0.0;
    
    double sumSquaredDiff = 0.0;
    for (double v : values) {
        sumSquaredDiff += (v - mean) * (v - mean);
    }
    return sqrt(sumSquaredDiff / (values.size() - 1));
}

InstanceStats computeStats(const string& instanceName,
                           const vector<Result>& results,
                           const Individual& bestIndividual,
                           const Problem& problem) {
    InstanceStats stats;
    stats.name = instanceName;
    stats.numRuns = results.size();
    stats.allResults = results;
    stats.bestIndividual = bestIndividual;
    stats.bestProblem = problem;

    vector<int> dfs, scps, fitnesses;
    vector<double> times;

    for (const auto& r : results) {
        dfs.push_back(r.df);
        scps.push_back(r.scp);
        fitnesses.push_back(r.fitness);
        times.push_back(r.time);
    }

    stats.bestDf = *min_element(dfs.begin(), dfs.end());
    stats.worstDf = *max_element(dfs.begin(), dfs.end());
    stats.avgDf = accumulate(dfs.begin(), dfs.end(), 0.0) / dfs.size();
    
    vector<double> dfDouble(dfs.begin(), dfs.end());
    stats.stdDevDf = calculateStdDev(dfDouble, stats.avgDf);

    stats.bestScp = *min_element(scps.begin(), scps.end());
    stats.worstScp = *max_element(scps.begin(), scps.end());
    stats.avgScp = accumulate(scps.begin(), scps.end(), 0.0) / scps.size();
    
    vector<double> scpDouble(scps.begin(), scps.end());
    stats.stdDevScp = calculateStdDev(scpDouble, stats.avgScp);

    stats.bestFitness = *min_element(fitnesses.begin(), fitnesses.end());
    stats.worstFitness = *max_element(fitnesses.begin(), fitnesses.end());
    stats.avgFitness = accumulate(fitnesses.begin(), fitnesses.end(), 0.0) / fitnesses.size();

    stats.totalTime = accumulate(times.begin(), times.end(), 0.0);
    stats.avgTime = stats.totalTime / times.size();

    return stats;
}

// ═══════════════════════════════════════════════════════════════════════
// TIMETABLE EXPORT FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════

void saveTimetableCSV(const Problem& problem, const Individual& individual,
                      const string& instanceName) {
    try {
        string filename = resultsDir + "/timetables/" + instanceName + "_timetable.csv";
        ofstream file(filename);
        
        if (!file.is_open()) {
            cerr << "[ERROR] Cannot open " << filename << "\n";
            return;
        }

        file << "Event_ID,Timeslot,Day,Period,Room,Feasible\n";

        bool isFeasible = (individual.distanceToFeasibility == 0);

        for (int e = 0; e < problem.numEvents; ++e) {
            int ts = individual.assignment[e].timeslot;
            int room = individual.assignment[e].room;
            int day = ts / 9;
            int period = ts % 9;

            file << e << "," 
                 << ts << "," 
                 << day << "," 
                 << period << "," 
                 << room << "," 
                 << (isFeasible ? "YES" : "NO") << "\n";
        }

        file.close();
        cout << "[SAVED] " << filename << "\n";
    } catch (const exception& e) {
        cerr << "[ERROR] CSV save: " << e.what() << "\n";
    }
}

void saveTimetableHTML(const Problem& problem, const Individual& individual,
                       const string& instanceName) {
    try {
        string filename = resultsDir + "/timetables/" + instanceName + "_timetable.html";
        ofstream file(filename);
        
        if (!file.is_open()) {
            cerr << "[ERROR] Cannot open " << filename << "\n";
            return;
        }

        bool isFeasible = (individual.distanceToFeasibility == 0);

        file << "<!DOCTYPE html>\n";
        file << "<html lang=\"en\">\n";
        file << "<head>\n";
        file << "  <meta charset=\"UTF-8\">\n";
        file << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
        file << "  <title>Timetable - " << instanceName << "</title>\n";
        file << "  <style>\n";
        file << "    body { font-family: Arial, sans-serif; margin: 20px; }\n";
        file << "    .info { background: #f0f0f0; padding: 15px; margin-bottom: 20px; border-radius: 5px; }\n";
        file << "    .feasible { color: green; font-weight: bold; }\n";
        file << "    .infeasible { color: red; font-weight: bold; }\n";
        file << "    table { border-collapse: collapse; width: 100%; margin: 20px 0; }\n";
        file << "    th, td { border: 1px solid #ddd; padding: 8px; text-align: center; }\n";
        file << "    th { background-color: #4CAF50; color: white; }\n";
        file << "    tr:nth-child(even) { background-color: #f9f9f9; }\n";
        file << "  </style>\n";
        file << "</head>\n";
        file << "<body>\n";
        file << "  <h1>Timetable Schedule</h1>\n";
        file << "  <h2>" << instanceName << "</h2>\n";
        file << "  <div class=\"info\">\n";
        file << "    <p><strong>Status:</strong> <span class=\"" << (isFeasible ? "feasible" : "infeasible") << "\">\n";
        file << (isFeasible ? "FEASIBLE ✓" : "INFEASIBLE ✗") << "</span></p>\n";
        file << "    <p><strong>Hard Violations (Df):</strong> " << individual.distanceToFeasibility << "</p>\n";
        file << "    <p><strong>Soft Constraint Penalty:</strong> " << individual.softConstraintPenalty << "</p>\n";
        file << "    <p><strong>Fitness:</strong> " << (individual.distanceToFeasibility * 1000000 + individual.softConstraintPenalty) << "</p>\n";
        file << "  </div>\n";
        file << "  <table>\n";
        file << "    <tr>\n";
        file << "      <th>Event ID</th>\n";
        file << "      <th>Timeslot</th>\n";
        file << "      <th>Day</th>\n";
        file << "      <th>Period</th>\n";
        file << "      <th>Room</th>\n";
        file << "    </tr>\n";

        for (int e = 0; e < problem.numEvents; ++e) {
            int ts = individual.assignment[e].timeslot;
            int room = individual.assignment[e].room;
            int day = ts / 9;
            int period = ts % 9 + 1;

            file << "    <tr>\n";
            file << "      <td>" << e << "</td>\n";
            file << "      <td>" << ts << "</td>\n";
            file << "      <td>Day " << day + 1 << "</td>\n";
            file << "      <td>Period " << period << "</td>\n";
            file << "      <td>" << room << "</td>\n";
            file << "    </tr>\n";
        }

        file << "  </table>\n";
        file << "</body>\n";
        file << "</html>\n";

        file.close();
        cout << "[SAVED] " << filename << "\n";
    } catch (const exception& e) {
        cerr << "[ERROR] HTML save: " << e.what() << "\n";
    }
}

void saveBestTimetable(const Problem& problem, const Individual& individual,
                       const string& instanceName, const InstanceStats& stats) {
    try {
        // Create best_timetables folder
        fs::create_directory(resultsDir + "/best_timetables");
        
        // Save best timetable CSV
        string csvFilename = resultsDir + "/best_timetables/" + instanceName + "_BEST.csv";
        ofstream csvFile(csvFilename);
        
        if (!csvFile.is_open()) {
            cerr << "[ERROR] Cannot open " << csvFilename << "\n";
            return;
        }

        csvFile << "Event_ID,Timeslot,Day,Period,Room\n";

        for (int e = 0; e < problem.numEvents; ++e) {
            int ts = individual.assignment[e].timeslot;
            int room = individual.assignment[e].room;
            int day = ts / 9;
            int period = ts % 9;

            csvFile << e << "," 
                    << ts << "," 
                    << day << "," 
                    << period << "," 
                    << room << "\n";
        }

        csvFile.close();
        cout << "[BEST] " << csvFilename << "\n";

        // Save best timetable metadata
        string metaFilename = resultsDir + "/best_timetables/" + instanceName + "_METADATA.txt";
        ofstream metaFile(metaFilename);
        
        if (!metaFile.is_open()) {
            cerr << "[ERROR] Cannot open " << metaFilename << "\n";
            return;
        }

        bool isFeasible = (individual.distanceToFeasibility == 0);

        metaFile << "Instance: " << instanceName << "\n";
        metaFile << "======================================\n\n";
        metaFile << "FEASIBILITY\n";
        metaFile << "  Status: " << (isFeasible ? "FEASIBLE" : "INFEASIBLE") << "\n";
        metaFile << "  Hard Violations (Df): " << individual.distanceToFeasibility << "\n";
        metaFile << "  Soft Constraint Penalty: " << individual.softConstraintPenalty << "\n";
        metaFile << "  Fitness: " << (individual.distanceToFeasibility * 1000000 + individual.softConstraintPenalty) << "\n\n";
        
        metaFile << "STATISTICS (20 RUNS)\n";
        metaFile << "  Best Df: " << stats.bestDf << "\n";
        metaFile << "  Avg Df: " << fixed << setprecision(2) << stats.avgDf << "\n";
        metaFile << "  Worst Df: " << stats.worstDf << "\n";
        metaFile << "  StdDev Df: " << fixed << setprecision(2) << stats.stdDevDf << "\n\n";
        
        metaFile << "  Best SCP: " << stats.bestScp << "\n";
        metaFile << "  Avg SCP: " << fixed << setprecision(2) << stats.avgScp << "\n";
        metaFile << "  Worst SCP: " << stats.worstScp << "\n";
        metaFile << "  StdDev SCP: " << fixed << setprecision(2) << stats.stdDevScp << "\n\n";
        
        metaFile << "  Best Fitness: " << stats.bestFitness << "\n";
        metaFile << "  Avg Fitness: " << fixed << setprecision(2) << stats.avgFitness << "\n";
        metaFile << "  Worst Fitness: " << stats.worstFitness << "\n\n";
        
        metaFile << "TIMING\n";
        metaFile << "  Avg Time per Run: " << fixed << setprecision(3) << stats.avgTime << " seconds\n";
        metaFile << "  Total Time: " << fixed << setprecision(1) << stats.totalTime << " seconds\n";

        metaFile.close();
        cout << "[META] " << metaFilename << "\n";

    } catch (const exception& e) {
        cerr << "[ERROR] Best save: " << e.what() << "\n";
    }
}

// ═══════════════════════════════════════════════════════════════════════
// PARALLEL RUNNER
// ═══════════════════════════════════════════════════════════════════════

void runInstanceMultipleTimes(const string& path, 
                              int numRuns,
                              int timeLimit,
                              const string& instanceName) {
    try {
        vector<Result> results;
        Individual bestIndividualOverall;
        int bestFitnessOverall = INT_MAX;

        Problem problemForBest;
        if (!problemForBest.loadFromFile(path)) {
            cerr << "[ERROR] Cannot load " << path << "\n";
            return;
        }

        for (int run = 1; run <= numRuns; ++run) {
            try {
                {
                    lock_guard<mutex> lock(consoleMutex);
                    cout << instanceName << " [" << run << "/" << numRuns << "]\n";
                    cout.flush();
                }
                
                Individual currentBestIndividual;
                Result r = runInstance(path, timeLimit, currentBestIndividual);
                
                if (r.df < 0) {
                    continue;
                }

                results.push_back(r);

                if (r.fitness < bestFitnessOverall) {
                    bestFitnessOverall = r.fitness;
                    bestIndividualOverall = currentBestIndividual;
                }
            } 
            catch (const exception& e) {
                cerr << "[RUN ERROR] " << instanceName << " run " << run << ": " << e.what() << "\n";
                continue;
            }
        }

        if (results.empty()) {
            cerr << "[SKIP] No valid results for " << instanceName << "\n";
            return;
        }

        InstanceStats stats = computeStats(instanceName, results, bestIndividualOverall, problemForBest);

        {
            lock_guard<mutex> lock(consoleMutex);
            cout << "\n[SAVING] Timetables for " << instanceName << "\n";
            cout.flush();
        }

        saveTimetableCSV(problemForBest, bestIndividualOverall, instanceName);
        saveTimetableHTML(problemForBest, bestIndividualOverall, instanceName);
        saveBestTimetable(problemForBest, bestIndividualOverall, instanceName, stats);

        {
            lock_guard<mutex> lock(resultMutex);
            globalStats.push_back(stats);
        }

        {
            lock_guard<mutex> lock(consoleMutex);
            cout << "[OK] " << instanceName << " completed\n";
            cout.flush();
        }
    } 
    catch (const exception& e) {
        cerr << "[INSTANCE ERROR] " << instanceName << ": " << e.what() << "\n";
    }
}

// ═══════════════════════════════════════════════════════════════════════
// OUTPUT FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════

void printSummaryTable() {
    cout << "\n";
    cout << "Instance,Best_Df,Avg_Df,Worst_Df,Best_SCP,Avg_SCP,Best_Fitness,Avg_Time\n";

    for (const auto& stats : globalStats) {
        cout << stats.name << ","
                  << stats.bestDf << ","
                  << fixed << setprecision(2) << stats.avgDf << ","
                  << stats.worstDf << ","
                  << stats.bestScp << ","
                  << fixed << setprecision(2) << stats.avgScp << ","
                  << stats.bestFitness << ","
                  << fixed << setprecision(3) << stats.avgTime
                  << "\n";
    }

    cout << "\n";
}

void saveResultsToCSV(const string& filename) {
    try {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "[ERROR] Cannot open " << filename << "\n";
            return;
        }

        file << "Instance,Best_Df,Avg_Df,Worst_Df,StdDev_Df,"
             << "Best_SCP,Avg_SCP,Worst_SCP,StdDev_SCP,"
             << "Best_Fitness,Avg_Fitness,Worst_Fitness,"
             << "Avg_Time,Total_Time\n";

        for (const auto& stats : globalStats) {
            file << stats.name << ","
                 << stats.bestDf << "," << fixed << setprecision(2) << stats.avgDf << "," << stats.worstDf << "," << stats.stdDevDf << ","
                 << stats.bestScp << "," << fixed << setprecision(2) << stats.avgScp << "," << stats.worstScp << "," << stats.stdDevScp << ","
                 << stats.bestFitness << "," << fixed << setprecision(2) << stats.avgFitness << "," << stats.worstFitness << ","
                 << fixed << setprecision(3) << stats.avgTime << "," << fixed << setprecision(1) << stats.totalTime << "\n";
        }

        file.close();
        cout << "[SAVED] " << filename << "\n";
    } catch (const exception& e) {
        cerr << "[ERROR] CSV save: " << e.what() << "\n";
    }
}

// ═══════════════════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════════════════

int main() {
    setupResultsDirectory();

    cout << "HGATS - ITC-2007 Parallel Solver\n\n";

    cout << "Available instances (1-24)\n";
    cout << "Enter instance numbers (e.g., '1,3,5' or '1-10' or 'all'): ";
    
    string input;
    getline(cin, input);

    vector<int> instNumbers;
    
    if (input == "all" || input == "ALL" || input.empty()) {
        for (int i = 1; i <= 10; ++i)
            instNumbers.push_back(i);
    } 
    else if (input.find('-') != string::npos) {
        size_t dashPos = input.find('-');
        int start = stoi(input.substr(0, dashPos));
        int end = stoi(input.substr(dashPos + 1));
        
        for (int i = start; i <= end; ++i) {
            instNumbers.push_back(i);
        }
    }
    else {
        stringstream ss(input);
        string item;
        while (getline(ss, item, ',')) {
            try {
                instNumbers.push_back(stoi(item));
            } catch (...) {}
        }
    }

    if (instNumbers.empty()) {
        cerr << "No instances selected\n";
        return 1;
    }

    cout << "Time limit per run (seconds) [default 60]: ";
    string t;
    getline(cin, t);
    int timeLimitSec = t.empty() ? 60 : stoi(t);

    cout << "Runs per instance [default 20]: ";
    string r;
    getline(cin, r);
    int numRunsPerInstance = r.empty() ? 20 : stoi(r);

    vector<pair<string, string>> instancePaths;

    for (int num : instNumbers) {
        string fname = "data/comp-2007-2-" + to_string(num) + ".tim";
        ifstream f(fname);
        if (f.good()) {
            string name = "comp-2007-2-" + to_string(num);
            instancePaths.push_back({name, fname});
        }
    }

    if (instancePaths.empty()) {
        cerr << "No .tim files found\n";
        return 1;
    }

    cout << "\nRunning " << instancePaths.size() << " instances, " 
              << numRunsPerInstance << " runs each\n";
    cout << "Using " << thread::hardware_concurrency() << " cores\n\n";

    auto totalStart = chrono::steady_clock::now();

    vector<thread> threads;
    int numCores = thread::hardware_concurrency();

    for (const auto& [name, path] : instancePaths) {
        threads.emplace_back(runInstanceMultipleTimes, path, numRunsPerInstance, timeLimitSec, name);

        if (threads.size() >= numCores) {
            for (auto& th : threads) {
                if (th.joinable()) th.join();
            }
            threads.clear();
        }
    }

    for (auto& th : threads) {
        if (th.joinable()) th.join();
    }

    auto totalEnd = chrono::steady_clock::now();
    double totalTime = chrono::duration<double>(totalEnd - totalStart).count();

    cout << "\n";
    printSummaryTable();
    
    cout << "[SAVING] Statistics...\n";
    saveResultsToCSV(resultsDir + "/statistics/results.csv");

    cout << "\n";
    cout << "Total time: " << fixed << setprecision(1) << totalTime << " seconds\n";
    cout << "Results saved in: " << resultsDir << "/\n";

    return 0;
}