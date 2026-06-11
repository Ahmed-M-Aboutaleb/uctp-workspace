#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <climits>
#include <stdexcept>
#include "problem.h"
#include "individual.h"
#include "gsnsga2.h"

void printUsage(const char *prog)
{
    std::cout << "Usage: " << prog << " <dataset_file> <time_limit_seconds> <s_max> [options]\n";
    std::cout << "\nRequired:\n";
    std::cout << "  dataset_file         UCTP dataset (e.g., small1.txt)\n";
    std::cout << "  time_limit_seconds   Max runtime per run in seconds (e.g., 60)\n";
    std::cout << "  s_max                Max LS steps per event (e.g., 100)\n";
    std::cout << "\nOptional:\n";
    std::cout << "  --pop <N>            Population size (default: 48)\n";
    std::cout << "  --gamma <g>          GS strategy probability (default: 0.6)\n";
    std::cout << "  --pm <p>             Mutation probability (default: 0.6)\n";
    std::cout << "  --tau <t>            MEM reconstruction frequency (default: 20)\n";
    std::cout << "  --seed <s>           Base random seed (default: 42)\n";
    std::cout << "  --runs <r>           Number of independent runs (default: 1)\n";
    std::cout << "\nSeed mode (for multi-run):\n";
    std::cout << "  --variable-seed      Each run uses seed+0, seed+1, ..., seed+N-1 [DEFAULT]\n";
    std::cout << "                       Gives statistically independent results across runs.\n";
    std::cout << "  --fixed-seed         Every run uses the SAME seed (reproducible).\n";
    std::cout << "                       Useful for debugging: all runs produce identical results.\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << prog << " small1.txt 60 100                             # single run\n";
    std::cout << "  " << prog << " small1.txt 60 100 --runs 20                   # 20 runs, variable seeds\n";
    std::cout << "  " << prog << " small1.txt 60 100 --runs 20 --fixed-seed      # 20 runs, same seed\n";
    std::cout << "  " << prog << " small1.txt 60 100 --runs 20 --seed 123        # 20 runs starting at seed 123\n";
}

// Extract best feasible solution from a Pareto front (lowest total SCV)
// Falls back to best infeasible if no feasible found
struct RunResult
{
    int run;
    int seed;
    int bestHCV;
    int bestF1, bestF2, bestF3;
    int bestSCV; // total soft constraint violations of best solution
    int paretoSize;
    int feasibleCount;
    double runtimeSecs;
    bool hasFeasible;
};

RunResult extractResult(int run, int seed, const std::vector<Individual> &front, double rt)
{
    RunResult r;
    r.run = run;
    r.seed = seed;
    r.paretoSize = (int)front.size();
    r.runtimeSecs = rt;
    r.feasibleCount = 0;
    r.hasFeasible = false;

    int bestHCV = INT_MAX, bestSCV = INT_MAX;
    int bestF1 = 0, bestF2 = 0, bestF3 = 0;

    for (const auto &ind : front)
    {
        if (ind.feasible())
            r.feasibleCount++;
        int scv = ind.f1 + ind.f2 + ind.f3;
        // Prefer feasible; among feasible, prefer lower total SCV
        // Among infeasible, prefer lower HCV then lower SCV
        bool better = false;
        if (ind.feasible() && !r.hasFeasible)
        {
            better = true;
        }
        else if (ind.feasible() && r.hasFeasible && scv < bestSCV)
        {
            better = true;
        }
        else if (!ind.feasible() && !r.hasFeasible)
        {
            if (ind.hcv < bestHCV || (ind.hcv == bestHCV && scv < bestSCV))
                better = true;
        }
        if (better)
        {
            bestHCV = ind.hcv;
            bestSCV = scv;
            bestF1 = ind.f1;
            bestF2 = ind.f2;
            bestF3 = ind.f3;
            r.hasFeasible = ind.feasible();
        }
    }

    r.bestHCV = bestHCV == INT_MAX ? -1 : bestHCV;
    r.bestSCV = bestSCV == INT_MAX ? -1 : bestSCV;
    r.bestF1 = bestF1;
    r.bestF2 = bestF2;
    r.bestF3 = bestF3;
    return r;
}

void printFront(const std::vector<Individual> &front)
{
    std::vector<Individual> sorted = front;
    std::sort(sorted.begin(), sorted.end(), [](const Individual &a, const Individual &b)
              { return (a.f1 + a.f2 + a.f3) < (b.f1 + b.f2 + b.f3); });

    std::cout << std::left
              << std::setw(6) << "#"
              << std::setw(8) << "HCV"
              << std::setw(10) << "f1(S1)"
              << std::setw(10) << "f2(S2)"
              << std::setw(10) << "f3(S3)"
              << std::setw(12) << "Total SCV"
              << std::setw(10) << "Feasible"
              << "\n";
    std::cout << std::string(66, '-') << "\n";

    int feasCount = 0;
    for (int i = 0; i < (int)sorted.size(); i++)
    {
        const auto &ind = sorted[i];
        if (ind.feasible())
            feasCount++;
        std::cout << std::left
                  << std::setw(6) << (i + 1)
                  << std::setw(8) << ind.hcv
                  << std::setw(10) << ind.f1
                  << std::setw(10) << ind.f2
                  << std::setw(10) << ind.f3
                  << std::setw(12) << (ind.f1 + ind.f2 + ind.f3)
                  << std::setw(10) << (ind.feasible() ? "YES" : "no")
                  << "\n";
    }
    std::cout << std::string(66, '-') << "\n";
    std::cout << "Feasible: " << feasCount << " / " << sorted.size() << "\n";
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printUsage(argv[0]);
        return 1;
    }

    std::string dataFile = argv[1];
    int timeLimit = 0, smax = 0;
    try
    {
        timeLimit = std::stoi(argv[2]);
        smax = std::stoi(argv[3]);
    }
    catch (...)
    {
        std::cerr << "Error: time_limit and s_max must be integers.\n";
        return 1;
    }
    if (timeLimit <= 0 || smax <= 0)
    {
        std::cerr << "Error: time_limit and s_max must be positive.\n";
        return 1;
    }

    GSNSGAParams params;
    params.timeLimitSecs = timeLimit;
    params.smax = smax;
    int numRuns = 1;
    bool fixedSeed = false; // default: variable seeds across runs

    for (int i = 4; i < argc; i++)
    {
        std::string opt = argv[i];
        if (opt == "--pop" && i + 1 < argc)
            params.popSize = std::stoi(argv[++i]);
        else if (opt == "--gamma" && i + 1 < argc)
            params.gamma = std::stod(argv[++i]);
        else if (opt == "--pm" && i + 1 < argc)
            params.pm = std::stod(argv[++i]);
        else if (opt == "--tau" && i + 1 < argc)
            params.tau = std::stoi(argv[++i]);
        else if (opt == "--seed" && i + 1 < argc)
            params.seed = std::stoi(argv[++i]);
        else if (opt == "--runs" && i + 1 < argc)
            numRuns = std::stoi(argv[++i]);
        else if (opt == "--fixed-seed")
            fixedSeed = true;
        else if (opt == "--variable-seed")
            fixedSeed = false;
        else if (opt == "--help" || opt == "-h")
        {
            printUsage(argv[0]);
            return 0;
        }
        else
            std::cerr << "Warning: Unknown option '" << opt << "'\n";
    }

    // Header
    std::cout << "========================================\n";
    std::cout << "  GSNSGA-II: University Course Timetabling\n";
    std::cout << "========================================\n\n";
    std::cout << "Dataset:      " << dataFile << "\n";
    std::cout << "Time/run:     " << timeLimit << " seconds\n";
    std::cout << "s_max:        " << smax << "\n";
    std::cout << "Population:   " << params.popSize << "\n";
    std::cout << "Gamma (GS%):  " << params.gamma << "\n";
    std::cout << "Pm (mutate):  " << params.pm << "\n";
    std::cout << "Tau:          " << params.tau << "\n";
    std::cout << "Base seed:    " << params.seed << "\n";
    std::cout << "Seed mode:    " << (fixedSeed ? "fixed (all runs use seed " + std::to_string(params.seed) + ")" : "variable (seed+" + std::to_string(0) + " .. seed+" + std::to_string(numRuns - 1) + ")") << "\n";
    std::cout << "Runs:         " << numRuns << "\n\n";

    // Load problem
    Problem pb;
    try
    {
        std::cout << "Loading dataset...\n";
        pb = Problem::load(dataFile);
        pb.printInfo();
        std::cout << "\n";
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    // Single run mode
    if (numRuns == 1)
    {
        params.verbose = true;
        auto t0 = std::chrono::steady_clock::now();
        auto front = GSNSGA2::run(pb, params);
        double rt = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

        std::cout << "\n========================================\n";
        std::cout << "  RESULTS\n";
        std::cout << "========================================\n";
        std::cout << "Runtime: " << std::fixed << std::setprecision(2) << rt << "s  |  "
                  << "Pareto front size: " << front.size() << "\n\n";
        printFront(front);
        std::cout << "\nf1=S1 (last slot), f2=S2 (>2 consecutive), f3=S3 (single event/day)\n";
        return 0;
    }

    // ---- Multi-run mode ----
    params.verbose = false;
    std::vector<RunResult> results;
    results.reserve(numRuns);

    int totalTimeEst = numRuns * timeLimit;
    std::cout << "Running " << numRuns << " independent runs ("
              << totalTimeEst << "s total estimated)...\n\n";

    // Per-run header
    std::cout << std::left
              << std::setw(5) << "Run"
              << std::setw(7) << "Seed"
              << std::setw(7) << "HCV"
              << std::setw(8) << "f1(S1)"
              << std::setw(8) << "f2(S2)"
              << std::setw(8) << "f3(S3)"
              << std::setw(11) << "Total SCV"
              << std::setw(9) << "Feasible"
              << std::setw(8) << "PFsize"
              << std::setw(8) << "Time(s)"
              << "\n";
    std::cout << std::string(79, '-') << "\n";

    int baseSeed = params.seed;
    for (int r = 1; r <= numRuns; r++)
    {
        params.seed = fixedSeed ? baseSeed : baseSeed + (r - 1);
        auto t0 = std::chrono::steady_clock::now();
        auto front = GSNSGA2::run(pb, params);
        double rt = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();

        RunResult res = extractResult(r, params.seed, front, rt);
        results.push_back(res);

        std::cout << std::left
                  << std::setw(5) << r
                  << std::setw(7) << params.seed
                  << std::setw(7) << res.bestHCV
                  << std::setw(8) << res.bestF1
                  << std::setw(8) << res.bestF2
                  << std::setw(8) << res.bestF3
                  << std::setw(11) << res.bestSCV
                  << std::setw(9) << (res.hasFeasible ? "YES" : "no")
                  << std::setw(8) << res.paretoSize
                  << std::fixed << std::setprecision(1) << std::setw(8) << rt
                  << "\n";
        std::cout.flush();
    }

    // ---- Summary Statistics ----
    std::cout << "\n"
              << std::string(79, '=') << "\n";
    std::cout << "  SUMMARY STATISTICS (" << numRuns << " runs)\n";
    std::cout << std::string(79, '=') << "\n\n";

    // Separate feasible and all-run stats
    std::vector<int> allSCV, feasSCV;
    std::vector<int> allF1, allF2, allF3;
    int feasRuns = 0;

    for (auto &res : results)
    {
        allSCV.push_back(res.bestSCV);
        allF1.push_back(res.bestF1);
        allF2.push_back(res.bestF2);
        allF3.push_back(res.bestF3);
        if (res.hasFeasible)
        {
            feasRuns++;
            feasSCV.push_back(res.bestSCV);
        }
    }

    auto calcStats = [](const std::vector<int> &v, double &avg, int &best, int &worst)
    {
        if (v.empty())
        {
            avg = 0;
            best = -1;
            worst = -1;
            return;
        }
        best = *std::min_element(v.begin(), v.end());
        worst = *std::max_element(v.begin(), v.end());
        avg = (double)std::accumulate(v.begin(), v.end(), 0LL) / v.size();
    };

    // Std dev
    auto calcStdDev = [](const std::vector<int> &v, double avg) -> double
    {
        if (v.size() < 2)
            return 0.0;
        double sq = 0;
        for (int x : v)
            sq += (x - avg) * (x - avg);
        return std::sqrt(sq / (v.size() - 1));
    };

    double avgSCV;
    int bestSCV, worstSCV;
    calcStats(allSCV, avgSCV, bestSCV, worstSCV);
    double stdSCV = calcStdDev(allSCV, avgSCV);

    double avgF1;
    int bestF1, worstF1;
    double avgF2;
    int bestF2, worstF2;
    double avgF3;
    int bestF3, worstF3;
    calcStats(allF1, avgF1, bestF1, worstF1);
    calcStats(allF2, avgF2, bestF2, worstF2);
    calcStats(allF3, avgF3, bestF3, worstF3);

    std::cout << "Feasibility:\n";
    std::cout << "  Runs with feasible solutions: " << feasRuns << " / " << numRuns << "\n";
    if (!feasSCV.empty())
    {
        double avgF;
        int bF, wF;
        calcStats(feasSCV, avgF, bF, wF);
        std::cout << "  Best SCV (feasible runs only): " << bF << "\n";
        std::cout << "  Avg  SCV (feasible runs only): " << std::fixed << std::setprecision(2) << avgF << "\n";
        std::cout << "  Worst SCV (feasible runs only): " << wF << "\n";
    }
    std::cout << "\n";

    std::cout << "All-run statistics (best solution per run):\n";
    std::cout << std::string(55, '-') << "\n";
    std::cout << std::left
              << std::setw(20) << "Metric"
              << std::setw(10) << "Best"
              << std::setw(12) << "Average"
              << std::setw(10) << "Worst"
              << std::setw(10) << "Std Dev"
              << "\n";
    std::cout << std::string(55, '-') << "\n";

    auto printRow = [&](const std::string &name, int best, double avg, int worst, double sd)
    {
        std::cout << std::left << std::setw(20) << name
                  << std::left << std::setw(10) << best
                  << std::fixed << std::setprecision(2)
                  << std::left << std::setw(12) << avg
                  << std::left << std::setw(10) << worst
                  << std::left << std::setw(10) << sd
                  << "\n";
    };

    printRow("Total SCV", bestSCV, avgSCV, worstSCV, stdSCV);
    printRow("f1 (S1)", bestF1, avgF1, worstF1, calcStdDev(allF1, avgF1));
    printRow("f2 (S2)", bestF2, avgF2, worstF2, calcStdDev(allF2, avgF2));
    printRow("f3 (S3)", bestF3, avgF3, worstF3, calcStdDev(allF3, avgF3));
    std::cout << std::string(55, '-') << "\n\n";

    // Best run detail
    int bestRunIdx = (int)(std::min_element(results.begin(), results.end(),
                                            [](const RunResult &a, const RunResult &b)
                                            {
                                                if (a.hasFeasible != b.hasFeasible)
                                                    return a.hasFeasible > b.hasFeasible;
                                                return a.bestSCV < b.bestSCV;
                                            }) -
                           results.begin());

    std::cout << "Best run: Run " << results[bestRunIdx].run
              << " (seed=" << results[bestRunIdx].seed << ")"
              << " | SCV=" << results[bestRunIdx].bestSCV
              << " | f1=" << results[bestRunIdx].bestF1
              << " | f2=" << results[bestRunIdx].bestF2
              << " | f3=" << results[bestRunIdx].bestF3
              << " | " << (results[bestRunIdx].hasFeasible ? "FEASIBLE" : "infeasible")
              << "\n\n";

    std::cout << "f1=S1 (last slot), f2=S2 (>2 consecutive), f3=S3 (single event/day)\n";

    return 0;
}
