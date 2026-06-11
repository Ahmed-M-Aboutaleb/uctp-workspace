#pragma once
#include <vector>
#include <random>
#include "individual.h"
#include "problem.h"

class Mutation
{
public:
    // Apply a random neighbourhood move (N1, N2, N3, or N7)
    static void mutate(Individual &ind, const Problem &pb, double pm, std::mt19937 &rng)
    {
        std::uniform_real_distribution<double> prob(0.0, 1.0);
        if (prob(rng) >= pm)
            return;

        std::uniform_int_distribution<int> nDist(0, 3);
        std::uniform_int_distribution<int> eDist(0, pb.n - 1);
        std::uniform_int_distribution<int> tDist(0, pb.p - 1);

        int neigh = nDist(rng);
        int e1 = eDist(rng);

        if ((int)ind.slotEvents.size() != pb.p)
            ind.buildFastState(pb);

        switch (neigh)
        {
        case 0:
        { // N1: move event to random slot
            int t = tDist(rng);
            ind.fastMoveEvent(e1, t, pb);
            break;
        }
        case 1:
        { // N2: swap two events
            int e2 = eDist(rng);
            while (e2 == e1)
                e2 = eDist(rng);
            int t1 = ind.timeSlot[e1], t2 = ind.timeSlot[e2];
            ind.fastMoveEvent(e1, t2, pb);
            ind.fastMoveEvent(e2, t1, pb);
            break;
        }
        case 2:
        { // N3: cyclic exchange of 3 events
            int e2 = eDist(rng), e3 = eDist(rng);
            while (e2 == e1)
                e2 = eDist(rng);
            while (e3 == e1 || e3 == e2)
                e3 = eDist(rng);
            int t1 = ind.timeSlot[e1], t2 = ind.timeSlot[e2], t3 = ind.timeSlot[e3];
            ind.fastMoveEvent(e1, t2, pb);
            ind.fastMoveEvent(e2, t3, pb);
            ind.fastMoveEvent(e3, t1, pb);
            break;
        }
        case 3:
        { // N7: swap two consecutive pairs
            int e2 = (e1 + 1) % pb.n;
            int e3 = eDist(rng);
            while (e3 == e1 || e3 == e2)
                e3 = eDist(rng);
            int e4 = (e3 + 1) % pb.n;
            while (e4 == e1 || e4 == e2)
                e4 = (e4 + 1) % pb.n;
            int t1 = ind.timeSlot[e1], t2 = ind.timeSlot[e2];
            int t3 = ind.timeSlot[e3], t4 = ind.timeSlot[e4];
            ind.fastMoveEvent(e1, t3, pb);
            ind.fastMoveEvent(e2, t4, pb);
            ind.fastMoveEvent(e3, t1, pb);
            ind.fastMoveEvent(e4, t2, pb);
            break;
        }
        }

        ind.assignRooms(pb);
        ind.evaluate(pb);
    }
};
