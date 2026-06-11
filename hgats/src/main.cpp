#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <fstream>

#include "problem.h"
#include "individual.h"
#include "hgats.h"

struct Result {
    std::string name;
    int df;
    int scp;
    double time;
};

HGATS::Params buildParams(int timeLimit) {
    HGATS::Params p;
    p.gsgaParams.populationSize = 50;
    p.gsgaParams.timeLimitSec   = timeLimit;
    p.tsParams.timeLimitSec     = timeLimit;
    return p;
}

Result runInstance(const std::string& path, int timeLimit) {
    Problem prob;
    Result r;

    if (!prob.loadFromFile(path)) {
        std::cerr << "[ERROR] Cannot load " << path << "\n";
        r.df = -1;
        return r;
    }

    auto start = std::chrono::steady_clock::now();

    HGATS solver(prob, buildParams(timeLimit));
    Individual best = solver.solve();

    auto end = std::chrono::steady_clock::now();

    r.name = prob.name;
    r.df   = best.distanceToFeasibility;
    r.scp  = best.softConstraintPenalty;
    r.time = std::chrono::duration<double>(end - start).count();

    return r;
}

int main() {
    std::cout << "================================================\n";
    std::cout << " HGATS: Hybrid GA + Tabu Search for PECTP\n";
    std::cout << " ITC-2007 Benchmark - Post Enrolment\n";
    std::cout << "================================================\n\n";

    // ── Print instances ─────────────────────
    std::cout << "Available instances (1-24):\n";
    for (int i = 1; i <= 24; ++i) {
        std::cout << std::setw(3) << i
                  << ") comp-2007-2-" << i << "\n";
    }

    // ── Input ───────────────────────────────
    std::vector<int> instNumbers;
    int timeLimitSec = 600;

    std::cout << "\nEnter instance number (1-24) or 'all': ";

    std::string input;
    std::getline(std::cin, input);

    if (input == "all" || input == "ALL") {
        for (int i = 1; i <= 24; ++i)
            instNumbers.push_back(i);
    } else {
        try {
            instNumbers.push_back(std::stoi(input));
        } catch (...) {
            std::cerr << "[ERROR] Invalid input\n";
            return 1;
        }
    }

    std::cout << "Enter time limit in seconds (default=600): ";
    std::string t;
    std::getline(std::cin, t);
    if (!t.empty()) timeLimitSec = std::stoi(t);

    // ── Search files ────────────────────────
    std::vector<std::string> paths;

    std::cout << "\nSearching for instances...\n";

    for (int num : instNumbers) {
        std::string fname =
            "data/comp-2007-2-" + std::to_string(num) + ".tim";

        std::ifstream f(fname);
        if (f.good()) {
            paths.push_back(fname);
            std::cout << "  [FOUND] " << fname << "\n";
        } else {
            std::cout << "  [MISS]  " << fname << "\n";
        }
    }

    if (paths.empty()) {
        std::cerr << "[ERROR] No .tim files found!\n";
        return 1;
    }

    // ── Config ─────────────────────────────
    std::cout << "\n================================================\n";
    std::cout << " Run Configuration\n";
    std::cout << "================================================\n";
    std::cout << "  Instances  : " << paths.size() << "\n";
    std::cout << "  Time limit : " << timeLimitSec << " seconds each\n";
    std::cout << "  Population : 50\n";
    std::cout << "  Alpha      : 0.2\n";
    std::cout << "  Beta       : 0.3\n";
    std::cout << "  Gamma      : 0.8\n";
    std::cout << "  Tau        : 20\n";
    std::cout << "  Pm         : 0.5\n";
    std::cout << "================================================\n\n";

    int totalSec = paths.size() * timeLimitSec;
    std::cout << "Estimated total time: "
              << totalSec / 60 << "m "
              << totalSec % 60 << "s\n\n";

    std::cout << "Press Enter to start...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();

    // ── Run ────────────────────────────────
    HGATS::Params params = buildParams(timeLimitSec);

    for (int i = 0; i < paths.size(); i++) {
        std::cout << "\n[" << i+1 << "/" << paths.size()
                  << "] Running...\n";

        Result r = runInstance(paths[i], timeLimitSec);

        std::cout << "Instance: " << r.name
                  << " | Df=" << r.df
                  << " | SCP=" << r.scp
                  << " | Time=" << r.time << "s\n";
    }

    return 0;
}