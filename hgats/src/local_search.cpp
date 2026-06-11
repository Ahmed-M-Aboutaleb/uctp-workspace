#include "local_search.h"
#include <random>
#include <algorithm>
#include <numeric>
#include <set>
#include <functional>

static std::mt19937 rng_ls(1234);

// ═══════════════════════════════════════════════════════════════
//  Timeslot Index
// ═══════════════════════════════════════════════════════════════

void LocalSearch::buildIndex(const Individual& ind) {
    tsEvents.assign(prob.numTimeslots, {});
    for (int e = 0; e < prob.numEvents; ++e) {
        int ts = ind.assignment[e].timeslot;
        if (ts >= 0 && ts < prob.numTimeslots)
            tsEvents[ts].push_back(e);
    }
}

void LocalSearch::moveEventInIndex(int e, int oldTs, int newTs) {
    if (oldTs >= 0 && oldTs < prob.numTimeslots) {
        auto& v = tsEvents[oldTs];
        v.erase(std::remove(v.begin(), v.end(), e), v.end());
    }
    if (newTs >= 0 && newTs < prob.numTimeslots)
        tsEvents[newTs].push_back(e);
}

// ═══════════════════════════════════════════════════════════════
//  Room Assignment - Greedy (fast, first-fit)
// ═══════════════════════════════════════════════════════════════

int LocalSearch::matchRoomGreedy(const Individual& ind,
                                  int e, int ts,
                                  int exclude1, int exclude2) const {
    if (ts < 0 || ts >= prob.numTimeslots) return -1;
    bool taken[1000] = {};
    for (int f : tsEvents[ts]) {
        if (f == e || f == exclude1 || f == exclude2) continue;
        if (ind.assignment[f].room >= 0)
            taken[ind.assignment[f].room] = true;
    }
    for (int r : prob.events[e].suitableRooms)
        if (!taken[r] && prob.rooms[r].capacity >= prob.events[e].numStudents)
            return r;
    return -1;
}

// ═══════════════════════════════════════════════════════════════
//  Room Assignment - Maximum Bipartite Matching (Network Flow)
//
//  Paper: "For every timeslot there is a list of events taking
//  place in it and a preprocessed list of possible rooms to which
//  these events can be assigned. The matching algorithm gives the
//  maximum cardinality matching between rooms and events."
//  (Rossi-Doria et al.)
//
//  sideEffects: {event → newRoom} for events other than
//  target_event whose rooms change in the matching result.
// ═══════════════════════════════════════════════════════════════

int LocalSearch::matchRoomBipartite(const Individual& ind,
                                     int target_event, int ts,
                                     std::vector<std::pair<int,int>>& sideEffects) const {
    if (ts < 0 || ts >= prob.numTimeslots) return -1;
    const auto& evts = tsEvents[ts];
    int n = (int)evts.size();
    int m = prob.numRooms;
    if (n == 0) return -1;

    int target_idx = -1;
    std::vector<std::vector<int>> adj(n);
    for (int i = 0; i < n; ++i) {
        int f = evts[i];
        if (f == target_event) target_idx = i;
        for (int r : prob.events[f].suitableRooms)
            if (prob.rooms[r].capacity >= prob.events[f].numStudents)
                adj[i].push_back(r);
    }
    if (target_idx < 0) return -1;

    std::vector<int> matchRoom(m, -1);
    std::vector<int> matchEvent(n, -1);

    std::function<bool(int, std::vector<bool>&)> augment =
        [&](int u, std::vector<bool>& vis) -> bool {
            for (int r : adj[u]) {
                if (vis[r]) continue;
                vis[r] = true;
                if (matchRoom[r] == -1 || augment(matchRoom[r], vis)) {
                    matchRoom[r] = u;
                    matchEvent[u] = r;
                    return true;
                }
            }
            return false;
        };

    for (int i = 0; i < n; ++i) {
        std::vector<bool> vis(m, false);
        augment(i, vis);
    }

    // Collect side effects (rooms that changed for events other than target)
    sideEffects.clear();
    for (int i = 0; i < n; ++i) {
        int f    = evts[i];
        int newR = matchEvent[i];
        if (f != target_event && ind.assignment[f].room != newR)
            sideEffects.push_back({f, newR});
    }

    return matchEvent[target_idx];
}

// Combined: try greedy first; if it fails, fall back to bipartite
int LocalSearch::findBestRoom(const Individual& ind,
                               int e, int ts,
                               std::vector<std::pair<int,int>>& sideEffects,
                               int exclude1, int exclude2) const {
    sideEffects.clear();
    int r = matchRoomGreedy(ind, e, ts, exclude1, exclude2);
    if (r >= 0) return r;
    return matchRoomBipartite(ind, e, ts, sideEffects);
}

// ═══════════════════════════════════════════════════════════════
//  Public: assign rooms for ALL timeslots via maximum matching
//  Call this right after timeslot-only initialization.
// ═══════════════════════════════════════════════════════════════

void LocalSearch::assignRoomsViaMatching(Individual& ind) const {
    std::vector<std::vector<int>> tsIdx(prob.numTimeslots);
    for (int e = 0; e < prob.numEvents; ++e) {
        int ts = ind.assignment[e].timeslot;
        if (ts >= 0 && ts < prob.numTimeslots)
            tsIdx[ts].push_back(e);
    }

    for (int ts = 0; ts < prob.numTimeslots; ++ts) {
        const auto& evts = tsIdx[ts];
        if (evts.empty()) continue;
        int n = (int)evts.size();
        int m = prob.numRooms;

        std::vector<std::vector<int>> adj(n);
        for (int i = 0; i < n; ++i) {
            int e = evts[i];
            for (int r : prob.events[e].suitableRooms)
                if (prob.rooms[r].capacity >= prob.events[e].numStudents)
                    adj[i].push_back(r);
        }

        std::vector<int> matchRoom(m, -1);
        std::vector<int> matchEvent(n, -1);

        std::function<bool(int, std::vector<bool>&)> augment =
            [&](int u, std::vector<bool>& vis) -> bool {
                for (int r : adj[u]) {
                    if (vis[r]) continue;
                    vis[r] = true;
                    if (matchRoom[r] == -1 || augment(matchRoom[r], vis)) {
                        matchRoom[r] = u;
                        matchEvent[u] = r;
                        return true;
                    }
                }
                return false;
            };

        for (int i = 0; i < n; ++i) {
            std::vector<bool> vis(m, false);
            augment(i, vis);
        }

        for (int i = 0; i < n; ++i)
            ind.assignment[evts[i]].room = matchEvent[i];
    }
}

// ═══════════════════════════════════════════════════════════════
//  Per-event hard penalty (via timeslot index)
// ═══════════════════════════════════════════════════════════════

int LocalSearch::eventHardPenaltyFast(const Individual& ind, int e) const {
    int pen = 0;
    const auto& a = ind.assignment[e];
    if (a.timeslot < 0 || a.room < 0) return 1;

    const auto& sr = prob.events[e].suitableRooms;
    if (std::find(sr.begin(), sr.end(), a.room) == sr.end()) ++pen;

    if (prob.events[e].hasTimeRestriction) {
        const auto& avail = prob.events[e].availableTimeSlots;
        if (std::find(avail.begin(), avail.end(), a.timeslot) == avail.end())
            ++pen;
    }

    for (int after : prob.events[e].mustBeBefore)
        if (a.timeslot >= ind.assignment[after].timeslot) ++pen;

    for (int f : tsEvents[a.timeslot]) {
        if (f == e) continue;
        if (ind.assignment[f].room == a.room) ++pen;
        if (prob.conflict[e][f]) ++pen;
    }
    return pen;
}

bool LocalSearch::isInfeasibleFast(const Individual& ind, int e) const {
    return eventHardPenaltyFast(ind, e) > 0;
}

// ═══════════════════════════════════════════════════════════════
//  LS3 - Algorithm 20 (deterministic ordered scan, as in paper)
// ═══════════════════════════════════════════════════════════════

void LocalSearch::applyLS3(Individual& ind, int maxSteps) {
    eval.evaluate(ind);
    buildIndex(ind);
    int steps = 0;
    bool anyImprovement = true;

    while (anyImprovement && steps < maxSteps) {
        anyImprovement = false;

        // Phase 1: fix hard constraints
        for (int e = 0; e < prob.numEvents && steps < maxSteps; ++e) {
            if (!isInfeasibleFast(ind, e)) continue;

            bool moved = moveN1(ind, e, true) ||
                         moveN2(ind, e, true) ||
                         moveN3(ind, e, true) ||
                         moveN4(ind, e, true);
            if (moved) {
                anyImprovement = true;
                eval.evaluate(ind);
            }
            ++steps;
        }

        if (ind.distanceToFeasibility > 0) break;

        // Phase 2: reduce soft constraints
        for (int e = 0; e < prob.numEvents && steps < maxSteps; ++e) {
            if (!eventHasSoftViolation(ind, e)) continue;
            bool moved = moveN1(ind, e, false) ||
                         moveN2(ind, e, false);
            if (moved) {
                anyImprovement = true;
                eval.evaluate(ind);
            }
            ++steps;
        }
    }
}

// ═══════════════════════════════════════════════════════════════
//  LS4 - Algorithm 21
// ═══════════════════════════════════════════════════════════════

void LocalSearch::applyLS4(Individual& ind, int percentTS) {
    buildIndex(ind);
    for (int e : prob.EE)
        moveN5(ind, e);
    eval.evaluate(ind);
    buildIndex(ind);
    moveN6(ind, percentTS);
    eval.evaluate(ind);
}

// ═══════════════════════════════════════════════════════════════
//  N1: Move event to a different timeslot
//      Uses bipartite matching as fallback for room finding.
// ═══════════════════════════════════════════════════════════════

bool LocalSearch::moveN1(Individual& ind, int e, bool fixHard) {
    const auto& avail = prob.events[e].hasTimeRestriction
                        ? prob.events[e].availableTimeSlots
                        : prob.T0_slots;

    int oldPen = eventHardPenaltyFast(ind, e);
    int oldTs  = ind.assignment[e].timeslot;
    int oldRm  = ind.assignment[e].room;

    for (int ts : avail) {
        if (ts == oldTs) continue;

        // Save rooms of events currently in target ts (for revert)
        std::vector<std::pair<int,int>> savedRooms;
        for (int f : tsEvents[ts])
            savedRooms.push_back({f, ind.assignment[f].room});

        // Find room for e in ts (greedy first, bipartite fallback)
        std::vector<std::pair<int,int>> sideEffects;
        int r = findBestRoom(ind, e, ts, sideEffects);
        if (r < 0) continue;

        // Apply: move e to ts with room r
        ind.assignment[e] = {ts, r};
        moveEventInIndex(e, oldTs, ts);
        // Apply any room rearrangements from bipartite matching
        for (auto& [f, newR] : sideEffects)
            ind.assignment[f].room = newR;

        int newPen = eventHardPenaltyFast(ind, e);
        bool better = fixHard ? (newPen < oldPen) : (newPen <= oldPen);

        if (better) return true;

        // Revert: restore e
        ind.assignment[e] = {oldTs, oldRm};
        moveEventInIndex(e, ts, oldTs);
        // Restore rooms of all events that were in ts
        for (auto& [f, savedR] : savedRooms)
            ind.assignment[f].room = savedR;
    }
    return false;
}

// ═══════════════════════════════════════════════════════════════
//  N2: Swap timeslots of two events
// ═══════════════════════════════════════════════════════════════

bool LocalSearch::moveN2(Individual& ind, int e, bool fixHard) {
    int oldPenE = eventHardPenaltyFast(ind, e);
    int oldTsE  = ind.assignment[e].timeslot;
    int oldRmE  = ind.assignment[e].room;

    for (int f = 0; f < prob.numEvents; ++f) {
        if (f == e) continue;
        int oldTsF  = ind.assignment[f].timeslot;
        int oldRmF  = ind.assignment[f].room;
        if (oldTsE == oldTsF) continue;

        int oldPenF  = eventHardPenaltyFast(ind, f);
        int oldTotal = oldPenE + oldPenF;

        // Greedy for swap (exclude the swapping partner)
        int newRmE = matchRoomGreedy(ind, e, oldTsF, f);
        if (newRmE < 0) continue;
        int newRmF = matchRoomGreedy(ind, f, oldTsE, e);
        if (newRmF < 0) continue;

        ind.assignment[e] = {oldTsF, newRmE};
        ind.assignment[f] = {oldTsE, newRmF};
        moveEventInIndex(e, oldTsE, oldTsF);
        moveEventInIndex(f, oldTsF, oldTsE);

        int newTotal = eventHardPenaltyFast(ind, e) + eventHardPenaltyFast(ind, f);
        bool better  = fixHard ? (newTotal < oldTotal) : (newTotal <= oldTotal);

        if (better) return true;

        ind.assignment[e] = {oldTsE, oldRmE};
        ind.assignment[f] = {oldTsF, oldRmF};
        moveEventInIndex(e, oldTsF, oldTsE);
        moveEventInIndex(f, oldTsE, oldTsF);
    }
    return false;
}

// ═══════════════════════════════════════════════════════════════
//  N3: 3-way cyclic rotation
// ═══════════════════════════════════════════════════════════════

bool LocalSearch::moveN3(Individual& ind, int e, bool fixHard) {
    int oldPenE = eventHardPenaltyFast(ind, e);
    int tsE = ind.assignment[e].timeslot;
    auto oldE = ind.assignment[e];

    for (int f = 0; f < prob.numEvents; ++f) {
        if (f == e) continue;
        int tsF = ind.assignment[f].timeslot;
        auto oldF = ind.assignment[f];

        for (int g = f + 1; g < prob.numEvents; ++g) {
            if (g == e) continue;
            int tsG = ind.assignment[g].timeslot;
            if (tsE == tsF && tsF == tsG) continue;
            auto oldG = ind.assignment[g];

            int oldPenF  = eventHardPenaltyFast(ind, f);
            int oldPenG  = eventHardPenaltyFast(ind, g);
            int oldTotal = oldPenE + oldPenF + oldPenG;

            int rmE = matchRoomGreedy(ind, e, tsF);
            if (rmE < 0) continue;
            int rmF = matchRoomGreedy(ind, f, tsG);
            if (rmF < 0) continue;
            int rmG = matchRoomGreedy(ind, g, tsE);
            if (rmG < 0) continue;

            ind.assignment[e] = {tsF, rmE};
            ind.assignment[f] = {tsG, rmF};
            ind.assignment[g] = {tsE, rmG};
            moveEventInIndex(e, tsE, tsF);
            moveEventInIndex(f, tsF, tsG);
            moveEventInIndex(g, tsG, tsE);

            int newTotal = eventHardPenaltyFast(ind, e) +
                           eventHardPenaltyFast(ind, f) +
                           eventHardPenaltyFast(ind, g);

            if (newTotal < oldTotal) return true;

            ind.assignment[e] = oldE;
            ind.assignment[f] = oldF;
            ind.assignment[g] = oldG;
            moveEventInIndex(e, tsF, tsE);
            moveEventInIndex(f, tsG, tsF);
            moveEventInIndex(g, tsE, tsG);
        }
    }
    return false;
}

// ═══════════════════════════════════════════════════════════════
//  N4: Random reassign two E0 events
// ═══════════════════════════════════════════════════════════════

bool LocalSearch::moveN4(Individual& ind, int e, bool fixHard) {
    if (prob.E0_events.size() < 2 || prob.T0_slots.size() < 2)
        return false;

    int currentDf = ind.distanceToFeasibility;
    bool improved = false;

    std::uniform_int_distribution<int>
        de(0, (int)prob.E0_events.size()-1);
    std::uniform_int_distribution<int>
        dt(0, (int)prob.T0_slots.size()-1);

    for (int trial = 0; trial < 10; ++trial) {
        int e1 = prob.E0_events[de(rng_ls)];
        int e2 = prob.E0_events[de(rng_ls)];
        if (e1 == e2) continue;
        int ts1 = prob.T0_slots[dt(rng_ls)];
        int ts2 = prob.T0_slots[dt(rng_ls)];

        int rm1 = matchRoomGreedy(ind, e1, ts1);
        if (rm1 < 0) continue;
        int rm2 = matchRoomGreedy(ind, e2, ts2);
        if (rm2 < 0) continue;

        auto old1 = ind.assignment[e1];
        auto old2 = ind.assignment[e2];
        int oldTs1 = old1.timeslot;
        int oldTs2 = old2.timeslot;

        ind.assignment[e1] = {ts1, rm1};
        ind.assignment[e2] = {ts2, rm2};
        moveEventInIndex(e1, oldTs1, ts1);
        moveEventInIndex(e2, oldTs2, ts2);
        eval.evaluate(ind);

        if (ind.distanceToFeasibility < currentDf) {
            currentDf = ind.distanceToFeasibility;
            improved = true;
        } else {
            ind.assignment[e1] = old1;
            ind.assignment[e2] = old2;
            moveEventInIndex(e1, ts1, oldTs1);
            moveEventInIndex(e2, ts2, oldTs2);
            eval.evaluate(ind);
        }
    }
    return improved;
}

// ═══════════════════════════════════════════════════════════════
//  N5: Fix precedence using N1 and N2
// ═══════════════════════════════════════════════════════════════

bool LocalSearch::moveN5(Individual& ind, int e) {
    bool improved = false;
    for (int after : prob.events[e].mustBeBefore) {
        if (ind.assignment[e].timeslot >= ind.assignment[after].timeslot) {
            int oldPen = eventHardPenaltyFast(ind, e);
            bool fixed = false;

            const auto& avail = prob.events[e].hasTimeRestriction
                                ? prob.events[e].availableTimeSlots
                                : prob.T0_slots;
            int oldTs = ind.assignment[e].timeslot;
            int oldRm = ind.assignment[e].room;

            for (int ts : avail) {
                if (ts >= ind.assignment[after].timeslot) continue;
                int r = matchRoomGreedy(ind, e, ts);
                if (r < 0) continue;

                ind.assignment[e] = {ts, r};
                moveEventInIndex(e, oldTs, ts);
                int newPen = eventHardPenaltyFast(ind, e);

                if (newPen <= oldPen) {
                    improved = true;
                    fixed = true;
                    break;
                }
                ind.assignment[e] = {oldTs, oldRm};
                moveEventInIndex(e, ts, oldTs);
            }

            if (!fixed) {
                for (int f = 0; f < prob.numEvents; ++f) {
                    if (f == e || f == after) continue;
                    if (ind.assignment[f].timeslot >= ind.assignment[after].timeslot)
                        continue;

                    int fTs = ind.assignment[f].timeslot;
                    int fRm = ind.assignment[f].room;
                    int newRmE = matchRoomGreedy(ind, e, fTs);
                    if (newRmE < 0) continue;
                    int newRmF = matchRoomGreedy(ind, f, oldTs);
                    if (newRmF < 0) continue;

                    ind.assignment[e] = {fTs, newRmE};
                    ind.assignment[f] = {oldTs, newRmF};
                    moveEventInIndex(e, oldTs, fTs);
                    moveEventInIndex(f, fTs, oldTs);

                    int newPen = eventHardPenaltyFast(ind, e) +
                                 eventHardPenaltyFast(ind, f);
                    int old2 = oldPen + eventHardPenaltyFast(ind, f);

                    if (newPen <= old2) {
                        improved = true;
                        break;
                    }

                    ind.assignment[e] = {oldTs, oldRm};
                    ind.assignment[f] = {fTs, fRm};
                    moveEventInIndex(e, fTs, oldTs);
                    moveEventInIndex(f, oldTs, fTs);
                }
            }
        }
    }
    return improved;
}

// ═══════════════════════════════════════════════════════════════
//  N6: Move events from worst timeslot
// ═══════════════════════════════════════════════════════════════

bool LocalSearch::moveN6(Individual& ind, int percentTS) {
    std::vector<int> occupied;
    for (int ts = 0; ts < prob.numTimeslots; ++ts)
        if (!tsEvents[ts].empty())
            occupied.push_back(ts);

    if (occupied.empty()) return false;

    int subsetSize = std::max(1, (int)(occupied.size() * percentTS / 100));
    std::shuffle(occupied.begin(), occupied.end(), rng_ls);
    occupied.resize(subsetSize);

    int worstTs = occupied[0];
    int worstPen = 0;
    for (int ts : occupied) {
        int pen = 0;
        for (int e : tsEvents[ts])
            pen += eventHardPenaltyFast(ind, e);
        if (pen > worstPen) {
            worstPen = pen;
            worstTs = ts;
        }
    }

    auto eventsInWorst = tsEvents[worstTs];
    if (eventsInWorst.empty()) return false;

    Individual tmp = ind;
    auto tmpIndex = tsEvents;

    for (int e : eventsInWorst) {
        const auto& avail = prob.events[e].hasTimeRestriction
                            ? prob.events[e].availableTimeSlots
                            : prob.T0_slots;
        for (int ts : avail) {
            if (ts == worstTs) continue;
            int r = matchRoomGreedy(tmp, e, ts);
            if (r >= 0) {
                tmp.assignment[e] = {ts, r};
                break;
            }
        }
    }

    eval.evaluate(tmp);
    if (tmp.fitness < ind.fitness) {
        ind = tmp;
        buildIndex(ind);
        return true;
    }
    return false;
}

// ═══════════════════════════════════════════════════════════════
//  Helpers
// ═══════════════════════════════════════════════════════════════

bool LocalSearch::eventHasSoftViolation(const Individual& ind, int e) const {
    const int SLOTS_PER_DAY = 9;
    int ts = ind.assignment[e].timeslot;
    if (ts < 0) return false;
    int slot = ts % SLOTS_PER_DAY;

    if (slot == SLOTS_PER_DAY - 1) return true;

    int day = ts / SLOTS_PER_DAY;
    for (int s : prob.events[e].students) {
        std::vector<int> daySlots;
        for (int f = 0; f < prob.numEvents; ++f) {
            int fts = ind.assignment[f].timeslot;
            if (fts < 0 || fts / SLOTS_PER_DAY != day) continue;
            for (int st : prob.events[f].students) {
                if (st == s) { daySlots.push_back(fts % SLOTS_PER_DAY); break; }
            }
        }
        if ((int)daySlots.size() == 1) return true;
        std::sort(daySlots.begin(), daySlots.end());
        int consec = 1;
        for (int i = 1; i < (int)daySlots.size(); ++i) {
            if (daySlots[i] == daySlots[i-1]+1) { if (++consec > 2) return true; }
            else consec = 1;
        }
    }
    return false;
}

bool LocalSearch::singleMoveN1(Individual& ind, int e, bool fixHard) {
    buildIndex(ind);
    return moveN1(ind, e, fixHard);
}
bool LocalSearch::singleMoveN2(Individual& ind, int e, bool fixHard) {
    buildIndex(ind);
    return moveN2(ind, e, fixHard);
}
bool LocalSearch::singleMoveN3(Individual& ind, int e, bool fixHard) {
    buildIndex(ind);
    return moveN3(ind, e, fixHard);
}
bool LocalSearch::singleMoveN4(Individual& ind, int e, bool fixHard) {
    buildIndex(ind);
    return moveN4(ind, e, fixHard);
}
