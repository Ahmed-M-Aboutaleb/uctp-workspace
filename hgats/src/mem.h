#pragma once
#include <vector>
#include <random>
#include <algorithm>

#include "problem.h"
#include "individual.h"

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

    void construct(const std::vector<Individual>& population,
                   double alpha,
                   const Evaluator& eval);

    RoomTimePair randomPairFor(int eventId) const;

    bool hasEntry(int eventId) const {
        return !lists[eventId].empty();
    }

private:
    const Problem& prob;
    std::vector<std::vector<RoomTimePair>> lists;

    int eventPenaltyFast(const Individual& ind, int e,
                         const std::vector<std::vector<int>>& tsIdx) const;

    mutable std::mt19937 rng{std::random_device{}()};
};