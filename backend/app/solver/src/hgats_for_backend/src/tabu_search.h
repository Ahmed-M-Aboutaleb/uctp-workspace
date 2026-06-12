#pragma once
#include <vector>
#include <deque>
#include <set>
#include <utility>
#include <random>
#include <chrono>
#include <iostream>
#include <algorithm>

#include "problem.h"
#include "individual.h"
#include "local_search.h"

using namespace std;

class TabuSearch {
public:
    struct Params {
        int    tabuListK        = 100;
        double probN1           = 0.4;
        double probN2           = 0.3;
        double probN4           = 0.3;
        int    timeLimitSec     = 600;
        int    maxNeighbors     = 50;
        int    stagnationLimit  = 5000;
    };

    TabuSearch(const Problem& p, const Params& par);
    Individual run(const Individual& startSolution);

private:
    const Problem& prob;
    Params         params;
    Evaluator      eval;
    LocalSearch    ls;

    int             tabuListLength;

    struct TabuEntry {
        int event;
        int timeslot;
        bool operator<(const TabuEntry& o) const {
            if (event != o.event) return event < o.event;
            return timeslot < o.timeslot;
        }
    };
    deque<TabuEntry>  tabuList;
    set<TabuEntry>    tabuSet;

    struct Move {
        int type          = 1;
        int event1        = -1;
        int event2        = -1;
        int newTs1        = -1;
        int newTs2        = -1;
        int newRoom1      = -1;
        int newRoom2      = -1;
        int resultFitness = 0;
    };

    vector<Move> generateNeighbors(
        const Individual& ind,
        int numToCheck) const;

    Individual applyMove(
        const Individual& ind,
        const Move& m) const;

    bool isTabu(const Move& m) const;
    void addToTabu(const Move& m);
    void clearTabu();

    Individual perturbAndRestart(const Individual& best,
                                  LocalSearch& ls_ref);

    mutable mt19937 rng{random_device{}()};
};
