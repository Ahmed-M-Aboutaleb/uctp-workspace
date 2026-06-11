#pragma once
#include <vector>
#include <random>
#include "individual.h"
#include "problem.h"

class Crossover
{
public:
    // Uniform crossover: for each event, randomly pick time slot from one parent
    // Then apply matching for room assignment
    static Individual uniformCrossover(
        const Individual &p1,
        const Individual &p2,
        const Problem &pb,
        std::mt19937 &rng)
    {
        Individual child;
        child.timeSlot.resize(pb.n);
        child.room.resize(pb.n, -1);

        std::uniform_int_distribution<int> coin(0, 1);
        for (int e = 0; e < pb.n; e++)
            child.timeSlot[e] = (coin(rng) == 0) ? p1.timeSlot[e] : p2.timeSlot[e];

        child.buildFastState(pb);
        child.assignRooms(pb);
        child.evaluate(pb);
        child.rank = 0;
        child.crowdingDist = 0.0;
        child.domCount = 0;
        return child;
    }
};
