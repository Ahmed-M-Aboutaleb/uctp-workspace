#pragma once
#include <vector>
#include <random>
#include <algorithm>

#include "problem.h"
#include "individual.h"

using namespace std;

struct RoomTimePair {
    int room;
    int timeslot;
    bool operator==(const RoomTimePair& o) const {
        return room == o.room && timeslot == o.timeslot;
    }
};

class MEM {
public:
    explicit MEM(const Problem& p) : prob(p) {
        lists.resize(p.numEvents);
    }

    void construct(const vector<Individual>& population,
                   double alpha,
                   const Evaluator& eval);

    RoomTimePair randomPairFor(int eventId) const;

    bool hasEntry(int eventId) const {
        return !lists[eventId].empty();
    }

private:
    const Problem& prob;
    vector<vector<RoomTimePair>> lists;

    int eventPenaltyFast(const Individual& ind, int e,
                         const vector<vector<int>>& tsIdx) const;

    mutable mt19937 rng{random_device{}()};
};