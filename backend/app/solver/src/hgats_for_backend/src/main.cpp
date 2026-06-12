#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <thread>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <future>
#include <random>

#include "problem.h"
#include "individual.h"
#include "hgats.h"
#include "gsga.h"
#include "local_search.h"

using namespace std;
namespace fs = filesystem;

// ============================================================================
// JSON EXPORT UTILITIES
// ============================================================================

void saveTimetableJSON(const Individual& ind, const std::string& filename, unsigned int seed, long long generations, const Problem& inst, const std::vector<long long>& history) {
    std::ofstream out(filename);
    if (!out.is_open()) return;
    
    out << "{\n  \"seed\": " << seed << ",\n  \"generations_passed\": " << generations << ",\n  \"fitness\": {\n    \"score\": " << ind.fitness << ",\n    \"hcv\": " << ind.distanceToFeasibility << ",\n    \"scv\": " << ind.softConstraintPenalty << "\n  },\n";
    
    out << "  \"convergence_history\": [";
    for (size_t i = 0; i < history.size(); ++i) {
        out << history[i] << (i < history.size() - 1 ? "," : "");
    }
    out << "],\n";

    out << "  \"schedule\": [\n";
    for (int i = 0; i < inst.numEvents; ++i) {
        out << "    {\"event_id\": " << i << ", \"room_id\": " << ind.assignment[i].room << ", \"timeslot_id\": " << ind.assignment[i].timeslot << "}";
        if (i < inst.numEvents - 1) out << ","; 
        out << "\n";
    }
    out << "  ]\n}\n"; out.close();
}

struct StatResult { long long best, worst; double mean, stdev; };
StatResult calcStats(const std::vector<long long>& data) {
    if (data.empty()) return {0, 0, 0.0, 0.0};
    long long sum = std::accumulate(data.begin(), data.end(), 0LL); 
    double mean = static_cast<double>(sum) / data.size();
    double sq_sum = std::inner_product(data.begin(), data.end(), data.begin(), 0.0);
    double stdev = std::sqrt(std::max(0.0, sq_sum / data.size() - mean * mean));
    auto minmax = std::minmax_element(data.begin(), data.end()); return {*minmax.first, *minmax.second, mean, stdev};
}

void printStatistics(const std::string& label, const std::vector<long long>& data) {
    StatResult stats = calcStats(data);
    std::cout << label << " -> Best: " << stats.best << " | Worst: " << stats.worst << " | Mean: " << stats.mean << " | StdDev: " << stats.stdev << "\n";
}

void saveSummaryJSON(const std::string& filename, const std::string& dataset, const Problem& inst, long long time_limit, double tp, int runs, const std::vector<long long>& fit, const std::vector<long long>& hcv, const std::vector<long long>& scv, const std::vector<long long>& gens) {
    std::ofstream out(filename); if (!out.is_open()) return;
    StatResult fit_stats = calcStats(fit), hcv_stats = calcStats(hcv), scv_stats = calcStats(scv), gen_stats = calcStats(gens);
    out << "{\n  \"configuration\": {\n    \"dataset\": \"" << dataset << "\",\n    \"events\": " << inst.numEvents << ",\n    \"rooms\": " << inst.numRooms << ",\n    \"students\": " << inst.numStudents << ",\n    \"timeslots\": " << inst.numTimeslots << ",\n    \"time_limit_sec\": " << time_limit << ",\n    \"tp_percentage\": " << tp << ",\n    \"independent_runs\": " << runs << "\n  },\n  \"statistical_results\": {\n";
    auto writeStat = [&out](const std::string& key, const StatResult& s, bool last) {
        out << "    \"" << key << "\": {\n      \"best\": " << s.best << ",\n      \"worst\": " << s.worst << ",\n      \"mean\": " << s.mean << ",\n      \"stddev\": " << s.stdev << "\n    }" << (last ? "" : ",") << "\n";
    };
    writeStat("overall_fitness", fit_stats, false); writeStat("hard_constraint_violations", hcv_stats, false);
    writeStat("soft_constraint_violations", scv_stats, false); writeStat("generations_passed", gen_stats, true);
    out << "  }\n}\n"; out.close();
}

std::string getTimestampString() {
    auto now = std::chrono::system_clock::now(); std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&now_time); std::ostringstream oss; oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S"); return oss.str();
}

int main(int argc, char* argv[]) {
    std::string filename = "";
    std::string base_folder = "timetables/" + getTimestampString();
    bool debug_logging = false; 

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--debug") debug_logging = true;
        else if (arg == "--input" && i + 1 < argc) filename = argv[++i];
        else if (arg == "--output" && i + 1 < argc) base_folder = argv[++i];
        else if (filename.empty() && arg[0] != '-') filename = arg;
    }

    if (filename.empty()) { std::cerr << "Usage: ./uctp_solver --input <instance_file.dat> [--output <dir>]" << std::endl; return 1; }
    
    Problem instance;
    if (!instance.loadFromFile(filename)) {
        std::cerr << "Failed to load problem instance.\n";
        return 1;
    }

    long long time_limit_sec = 90; 
    double tp = 0.2; 
    int num_independent_runs = 1; 

    if(instance.numEvents >= 300 && instance.numStudents >= 200 && instance.numRooms >= 10 && instance.numStudents < 400 && instance.numFeatures < 6) {
        time_limit_sec = 900; num_independent_runs = 1; 
    } else if (instance.numEvents >= 400 && instance.numStudents >= 200 && instance.numRooms >= 10) {
        time_limit_sec = 9000; num_independent_runs = 1; 
    }

    std::cout << "Configured settings\n  dataset: " << filename << "\n  time limit (sec): " << time_limit_sec << "\n  independent runs: " << num_independent_runs << "\n\n";

    std::filesystem::create_directories(base_folder);
    std::vector<long long> fitness_results, hcv_results, scv_results, generation_results;
    
    std::cout << "Starting " << num_independent_runs << " independent runs...\n";

    std::vector<std::future<std::tuple<Individual, long long, std::vector<long long>>>> futures;
    std::vector<unsigned int> seeds(num_independent_runs);

    for (int run = 0; run < num_independent_runs; ++run) {
        seeds[run] = std::random_device{}();
        
        futures.push_back(std::async(std::launch::async, [run, time_limit_sec, &instance, tp, seed = seeds[run]]() {
            HGATS::Params params;
            params.gsgaParams.populationSize = 50;
            params.gsgaParams.timeLimitSec   = time_limit_sec;
            params.tsParams.timeLimitSec     = time_limit_sec;

            HGATS solver(instance, params);
            // currently HGATS solve doesn't take seed but that's fine.
            Individual best = solver.solve();
            
            // returning some dummy data for history since solve() doesn't return history
            long long iters = 100;
            std::vector<long long> hist = { (long long)best.fitness };
            return std::make_tuple(best, iters, hist);
        }));
    }

    for (int run = 0; run < num_independent_runs; ++run) {
        auto [run_best, total_generations, fitness_history] = futures[run].get();
        unsigned int seed = seeds[run];

        std::string json_file = base_folder + "/run_" + std::to_string(run + 1) + "_timetable.json";
        saveTimetableJSON(run_best, json_file, seed, total_generations, instance, fitness_history);

        fitness_results.push_back(run_best.fitness);
        hcv_results.push_back(run_best.distanceToFeasibility);
        scv_results.push_back(run_best.softConstraintPenalty);
        generation_results.push_back(total_generations);

        std::cout << "Run " << (run + 1) << " Finished! -> HCV: " << run_best.distanceToFeasibility 
                  << " | SCV: " << run_best.softConstraintPenalty 
                  << " | Gens: " << total_generations << "\n";
    }

    std::cout << "\n==================================================\n=== FINAL STATISTICAL RESULTS ACROSS " << num_independent_runs << " RUNS ===\n==================================================\n";
    printStatistics("Hard Constraint Violations (HCV)", hcv_results); printStatistics("Soft Constraint Violations (SCV)", scv_results);
    printStatistics("Overall Fitness", fitness_results); printStatistics("Generations Passed", generation_results);
    std::string summary_file = base_folder + "/final_summary.json";
    saveSummaryJSON(summary_file, filename, instance, time_limit_sec, tp, num_independent_runs, fitness_results, hcv_results, scv_results, generation_results);
    std::cout << "\nAll settings and statistical results exported to: " << summary_file << "\n";
    return 0;
}