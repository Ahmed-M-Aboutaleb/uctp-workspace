#include "hgats.h"
#include <iostream>
#include <iomanip>
#include <chrono>

using namespace std;

Individual HGATS::solve() {
    auto globalStart = chrono::steady_clock::now();
    int totalTime = params.gsgaParams.timeLimitSec;

    cout << "\n";
    cout << "------------------------------------------\n";
    cout << "  HGATS - Phase I: GSGA\n";
    cout << "------------------------------------------\n\n";

    // Phase I: GSGA gets 50% of total time
    GSGA::Params gp = params.gsgaParams;
    gp.timeLimitSec = (int)(totalTime * 0.50);

    GSGA gsga(prob, gp);
    Individual best = gsga.run();

    auto afterPhase1 = chrono::steady_clock::now();
    double usedByPhase1 =
        chrono::duration<double>(afterPhase1 - globalStart).count();

    cout << "\n[Phase I Result]\n";
    cout << "  Distance to Feasibility : "
              << best.distanceToFeasibility << "\n";
    cout << "  Soft Constraint Penalty : "
              << best.softConstraintPenalty << "\n";

    if (isOptimal(best)) {
        cout << "\n*** Optimal solution found in Phase I! ***\n";
        printResults(best);
        return best;
    }

    // Phase II: Tabu Search gets ALL remaining time
    int remaining = max(10, totalTime - (int)usedByPhase1);

    cout << "\n";
    cout << "------------------------------------------\n";
    cout << "  HGATS - Phase II: Tabu Search (" << remaining << "s remaining)\n";
    cout << "------------------------------------------\n\n";

    TabuSearch::Params tp = params.tsParams;
    tp.timeLimitSec = remaining;

    TabuSearch ts(prob, tp);
    Individual improved = ts.run(best);

    cout << "\n[Phase II Result]\n";
    cout << "  Distance to Feasibility : "
              << improved.distanceToFeasibility << "\n";
    cout << "  Soft Constraint Penalty : "
              << improved.softConstraintPenalty << "\n";

    Individual finalBest =
        (improved.fitness < best.fitness) ? improved : best;

    printResults(finalBest);
    return finalBest;
}

void HGATS::printResults(const Individual& best) const {
    cout << "\n";
    cout << "------------------------------------------\n";
    cout << "  FINAL RESULTS\n";
    cout << "------------------------------------------\n";
    cout << "  Df  (Hard Violations) : "
              << best.distanceToFeasibility << "\n";
    cout << "  SCP (Soft Violations) : "
              << best.softConstraintPenalty << "\n";
    cout << "  Fitness               : "
              << best.fitness << "\n";
    cout << "------------------------------------------\n";
}