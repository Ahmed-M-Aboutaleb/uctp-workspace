// hgats.h ─────────────────────────────────────────────────────────────────────
#pragma once
#include "problem.h"
#include "individual.h"
#include "gsga.h"
#include "tabu_search.h"

// ─────────────────────────────────────────────────────────────────────────────
//  HGATS: Hybrid GA + Tabu Search (Algorithm 15)
//  Phase I  → GSGA
//  Phase II → TabuSearch (if not optimal)
// ─────────────────────────────────────────────────────────────────────────────
class HGATS {
public:
    struct Params {
        GSGA::Params       gsgaParams;
        TabuSearch::Params tsParams;
    };

    explicit HGATS(const Problem& p, const Params& par)
        : prob(p), params(par) {}

    // Main entry point
    Individual solve();

    // Result reporting
    void printResults(const Individual& best) const;

private:
    const Problem& prob;
    Params         params;

    bool isOptimal(const Individual& ind) const {
        return ind.distanceToFeasibility == 0 &&
               ind.softConstraintPenalty  == 0;
    }
};