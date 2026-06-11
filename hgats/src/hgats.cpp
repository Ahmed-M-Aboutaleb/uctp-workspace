#include "hgats.h"
#include <iostream>
#include <iomanip>
#include <chrono>

Individual HGATS::solve() {
    auto globalStart = std::chrono::steady_clock::now();
    int totalTime = params.gsgaParams.timeLimitSec;

    std::cout << "\n";
    std::cout << "------------------------------------------\n";
    std::cout << "  HGATS - Phase I: GSGA\n";
    std::cout << "------------------------------------------\n\n";

    // Phase I: GSGA gets 40% of total time
    GSGA::Params gp = params.gsgaParams;
    gp.timeLimitSec = (int)(totalTime * 0.50);


    GSGA gsga(prob, gp);
    Individual best = gsga.run();

    auto afterPhase1 = std::chrono::steady_clock::now();
    double usedByPhase1 =
        std::chrono::duration<double>(afterPhase1 - globalStart).count();

    std::cout << "\n[Phase I Result]\n";
    std::cout << "  Distance to Feasibility : "
              << best.distanceToFeasibility << "\n";
    std::cout << "  Soft Constraint Penalty : "
              << best.softConstraintPenalty << "\n";

    if (isOptimal(best)) {
        std::cout << "\n*** Optimal solution found in Phase I! ***\n";
        printResults(best);
        return best;
    }

    // Phase II: Tabu Search gets ALL remaining time
    int remaining = std::max(10, totalTime - (int)usedByPhase1);

    std::cout << "\n";
    std::cout << "------------------------------------------\n";
    std::cout << "  HGATS - Phase II: Tabu Search (" << remaining << "s remaining)\n";
    std::cout << "------------------------------------------\n\n";

    TabuSearch::Params tp = params.tsParams;
    tp.timeLimitSec = remaining;

    TabuSearch ts(prob, tp);
    Individual improved = ts.run(best);

    std::cout << "\n[Phase II Result]\n";
    std::cout << "  Distance to Feasibility : "
              << improved.distanceToFeasibility << "\n";
    std::cout << "  Soft Constraint Penalty : "
              << improved.softConstraintPenalty << "\n";

    Individual finalBest =
        (improved.fitness < best.fitness) ? improved : best;

    printResults(finalBest);
    return finalBest;
}

void HGATS::printResults(const Individual& best) const {
    std::cout << "\n";
    std::cout << "------------------------------------------\n";
    std::cout << "  FINAL RESULTS\n";
    std::cout << "------------------------------------------\n";
    std::cout << "  Df  (Hard Violations) : "
              << best.distanceToFeasibility << "\n";
    std::cout << "  SCP (Soft Violations) : "
              << best.softConstraintPenalty << "\n";
    std::cout << "  Fitness               : "
              << best.fitness << "\n";
    std::cout << "------------------------------------------\n";
}
