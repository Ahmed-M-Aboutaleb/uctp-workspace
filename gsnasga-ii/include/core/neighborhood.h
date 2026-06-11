#pragma once
#include <vector>
#include <random>
#include <algorithm>
#include "individual.h"
#include "problem.h"

// Neighborhood structures N1, N2, N3, N7
// N1: move an event to a different time slot
// N2: swap the time slots of two events
// N3: cyclic exchange of three events' time slots
// N7: swap time slots of two consecutive events with time slots of another two consecutive events

struct Move
{
    enum Type
    {
        N1,
        N2,
        N3,
        N7
    };
    Type type;
    int e1, e2, e3, e4; // events involved
    int t1, t2, t3, t4; // new time slots
};

class Neighborhood
{
public:
    // Generate all N1 moves for event e (try all other time slots)
    static std::vector<Move> n1Moves(int e, const Individual &ind, const Problem &pb)
    {
        std::vector<Move> moves;
        for (int t = 0; t < pb.p; t++)
        {
            if (t != ind.timeSlot[e])
            {
                Move m;
                m.type = Move::N1;
                m.e1 = e;
                m.t1 = t;
                moves.push_back(m);
            }
        }
        return moves;
    }

    // Generate N2 moves for event e: swap with another event
    static std::vector<Move> n2Moves(int e, const Individual &ind, const Problem &pb)
    {
        std::vector<Move> moves;
        for (int e2 = 0; e2 < pb.n; e2++)
        {
            if (e2 != e && ind.timeSlot[e2] != ind.timeSlot[e])
            {
                Move m;
                m.type = Move::N2;
                m.e1 = e;
                m.e2 = e2;
                m.t1 = ind.timeSlot[e2]; // e1 gets e2's slot
                m.t2 = ind.timeSlot[e];  // e2 gets e1's slot
                moves.push_back(m);
            }
        }
        return moves;
    }

    // Generate N3 moves for event e: cyclic exchange with next two events
    static std::vector<Move> n3Moves(int e, const Individual &ind, const Problem &pb)
    {
        std::vector<Move> moves;
        for (int e2 = 0; e2 < pb.n; e2++)
        {
            if (e2 == e)
                continue;
            for (int e3 = e2 + 1; e3 < pb.n; e3++)
            {
                if (e3 == e)
                    continue;
                // Cyclic: e gets e2's slot, e2 gets e3's slot, e3 gets e's slot
                Move m;
                m.type = Move::N3;
                m.e1 = e;
                m.e2 = e2;
                m.e3 = e3;
                m.t1 = ind.timeSlot[e2];
                m.t2 = ind.timeSlot[e3];
                m.t3 = ind.timeSlot[e];
                moves.push_back(m);
            }
        }
        return moves;
    }

    // N7: swap two consecutive events with another two consecutive events
    static std::vector<Move> n7Moves(int e, const Individual &ind, const Problem &pb)
    {
        std::vector<Move> moves;
        // e is treated as one of the pair
        int e2 = (e + 1) % pb.n;
        for (int e3 = 0; e3 < pb.n; e3++)
        {
            if (e3 == e || e3 == e2)
                continue;
            int e4 = (e3 + 1) % pb.n;
            if (e4 == e || e4 == e2)
                continue;

            Move m;
            m.type = Move::N7;
            m.e1 = e;
            m.e2 = e2;
            m.e3 = e3;
            m.e4 = e4;
            m.t1 = ind.timeSlot[e3]; // e gets e3's slot
            m.t2 = ind.timeSlot[e4]; // e2 gets e4's slot
            m.t3 = ind.timeSlot[e];  // e3 gets e's slot
            m.t4 = ind.timeSlot[e2]; // e4 gets e2's slot
            moves.push_back(m);
        }
        return moves;
    }

    // Apply a move to get new time slots (returns modified copy)
    static std::vector<int> applyMove(const Move &mv, const Individual &ind)
    {
        std::vector<int> ts = ind.timeSlot;
        switch (mv.type)
        {
        case Move::N1:
            ts[mv.e1] = mv.t1;
            break;
        case Move::N2:
            ts[mv.e1] = mv.t1;
            ts[mv.e2] = mv.t2;
            break;
        case Move::N3:
            ts[mv.e1] = mv.t1;
            ts[mv.e2] = mv.t2;
            ts[mv.e3] = mv.t3;
            break;
        case Move::N7:
            ts[mv.e1] = mv.t1;
            ts[mv.e2] = mv.t2;
            ts[mv.e3] = mv.t3;
            ts[mv.e4] = mv.t4;
            break;
        }
        return ts;
    }

    // Delta-evaluate hard constraint violations for affected events
    // Returns delta_hcv (positive = improvement, negative = worsening)
    static int deltaHCV(const Move &mv, const Individual &ind, const Problem &pb)
    {
        // Compute old and new HCV contribution from affected events
        std::vector<int> newTs = applyMove(mv, ind);

        // Determine affected events
        std::vector<int> affected;
        affected.push_back(mv.e1);
        if (mv.type == Move::N2 || mv.type == Move::N3 || mv.type == Move::N7)
            affected.push_back(mv.e2);
        if (mv.type == Move::N3 || mv.type == Move::N7)
            affected.push_back(mv.e3);
        if (mv.type == Move::N7)
            affected.push_back(mv.e4);

        // Old HCV contribution
        int oldHCV = 0, newHCV = 0;
        for (int e : affected)
        {
            int oldT = ind.timeSlot[e];
            int newT = newTs[e];

            // Conflicts with non-affected events
            for (int e2 = 0; e2 < pb.n; e2++)
            {
                bool e2Affected = false;
                for (int a : affected)
                    if (a == e2)
                    {
                        e2Affected = true;
                        break;
                    }
                if (e2Affected)
                    continue;

                if (pb.eventConflict[e][e2])
                {
                    if (ind.timeSlot[e2] == oldT)
                        oldHCV++;
                    if (ind.timeSlot[e2] == newT)
                        newHCV++;
                }
            }
        }
        // Conflicts among affected events
        for (int i = 0; i < (int)affected.size(); i++)
        {
            for (int j = i + 1; j < (int)affected.size(); j++)
            {
                int e1 = affected[i], e2 = affected[j];
                if (pb.eventConflict[e1][e2])
                {
                    if (ind.timeSlot[e1] == ind.timeSlot[e2])
                        oldHCV++;
                    if (newTs[e1] == newTs[e2])
                        newHCV++;
                }
            }
        }

        return oldHCV - newHCV; // positive = reduces violations
    }

    // Delta-evaluate soft constraints for affected events
    static int deltaSCV(const Move &mv, const Individual &ind, const Problem &pb)
    {
        std::vector<int> newTs = applyMove(mv, ind);
        std::vector<int> affected;
        affected.push_back(mv.e1);
        if (mv.type == Move::N2 || mv.type == Move::N3 || mv.type == Move::N7)
            affected.push_back(mv.e2);
        if (mv.type == Move::N3 || mv.type == Move::N7)
            affected.push_back(mv.e3);
        if (mv.type == Move::N7)
            affected.push_back(mv.e4);

        const int slotsPerDay = 9;
        const int days = 5;

        // Collect students affected by these events
        std::vector<bool> affectedStudent(pb.k, false);
        for (int e : affected)
        {
            for (int s = 0; s < pb.k; s++)
            {
                if (pb.studentEvent[s][e])
                    affectedStudent[s] = true;
            }
        }

        auto computeStudentSoft = [&](const std::vector<int> &ts)
        {
            int sc = 0;
            for (int s = 0; s < pb.k; s++)
            {
                if (!affectedStudent[s])
                    continue;
                std::vector<int> slots;
                for (int e = 0; e < pb.n; e++)
                {
                    if (pb.studentEvent[s][e])
                        slots.push_back(ts[e]);
                }
                std::sort(slots.begin(), slots.end());
                // S1
                for (int t : slots)
                {
                    if (t % slotsPerDay == slotsPerDay - 1)
                        sc++;
                }
                // S2
                for (int d = 0; d < days; d++)
                {
                    int ds = d * slotsPerDay;
                    int de = ds + slotsPerDay;
                    std::vector<int> ds2;
                    for (int t : slots)
                        if (t >= ds && t < de)
                            ds2.push_back(t);
                    std::sort(ds2.begin(), ds2.end());
                    if ((int)ds2.size() > 2)
                    {
                        int cons = 1;
                        for (int i = 1; i < (int)ds2.size(); i++)
                        {
                            if (ds2[i] == ds2[i - 1] + 1)
                            {
                                cons++;
                                if (cons > 2)
                                    sc++;
                            }
                            else
                            {
                                cons = 1;
                            }
                        }
                    }
                }
                // S3
                for (int d = 0; d < days; d++)
                {
                    int ds = d * slotsPerDay;
                    int de = ds + slotsPerDay;
                    int cnt = 0;
                    for (int t : slots)
                        if (t >= ds && t < de)
                            cnt++;
                    if (cnt == 1)
                        sc++;
                }
            }
            return sc;
        };

        int oldSCV = computeStudentSoft(ind.timeSlot);
        int newSCV = computeStudentSoft(newTs);
        return oldSCV - newSCV; // positive = reduces violations
    }
};
