#pragma once
#include <vector>
#include <array>
#include <random>
#include <algorithm>
#include <numeric>
#include "individual.h"
#include "problem.h"

// MEM data structure: for each event, list of (room, timeslot) pairs from good solutions
struct MEMEntry
{
    int room;
    int slot;
};

using MEM = std::vector<std::vector<MEMEntry>>; // MEM[event] = list of (room,slot)

class GSStrategy
{
public:
    static const int NUM_MEMS = 3;

    // Algorithm 24: Construct MEMi data structures
    static std::array<MEM, NUM_MEMS> constructMEM(
        const std::vector<Individual> &pop,
        const Problem &pb,
        double alpha,
        std::mt19937 & /*rng*/)
    {
        int N = (int)pop.size();
        int selectCount = std::max(1, (int)(alpha * N));

        std::vector<int> indices(N);
        std::iota(indices.begin(), indices.end(), 0);
        std::sort(indices.begin(), indices.end(), [&](int a, int b)
                  {
            if (pop[a].rank != pop[b].rank) return pop[a].rank < pop[b].rank;
            return pop[a].crowdingDist > pop[b].crowdingDist; });
        indices.resize(selectCount);

        std::array<MEM, NUM_MEMS> mems;
        for (int i = 0; i < NUM_MEMS; i++)
            mems[i].resize(pb.n);

        for (int idx : indices)
        {
            const Individual &ind = pop[idx];
            int objVals[3] = {ind.f1, ind.f2, ind.f3};
            for (int oi = 0; oi < NUM_MEMS; oi++)
            {
                if (objVals[oi] == 0 && ind.feasible())
                {
                    for (int e = 0; e < pb.n; e++)
                    {
                        if (ind.h1Count[e] == 0 && ind.room[e] != -1)
                        {
                            mems[oi][e].push_back({ind.room[e], ind.timeSlot[e]});
                        }
                    }
                }
            }
        }
        return mems;
    }

    // Algorithm 25: Generate child using GS strategy
    static Individual guidedSearch(
        const std::array<MEM, NUM_MEMS> &mems,
        const Problem &pb,
        double beta,
        std::mt19937 &rng)
    {
        Individual child;
        child.timeSlot.resize(pb.n);

        std::uniform_int_distribution<int> memDist(0, NUM_MEMS - 1);
        int memIdx = memDist(rng);
        const MEM &mem = mems[memIdx];

        int numSelected = std::max(1, (int)(beta * pb.n));
        std::vector<int> eventOrder(pb.n);
        std::iota(eventOrder.begin(), eventOrder.end(), 0);
        std::shuffle(eventOrder.begin(), eventOrder.end(), rng);
        std::vector<bool> inEs(pb.n, false);
        for (int i = 0; i < numSelected; i++)
            inEs[eventOrder[i]] = true;

        std::uniform_int_distribution<int> slotDist(0, pb.p - 1);

        for (int e = 0; e < pb.n; e++)
        {
            if (inEs[e] && !mem[e].empty())
            {
                std::uniform_int_distribution<int> ed(0, (int)mem[e].size() - 1);
                child.timeSlot[e] = mem[e][ed(rng)].slot;
            }
            else
            {
                child.timeSlot[e] = slotDist(rng);
            }
        }

        child.buildFastState(pb);
        child.assignRooms(pb);
        child.evaluate(pb);
        child.rank = 0;
        child.crowdingDist = 0.0;
        child.domCount = 0;
        return child;
    }
};
