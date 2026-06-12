#include "mem.h"
#include <algorithm>
#include <random>

using namespace std;

void MEM::construct(const vector<Individual>& population,
                    double alpha,
                    const Evaluator& eval) {
    for (auto& l : lists) l.clear();

    vector<const Individual*> sorted;
    sorted.reserve(population.size());
    for (const auto& ind : population)
        sorted.push_back(&ind);

    sort(sorted.begin(), sorted.end(),
              [](const Individual* a, const Individual* b) {
                  return a->fitness < b->fitness;
              });

    int Q = max(1, (int)(alpha * (int)population.size()));

    for (int idx = 0; idx < Q; ++idx) {
        const Individual& ind = *sorted[idx];

        // build tsIndex once per individual
        vector<vector<int>> tsIdx(prob.numTimeslots);
        for (int e = 0; e < prob.numEvents; ++e) {
            int ts = ind.assignment[e].timeslot;
            if (ts >= 0 && ts < prob.numTimeslots)
                tsIdx[ts].push_back(e);
        }

        for (int e : prob.E0_events) {
            if (eventPenaltyFast(ind, e, tsIdx) == 0) {
                RoomTimePair p{ind.assignment[e].room,
                               ind.assignment[e].timeslot};
                // avoid duplicates
                auto& lst = lists[e];
                if (find(lst.begin(), lst.end(), p) == lst.end())
                    lst.push_back(p);
            }
        }
    }
}

RoomTimePair MEM::randomPairFor(int eventId) const {
    const auto& lst = lists[eventId];
    if (lst.empty()) return {-1, -1};
    uniform_int_distribution<int> dist(0, (int)lst.size() - 1);
    return lst[dist(rng)];
}

int MEM::eventPenaltyFast(const Individual& ind, int e,
                           const vector<vector<int>>& tsIdx) const {
    int pen = 0;
    const auto& a = ind.assignment[e];

    // H2
    const auto& sr = prob.events[e].suitableRooms;
    if (find(sr.begin(), sr.end(), a.room) == sr.end())
        ++pen;

    // H4
    if (prob.events[e].hasTimeRestriction) {
        const auto& avail = prob.events[e].availableTimeSlots;
        if (find(avail.begin(), avail.end(), a.timeslot) == avail.end())
            ++pen;
    }

    // H1 + H3: only same timeslot via index
    if (a.timeslot >= 0 && a.timeslot < prob.numTimeslots) {
        for (int f : tsIdx[a.timeslot]) {
            if (f == e) continue;
            if (prob.conflict[e][f])              ++pen;
            if (ind.assignment[f].room == a.room) ++pen;
        }
    }

    return pen;
}