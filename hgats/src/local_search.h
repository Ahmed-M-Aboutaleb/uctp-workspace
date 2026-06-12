#pragma once
#include <vector>
#include <set>
#include <random>
#include <algorithm>
#include <functional>

#include "problem.h"
#include "individual.h"

using namespace std;

class LocalSearch {
public:
    LocalSearch(const Problem& p, const Evaluator& ev)
        : prob(p), eval(ev) {}

    void applyLS3(Individual& ind, int maxSteps = 500);
    void applyLS4(Individual& ind, int percentTS = 30);

    // Used after timeslot-only initialization to assign rooms via matching
    void assignRoomsViaMatching(Individual& ind) const;

    // Single move in a specific neighborhood (for mutation)
    bool singleMoveN1(Individual& ind, int e, bool fixHard);
    bool singleMoveN2(Individual& ind, int e, bool fixHard);
    bool singleMoveN3(Individual& ind, int e, bool fixHard);
    bool singleMoveN4(Individual& ind, int e, bool fixHard);

    int eventHardPenaltyFast(const Individual& ind, int e) const;
    bool eventHasSoftViolation(const Individual& ind, int e) const;


private:
    const Problem&   prob;
    const Evaluator& eval;

    bool moveN1(Individual& ind, int e, bool fixHard);
    bool moveN2(Individual& ind, int e, bool fixHard);
    bool moveN3(Individual& ind, int e, bool fixHard);
    bool moveN4(Individual& ind, int e, bool fixHard);
    bool moveN5(Individual& ind, int e);
    bool moveN6(Individual& ind, int percentTS);

    // Timeslot index for fast lookups
    vector<vector<int>> tsEvents;
    void buildIndex(const Individual& ind);
    void moveEventInIndex(int e, int oldTs, int newTs);

    // ── Room Assignment ──────────────────────────────────────────
    // Greedy: first free suitable room in ts (ignoring event e itself)
    int matchRoomGreedy(const Individual& ind, int e, int ts,
                        int exclude1 = -1, int exclude2 = -1) const;

    // Bipartite matching for ALL events in ts.
    // Returns room for target_event, -1 if impossible.
    // Also fills sideEffects: {event → newRoom} for events whose
    // room assignment changed during matching (so caller can revert).
    int matchRoomBipartite(const Individual& ind,
                           int target_event, int ts,
                           vector<pair<int,int>>& sideEffects) const;

    // Combined: greedy first, fallback to bipartite.
    // sideEffects is populated only when bipartite was used.
    int findBestRoom(const Individual& ind, int e, int ts,
                     vector<pair<int,int>>& sideEffects,
                     int exclude1 = -1, int exclude2 = -1) const;

    bool isInfeasibleFast(const Individual& ind, int e) const;

    mutable mt19937 rng{random_device{}()};
};