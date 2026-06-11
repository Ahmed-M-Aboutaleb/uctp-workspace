#pragma once
#include <vector>
#include <algorithm>
#include <random>
#include <numeric>
#include <climits>
#include <iostream>
#include "problem.h"
#include "matching.h"

struct Individual
{
    std::vector<int> timeSlot; // timeSlot[event] = time slot (0..p-1)
    std::vector<int> room;     // room[event] = room index, or -1

    // Objective values (3 soft constraint objectives)
    int f1, f2, f3;

    // Hard constraint violations (H1+H2+H3)
    int hcv;

    // Fast incremental tracking
    std::vector<std::vector<int>> slotEvents; // slotEvents[t] = list of events in slot t
    std::vector<int> h1Count;                 // h1Count[e] = # H1 violations for event e
    int totalH1;                              // total H1 violations (each pair counted once)

    // NSGA-II
    int rank;
    double crowdingDist;
    int domCount;
    std::vector<int> dominated;

    bool feasible() const { return hcv == 0; }

    // Build fast state from timeSlot array
    void buildFastState(const Problem &pb)
    {
        slotEvents.assign(pb.p, {});
        for (int e = 0; e < pb.n; e++)
            slotEvents[timeSlot[e]].push_back(e);

        h1Count.assign(pb.n, 0);
        totalH1 = 0;
        for (int t = 0; t < pb.p; t++)
        {
            const auto &evts = slotEvents[t];
            for (int i = 0; i < (int)evts.size(); i++)
            {
                for (int j = i + 1; j < (int)evts.size(); j++)
                {
                    if (pb.eventConflict[evts[i]][evts[j]])
                    {
                        h1Count[evts[i]]++;
                        h1Count[evts[j]]++;
                        totalH1++;
                    }
                }
            }
        }
    }

    // Move event e to new slot (updates fast state, does NOT update rooms)
    void fastMoveEvent(int e, int newSlot, const Problem &pb)
    {
        int oldSlot = timeSlot[e];
        if (oldSlot == newSlot)
            return;

        // Update H1 counts using conflict adjacency list
        for (int e2 : pb.conflictAdj[e])
        {
            if (timeSlot[e2] == oldSlot)
            {
                h1Count[e]--;
                h1Count[e2]--;
                totalH1--;
            }
            if (timeSlot[e2] == newSlot)
            {
                h1Count[e]++;
                h1Count[e2]++;
                totalH1++;
            }
        }

        // Update slotEvents
        auto &oldList = slotEvents[oldSlot];
        oldList.erase(std::find(oldList.begin(), oldList.end(), e));
        slotEvents[newSlot].push_back(e);
        timeSlot[e] = newSlot;
    }

    // Delta H1 if we move event e to newSlot (without applying)
    int deltaH1Move(int e, int newSlot, const Problem &pb) const
    {
        int oldSlot = timeSlot[e];
        if (oldSlot == newSlot)
            return 0;
        int gain = 0;
        for (int e2 : pb.conflictAdj[e])
        {
            if (timeSlot[e2] == oldSlot)
                gain++; // remove conflict
            if (timeSlot[e2] == newSlot)
                gain--; // add conflict
        }
        return gain; // positive = improvement
    }

    // Delta H1 for swapping events e1 and e2
    int deltaH1Swap(int e1, int e2, const Problem &pb) const
    {
        int t1 = timeSlot[e1], t2 = timeSlot[e2];
        if (t1 == t2)
            return 0;
        int gain = 0;
        for (int ex : pb.conflictAdj[e1])
        {
            if (ex == e2)
                continue;
            if (timeSlot[ex] == t1)
                gain++; // e1 leaves t1
            if (timeSlot[ex] == t2)
                gain--; // e1 enters t2
        }
        for (int ex : pb.conflictAdj[e2])
        {
            if (ex == e1)
                continue;
            if (timeSlot[ex] == t2)
                gain++; // e2 leaves t2
            if (timeSlot[ex] == t1)
                gain--; // e2 enters t1
        }
        // e1-e2 conflict: was in different slots (no violation), still different slots
        // So no net effect from the e1-e2 pair
        return gain;
    }

    // Apply room matching for all slots
    void assignRooms(const Problem &pb)
    {
        room.assign(pb.n, -1);
        for (int t = 0; t < pb.p; t++)
        {
            if (slotEvents[t].empty())
                continue;
            auto assigned = Matching::assignRooms(slotEvents[t], pb);
            for (int i = 0; i < (int)slotEvents[t].size(); i++)
                room[slotEvents[t][i]] = assigned[i];
        }
    }

    // Full evaluation of all objectives and HCV
    void evaluate(const Problem &pb)
    {
        hcv = 0;
        f1 = 0;
        f2 = 0;
        f3 = 0;

        // Rebuild fast state if needed
        if ((int)slotEvents.size() != pb.p)
            buildFastState(pb);

        // H1: student conflicts
        hcv += totalH1;

        // H2: room suitability
        for (int e = 0; e < pb.n; e++)
        {
            if (room[e] == -1 || pb.eventRoom[e][room[e]] == 0)
                hcv++;
        }

        // H3: two events in same slot sharing same room
        for (int t = 0; t < pb.p; t++)
        {
            const auto &evts = slotEvents[t];
            for (int i = 0; i < (int)evts.size(); i++)
            {
                for (int j = i + 1; j < (int)evts.size(); j++)
                {
                    int ri = room[evts[i]], rj = room[evts[j]];
                    if (ri != -1 && rj != -1 && ri == rj)
                        hcv++;
                }
            }
        }

        // Soft constraints
        const int slotsPerDay = 9;
        const int days = 5;

        // Build per-student slot lists (deduplicated and sorted)
        for (int s = 0; s < pb.k; s++)
        {
            std::vector<int> slots;
            slots.reserve(16);
            for (int e = 0; e < pb.n; e++)
                if (pb.studentEvent[s][e])
                    slots.push_back(timeSlot[e]);
            std::sort(slots.begin(), slots.end());
            slots.erase(std::unique(slots.begin(), slots.end()), slots.end()); // dedup for infeasible

            // S1: event in last slot of day
            for (int t : slots)
                if (t % slotsPerDay == slotsPerDay - 1)
                    f1++;

            // Per-day analysis
            for (int d = 0; d < days; d++)
            {
                int ds = d * slotsPerDay;
                int de = ds + slotsPerDay;
                std::vector<int> daySlots;
                for (int t : slots)
                    if (t >= ds && t < de)
                        daySlots.push_back(t);

                // S3: single event in a day
                if ((int)daySlots.size() == 1)
                {
                    f3++;
                    continue;
                }

                // S2: more than 2 consecutive
                if ((int)daySlots.size() >= 2)
                {
                    int cons = 1;
                    for (int i = 1; i < (int)daySlots.size(); i++)
                    {
                        if (daySlots[i] == daySlots[i - 1] + 1)
                        {
                            cons++;
                            if (cons > 2)
                                f2++;
                        }
                        else
                        {
                            cons = 1;
                        }
                    }
                }
            }
        }
    }

    // Initialize with random time slots
    static Individual random(const Problem &pb, std::mt19937 &rng)
    {
        Individual ind;
        ind.timeSlot.resize(pb.n);
        std::uniform_int_distribution<int> slotDist(0, pb.p - 1);
        for (int e = 0; e < pb.n; e++)
            ind.timeSlot[e] = slotDist(rng);
        ind.buildFastState(pb);
        ind.assignRooms(pb);
        ind.evaluate(pb);
        ind.rank = 0;
        ind.crowdingDist = 0.0;
        ind.domCount = 0;
        return ind;
    }

    // Initialize with a greedy approach: assign events to slots minimizing conflicts
    static Individual greedy(const Problem &pb, std::mt19937 &rng)
    {
        Individual ind;
        ind.timeSlot.resize(pb.n, -1);

        // Shuffle events and assign greedily to slot with fewest conflicts
        std::vector<int> order(pb.n);
        std::iota(order.begin(), order.end(), 0);
        // Sort by degree descending (hardest events first)
        std::sort(order.begin(), order.end(), [&](int a, int b)
                  { return pb.conflictAdj[a].size() > pb.conflictAdj[b].size(); });
        // Add random jitter
        int blockSize = std::max(1, pb.n / 10);
        for (int i = 0; i < pb.n; i += blockSize)
        {
            int end = std::min(i + blockSize, pb.n);
            std::shuffle(order.begin() + i, order.begin() + end, rng);
        }

        // slotLoad[t] = how many events already placed in slot t
        std::vector<int> slotLoad(pb.p, 0);
        // Track which events are in each slot for conflict checking
        std::vector<std::vector<int>> slotOccupants(pb.p);

        for (int e : order)
        {
            // Find the slot with lowest conflict addition and room availability
            int bestSlot = 0;
            int bestScore = INT_MAX;
            // Sample a subset of slots for speed on large instances
            int maxCheck = std::min(pb.p, pb.p); // check all 45 slots

            for (int t = 0; t < maxCheck; t++)
            {
                // Skip slots that are already full (more events than rooms)
                if (slotLoad[t] >= pb.m)
                    continue;

                // Count H1 violations this event would add
                int conflicts = 0;
                for (int e2 : slotOccupants[t])
                {
                    if (pb.eventConflict[e][e2])
                        conflicts++;
                }

                // Score: (conflicts * 1000) + load (prefer less loaded slots)
                int score = conflicts * 1000 + slotLoad[t];
                if (score < bestScore)
                {
                    bestScore = score;
                    bestSlot = t;
                }
            }

            ind.timeSlot[e] = bestSlot;
            slotLoad[bestSlot]++;
            slotOccupants[bestSlot].push_back(e);
        }

        ind.buildFastState(pb);
        ind.assignRooms(pb);
        ind.evaluate(pb);
        ind.rank = 0;
        ind.crowdingDist = 0.0;
        ind.domCount = 0;
        return ind;
    }

    void print(int idx = -1) const
    {
        if (idx >= 0)
            std::cout << "Solution[" << idx << "] ";
        std::cout << "rank=" << rank
                  << " HCV=" << hcv
                  << " f1=" << f1 << " f2=" << f2 << " f3=" << f3
                  << " crowdDist=" << crowdingDist << "\n";
    }
};
