#pragma once
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
#include "individual.h"
#include "problem.h"

class LocalSearch
{
public:
    // LS5: Fix hard constraints first, then reduce soft constraints
    // Uses fast incremental H1 tracking (O(degree) per move)
    static void ls5(Individual &ind, const Problem &pb, int smax, std::mt19937 &rng)
    {
        // Ensure fast state is built
        if ((int)ind.slotEvents.size() != pb.p)
            ind.buildFastState(pb);

        // Phase 1: Fix hard constraint violations (H1 focus)
        if (ind.totalH1 > 0)
        {
            repairH1(ind, pb, smax, rng);
        }

        // Re-assign rooms after slot changes
        ind.assignRooms(pb);
        ind.evaluate(pb);

        // Phase 2: Reduce soft constraints (only if feasible)
        if (ind.hcv == 0)
        {
            reduceSoft(ind, pb, smax, rng);
        }
    }

    // LS2: Select worst time slot from random sample, apply N1 moves
    static void ls2(Individual &ind, const Problem &pb, double tp, std::mt19937 &rng)
    {
        int sampleSize = std::max(1, (int)(tp * pb.p));
        std::vector<int> allSlots(pb.p);
        std::iota(allSlots.begin(), allSlots.end(), 0);
        std::shuffle(allSlots.begin(), allSlots.end(), rng);
        allSlots.resize(sampleSize);

        // Select worst slot by total penalty
        int worstSlot = -1;
        int worstPenalty = -1;
        for (int t : allSlots)
        {
            int pen = 0;
            for (int e : ind.slotEvents[t])
            {
                pen += ind.h1Count[e] * 1000;
                // Room penalty
                if (ind.room[e] == -1 || pb.eventRoom[e][ind.room[e]] == 0)
                    pen += 1000;
            }
            if (pen > worstPenalty)
            {
                worstPenalty = pen;
                worstSlot = t;
            }
        }

        if (worstSlot < 0 || ind.slotEvents[worstSlot].empty())
            return;

        // Try N1 moves for all events in worst slot simultaneously
        std::vector<int> targets; // new slot for each event in worst slot
        bool anyImprove = false;
        std::vector<int> eventsInSlot = ind.slotEvents[worstSlot]; // copy

        for (int e : eventsInSlot)
        {
            int bestT = ind.timeSlot[e];
            int bestGain = 0;
            for (int t = 0; t < pb.p; t++)
            {
                if (t == ind.timeSlot[e])
                    continue;
                int g = ind.deltaH1Move(e, t, pb);
                if (g > bestGain && (int)ind.slotEvents[t].size() < pb.m)
                {
                    bestGain = g;
                    bestT = t;
                }
            }
            targets.push_back(bestT);
            if (bestT != ind.timeSlot[e])
                anyImprove = true;
        }

        if (anyImprove)
        {
            for (int i = 0; i < (int)eventsInSlot.size(); i++)
            {
                if (targets[i] != ind.timeSlot[eventsInSlot[i]])
                    ind.fastMoveEvent(eventsInSlot[i], targets[i], pb);
            }
            ind.assignRooms(pb);
            ind.evaluate(pb);
        }
    }

    // Combined: LS5 then LS2
    static void applyLS(Individual &ind, const Problem &pb, int smax, std::mt19937 &rng)
    {
        ls5(ind, pb, smax, rng);
        ls2(ind, pb, 0.25, rng);
        if (ind.hcv > 0)
        {
            // Second pass if still infeasible
            ls5(ind, pb, smax / 2, rng);
        }
    }

private:
    // Phase 1: Fix H1 violations using fast incremental moves
    static void repairH1(Individual &ind, const Problem &pb, int smax, std::mt19937 &rng)
    {
        const int maxPasses = 20;
        for (int pass = 0; pass < maxPasses && ind.totalH1 > 0; pass++)
        {
            bool improved = false;

            // Collect events with H1 violations, sorted by violation count (worst first)
            std::vector<int> violEvents;
            violEvents.reserve(pb.n);
            for (int e = 0; e < pb.n; e++)
                if (ind.h1Count[e] > 0)
                    violEvents.push_back(e);

            std::sort(violEvents.begin(), violEvents.end(), [&](int a, int b)
                      { return ind.h1Count[a] > ind.h1Count[b]; });

            int steps = 0;
            for (int e : violEvents)
            {
                if (ind.h1Count[e] == 0)
                    continue; // already fixed
                if (steps >= smax)
                    break;

                // Try N1: find best slot to move e to (max H1 gain, respect room capacity)
                int bestGain = 0, bestSlot = -1;
                for (int t = 0; t < pb.p && steps < smax; t++, steps++)
                {
                    if (t == ind.timeSlot[e])
                        continue;
                    // Don't move to a slot that's already at room capacity
                    if ((int)ind.slotEvents[t].size() >= pb.m)
                        continue;
                    int gain = ind.deltaH1Move(e, t, pb);
                    if (gain > bestGain)
                    {
                        bestGain = gain;
                        bestSlot = t;
                    }
                }

                if (bestSlot != -1 && bestGain > 0)
                {
                    ind.fastMoveEvent(e, bestSlot, pb);
                    improved = true;
                    continue;
                }

                // N2: try swapping with a conflicting event
                for (int e2 : pb.conflictAdj[e])
                {
                    if (ind.timeSlot[e2] != ind.timeSlot[e])
                        continue; // not in same slot
                    steps++;
                    if (steps >= smax)
                        break;

                    // Also try moving e2 to a free slot
                    int bestMoveGain = 0, bestMoveSlot = -1;
                    for (int t = 0; t < pb.p; t++)
                    {
                        if (t == ind.timeSlot[e2])
                            continue;
                        if ((int)ind.slotEvents[t].size() >= pb.m)
                            continue;
                        int mg = ind.deltaH1Move(e2, t, pb);
                        if (mg > bestMoveGain)
                        {
                            bestMoveGain = mg;
                            bestMoveSlot = t;
                        }
                    }
                    if (bestMoveSlot != -1 && bestMoveGain > 0)
                    {
                        ind.fastMoveEvent(e2, bestMoveSlot, pb);
                        improved = true;
                        break;
                    }
                }
            }

            if (!improved)
            {
                // Perturbation: randomly move a few violating events
                perturbViolating(ind, pb, rng);
            }
        }
    }

    // Perturbation: randomly move events with violations to low-load slots
    static void perturbViolating(Individual &ind, const Problem &pb, std::mt19937 &rng)
    {
        std::vector<int> violEvents;
        for (int e = 0; e < pb.n; e++)
            if (ind.h1Count[e] > 0)
                violEvents.push_back(e);
        if (violEvents.empty())
            return;

        // Move up to 5 random violating events to least-loaded slot
        int numToMove = std::min((int)violEvents.size(), 5);
        std::shuffle(violEvents.begin(), violEvents.end(), rng);

        // Find slots with low load
        std::vector<int> slotsByLoad(pb.p);
        std::iota(slotsByLoad.begin(), slotsByLoad.end(), 0);
        std::sort(slotsByLoad.begin(), slotsByLoad.end(), [&](int a, int b)
                  { return ind.slotEvents[a].size() < ind.slotEvents[b].size(); });

        for (int i = 0; i < numToMove; i++)
        {
            int e = violEvents[i];
            // Pick from top-10 least loaded slots randomly
            int pick = std::min((int)slotsByLoad.size(), 10);
            std::uniform_int_distribution<int> sd(0, pick - 1);
            int t = slotsByLoad[sd(rng)];
            if ((int)ind.slotEvents[t].size() < pb.m)
                ind.fastMoveEvent(e, t, pb);
        }
    }

    // Phase 2: Reduce soft constraints using N1/N2 moves (no H1 worsening)
    static void reduceSoft(Individual &ind, const Problem &pb, int smax, std::mt19937 &rng)
    {
        const int slotsPerDay = 9;
        const int days = 5;

        int steps = 0;
        bool improved = true;
        while (improved && steps < smax)
        {
            improved = false;

            // Try N1 moves for all events
            // Order events randomly for diversity
            std::vector<int> order(pb.n);
            std::iota(order.begin(), order.end(), 0);
            std::shuffle(order.begin(), order.end(), rng);

            for (int e : order)
            {
                if (steps >= smax)
                    break;
                int bestGain = 0, bestSlot = -1;

                for (int t = 0; t < pb.p; t++, steps++)
                {
                    if (t == ind.timeSlot[e])
                        continue;
                    // Must not worsen H1
                    int deltaH = ind.deltaH1Move(e, t, pb);
                    if (deltaH < 0)
                        continue;
                    // Must not exceed room capacity
                    if ((int)ind.slotEvents[t].size() >= pb.m)
                        continue;

                    // Compute soft gain: temporarily move and check
                    // Fast approximation: check S1 change for this event's students
                    int s1Gain = softS1Delta(e, t, pb, ind, slotsPerDay);
                    // For S2 and S3, do quick estimate
                    int s23Gain = softS23Delta(e, ind.timeSlot[e], t, pb, ind, slotsPerDay, days);

                    int gain = s1Gain + s23Gain;
                    if (gain > bestGain)
                    {
                        bestGain = gain;
                        bestSlot = t;
                    }
                }

                if (bestSlot != -1 && bestGain > 0)
                {
                    ind.fastMoveEvent(e, bestSlot, pb);
                    improved = true;
                }
            }
        }

        ind.assignRooms(pb);
        ind.evaluate(pb);
    }

    // Quick S1 delta: change in last-slot violations for event e moving from old to new slot
    static int softS1Delta(int e, int newSlot, const Problem &pb,
                           const Individual &ind, int slotsPerDay)
    {
        int oldSlot = ind.timeSlot[e];
        bool oldLast = (oldSlot % slotsPerDay == slotsPerDay - 1);
        bool newLast = (newSlot % slotsPerDay == slotsPerDay - 1);
        if (oldLast == newLast)
            return 0;
        int numStudents = (int)pb.eventStudents[e].size();
        return oldLast ? numStudents : -numStudents; // positive = improvement
    }

    // Quick S2/S3 delta estimate for students attending event e
    static int softS23Delta(int e, int oldSlot, int newSlot,
                            const Problem &pb, const Individual &ind,
                            int slotsPerDay, int /*days*/)
    {
        int gain = 0;
        int oldDay = oldSlot / slotsPerDay;
        int newDay = newSlot / slotsPerDay;
        if (oldDay == newDay)
            return 0;

        for (int s : pb.eventStudents[e])
        {
            // Count events per day for old and new day (excluding event e)
            int oldDayCount = 0, newDayCount = 0;
            for (int e2 = 0; e2 < pb.n; e2++)
            {
                if (e2 == e || !pb.studentEvent[s][e2])
                    continue;
                int t2 = ind.timeSlot[e2];
                if (t2 / slotsPerDay == oldDay)
                    oldDayCount++;
                if (t2 / slotsPerDay == newDay)
                    newDayCount++;
            }
            // S3: single event per day
            // Old day: was 1 (with e), becomes 0 (without e) -> fixes S3 if was 1
            if (oldDayCount == 0)
                gain++; // was single event day -> no longer
            if (newDayCount == 0)
                gain--; // new day becomes single event day

            // S3 at new day: was isolated, now has more
            if (newDayCount == 1)
                gain++; // new day had 1, now has 2 -> fixes isolation
        }
        return gain;
    }
};
