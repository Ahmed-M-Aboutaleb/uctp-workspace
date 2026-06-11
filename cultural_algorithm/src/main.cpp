// Cultural Algorithm with Operator Parameters Control for the
// University Course Timetabling Problem (UCTP).
//
// Implements: Soza, Landa, Riff, Coello —
// "A Cultural Algorithm with Operator Parameters Control for Solving
//  Timetabling Problems".
//
// Usage:
//   cultural_algorithm <dataset> [options]
//
// Options:
//   -t, --time   <seconds>   wall-clock time limit (default 360)
//   -g, --gens   <num>       hard cap on generations  (default very large)
//   -p, --pop    <size>      population size           (default 10)
//   -s, --seed   <uint>      RNG seed                  (0 = random)
//       --quiet              less verbose output
//
// The program prints the best solution found and writes the schedule to
// solution.txt.

#include "CulturalAlgorithm.hpp"
#include "Individual.hpp"
#include "Problem.hpp"
#include "Random.hpp"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

static void printUsage(const char* arg0) {
    std::cout << "Usage: " << arg0 << " <dataset> [options]\n"
              << "  -t, --time   <seconds>   time limit (default 360)\n"
              << "  -g, --gens   <num>       max generations\n"
              << "  -p, --pop    <size>      population size (default 10)\n"
              << "  -s, --seed   <uint>      RNG seed (0=random)\n"
              << "      --quiet              less verbose output\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string datasetPath = argv[1];
    CAConfig cfg;

    for (int i = 2; i < argc; ++i) {
        std::string a = argv[i];
        auto need = [&](const char* opt) {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for " << opt << "\n";
                std::exit(1);
            }
            return argv[++i];
        };
        if      (a == "-t" || a == "--time")  cfg.timeLimitSec   = std::atof(need("--time"));
        else if (a == "-g" || a == "--gens")  cfg.maxGenerations = std::atoi(need("--gens"));
        else if (a == "-p" || a == "--pop")   cfg.populationSize = std::atoi(need("--pop"));
        else if (a == "-s" || a == "--seed")  cfg.seed           = (uint32_t)std::strtoul(need("--seed"), nullptr, 10);
        else if (a == "--quiet")              cfg.verbose        = false;
        else if (a == "-h" || a == "--help")  { printUsage(argv[0]); return 0; }
        else {
            std::cerr << "Unknown option: " << a << "\n";
            return 1;
        }
    }

    Problem problem;
    if (!problem.load(datasetPath)) return 1;

    CulturalAlgorithm ca(problem, cfg);
    Individual best = ca.run();

    // Final report
    std::cout << "\n========== RESULT ==========\n";
    std::cout << "Hard violations: " << best.hardViol << "\n";
    std::cout << "Soft violations: " << best.softViol << "\n";
    std::cout << "Fitness        : " << best.fitness << "\n";
    std::cout << "Feasible       : " << (best.hardViol == 0 ? "YES" : "NO") << "\n";

    // Dump the schedule (one line per event: room timeslot)
    std::ofstream out("solution.txt");
    if (out.is_open()) {
        for (int e = 0; e < problem.numEvents; ++e) {
            out << best.eventRoom[e] << " " << best.eventSlot[e] << "\n";
        }
        std::cout << "Schedule written to solution.txt\n";
    }
    return 0;
}
