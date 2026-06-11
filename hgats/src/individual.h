#pragma once
#include "problem.h"
#include <vector>
#include <limits>

// ─────────────────────────────────────────────────────────────────────────────
//  A single timetable solution
//  assignment[e] = {timeslot, room}   for event e
// ─────────────────────────────────────────────────────────────────────────────

struct Assignment {
    int timeslot = -1;
    int room     = -1;
};

struct Individual {
    std::vector<Assignment> assignment;   // size = numEvents
    int distanceToFeasibility = 0;        // Df  (hard constraint violations)
    int softConstraintPenalty = 0;        // SCP (soft constraint violations)
    int fitness               = 0;        // combined: Df * BIG + SCP

    static constexpr int BIG = 1'000'000; // weight for hard violations

    void init(int numEvents) {
        assignment.assign(numEvents, Assignment{});
    }

    // lower is better
    bool operator<(const Individual& o) const {
        return fitness < o.fitness;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  Evaluation
// ─────────────────────────────────────────────────────────────────────────────
class Evaluator {
public:
    explicit Evaluator(const Problem& p) : prob(p) {}

    // Full evaluation – returns fitness and updates individual fields
    int evaluate(Individual& ind) const;

    // True delta evaluation: O(n/T) instead of O(n²)
    int deltaEvaluate(const Individual& ind,
                      int eventId,
                      int newTimeslot,
                      int newRoom) const;

private:
    const Problem& prob;

    int countHardViolations(const Individual& ind) const;
    int countSoftViolations(const Individual& ind) const;

    // Per-event hard violation count (only checks timeslot neighbors)
    int eventHardPenalty(const Individual& ind, int e) const;

    bool checkH4(const Individual& ind, int e) const;
    bool checkH5(const Individual& ind, int e) const;
};
