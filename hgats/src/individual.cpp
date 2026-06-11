#include "individual.h"
#include <algorithm>

int Evaluator::evaluate(Individual& ind) const {
    ind.distanceToFeasibility = countHardViolations(ind);
    ind.softConstraintPenalty = countSoftViolations(ind);
    ind.fitness = ind.distanceToFeasibility * Individual::BIG
                + ind.softConstraintPenalty;
    return ind.fitness;
}

// Optimized: build timeslot index first, then check only same-timeslot pairs
int Evaluator::countHardViolations(const Individual& ind) const {
    int violations = 0;

    // Build timeslot → events index
    std::vector<std::vector<int>> tsEvents(prob.numTimeslots);
    for (int e = 0; e < prob.numEvents; ++e) {
        int ts = ind.assignment[e].timeslot;
        if (ts >= 0 && ts < prob.numTimeslots)
            tsEvents[ts].push_back(e);
    }

    for (int e = 0; e < prob.numEvents; ++e) {
        const auto& a = ind.assignment[e];
        if (a.timeslot < 0 || a.room < 0) {
            ++violations;
            continue;
        }

        // H2: room must be suitable
        const auto& sr = prob.events[e].suitableRooms;
        if (std::find(sr.begin(), sr.end(), a.room) == sr.end())
            ++violations;

        // H4: timeslot must be available
        if (!checkH4(ind, e)) ++violations;

        // H5: precedence
        if (!checkH5(ind, e)) ++violations;
    }

    // H1 + H3: only check events in the SAME timeslot
    for (int ts = 0; ts < prob.numTimeslots; ++ts) {
        const auto& evts = tsEvents[ts];
        for (int i = 0; i < (int)evts.size(); ++i) {
            for (int j = i + 1; j < (int)evts.size(); ++j) {
                int ei = evts[i], ej = evts[j];
                if (ind.assignment[ei].room == ind.assignment[ej].room)
                    ++violations;  // H1
                if (prob.conflict[ei][ej])
                    ++violations;  // H3
            }
        }
    }

    return violations;
}

// Per-event hard penalty (for delta-evaluate)
int Evaluator::eventHardPenalty(const Individual& ind, int e) const {
    int pen = 0;
    const auto& a = ind.assignment[e];

    if (a.timeslot < 0 || a.room < 0) return 1;

    // H2
    const auto& sr = prob.events[e].suitableRooms;
    if (std::find(sr.begin(), sr.end(), a.room) == sr.end())
        ++pen;

    // H4
    if (!checkH4(ind, e)) ++pen;

    // H5
    if (!checkH5(ind, e)) ++pen;

    // H1 + H3: only events in same timeslot
    for (int f = 0; f < prob.numEvents; ++f) {
        if (f == e) continue;
        if (ind.assignment[f].timeslot == a.timeslot) {
            if (ind.assignment[f].room == a.room) ++pen;   // H1
            if (prob.conflict[e][f]) ++pen;                 // H3
        }
    }

    // H5 reverse: events that must come before e
    for (int f = 0; f < prob.numEvents; ++f) {
        if (f == e) continue;
        for (int after : prob.events[f].mustBeBefore) {
            if (after == e && ind.assignment[f].timeslot >= a.timeslot)
                ++pen;
        }
    }

    return pen;
}

// True delta-evaluate: only recalculate what changes
int Evaluator::deltaEvaluate(const Individual& ind,
                               int eventId,
                               int newTs,
                               int newRoom) const {
    // Calculate current penalty contribution of this event
    int oldPen = eventHardPenalty(ind, eventId);

    // Also count violations of OTHER events that conflict with this event
    // at the OLD timeslot (these will be resolved by moving)
    int oldOtherPen = 0;
    int oldTs = ind.assignment[eventId].timeslot;
    int oldRm = ind.assignment[eventId].room;
    if (oldTs >= 0) {
        for (int f = 0; f < prob.numEvents; ++f) {
            if (f == eventId) continue;
            if (ind.assignment[f].timeslot == oldTs) {
                if (ind.assignment[f].room == oldRm) ++oldOtherPen; // H1 counted from f's side
                if (prob.conflict[eventId][f]) ++oldOtherPen;       // H3 counted from f's side
            }
        }
    }

    // Temporarily apply the move
    Individual tmp = ind;
    tmp.assignment[eventId].timeslot = newTs;
    tmp.assignment[eventId].room     = newRoom;

    // Calculate new penalty contribution
    int newPen = eventHardPenalty(tmp, eventId);

    int newOtherPen = 0;
    for (int f = 0; f < prob.numEvents; ++f) {
        if (f == eventId) continue;
        if (tmp.assignment[f].timeslot == newTs) {
            if (tmp.assignment[f].room == newRoom) ++newOtherPen;
            if (prob.conflict[eventId][f]) ++newOtherPen;
        }
    }

    int dfDelta = (newPen + newOtherPen) - (oldPen + oldOtherPen);
    int newDf = ind.distanceToFeasibility + dfDelta;
    if (newDf < 0) newDf = 0;

    // SCP only matters if feasible
    int newScp = 0;
    if (newDf == 0) {
        // Full SCP calculation needed (rare case)
        evaluate(tmp);
        return tmp.fitness;
    }

    return newDf * Individual::BIG + newScp;
}

int Evaluator::countSoftViolations(const Individual& ind) const {
    if (ind.distanceToFeasibility > 0) return 0;

    const int SLOTS_PER_DAY = 9;
    int penalty = 0;

    std::vector<std::vector<std::vector<int>>>
        studentDaySlots(prob.numStudents,
            std::vector<std::vector<int>>(5));

    for (int e = 0; e < prob.numEvents; ++e) {
        int ts = ind.assignment[e].timeslot;
        if (ts < 0) continue;
        int day  = ts / SLOTS_PER_DAY;
        int slot = ts % SLOTS_PER_DAY;
        for (int s : prob.events[e].students)
            studentDaySlots[s][day].push_back(slot);
    }

    for (int s = 0; s < prob.numStudents; ++s) {
        for (int d = 0; d < 5; ++d) {
            auto& slots = studentDaySlots[s][d];
            if (slots.empty()) continue;
            std::sort(slots.begin(), slots.end());

            // S1: last slot of day
            if (slots.back() == SLOTS_PER_DAY - 1)
                ++penalty;

            // S2: more than 2 consecutive
            int consecutive = 1;
            for (int i = 1; i < (int)slots.size(); ++i) {
                if (slots[i] == slots[i-1] + 1) {
                    ++consecutive;
                    if (consecutive > 2) ++penalty;
                } else {
                    consecutive = 1;
                }
            }

            // S3: single class on a day
            if ((int)slots.size() == 1) ++penalty;
        }
    }

    return penalty;
}

bool Evaluator::checkH4(const Individual& ind, int e) const {
    const auto& avail = prob.events[e].availableTimeSlots;
    if (avail.empty()) return true;
    int ts = ind.assignment[e].timeslot;
    return std::find(avail.begin(), avail.end(), ts) != avail.end();
}

bool Evaluator::checkH5(const Individual& ind, int e) const {
    int ts_e = ind.assignment[e].timeslot;
    for (int after : prob.events[e].mustBeBefore) {
        int ts_after = ind.assignment[after].timeslot;
        if (ts_e >= ts_after) return false;
    }
    return true;
}
