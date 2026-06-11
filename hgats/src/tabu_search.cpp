#include "tabu_search.h"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <numeric>

TabuSearch::TabuSearch(const Problem& p, const Params& par)
    : prob(p), params(par), eval(p), ls(p, eval) {
    tabuListLength = std::max(1,
        prob.numEvents / params.tabuListK);
    params.maxNeighbors = std::max(1,
        (int)(prob.numEvents * 0.30));
}

Individual TabuSearch::run(const Individual& startSolution) {
    auto startTime = std::chrono::steady_clock::now();

    Individual current = startSolution;
    Individual best    = startSolution;
    eval.evaluate(current);
    eval.evaluate(best);

    clearTabu();

    int iter         = 0;
    int noImprov     = 0;
    int perturbCount = 0;  // خارج الـ stagnation block

    while (true) {
        auto now = std::chrono::steady_clock::now();
        double elapsed =
            std::chrono::duration<double>(now - startTime).count();
        if (elapsed >= params.timeLimitSec) break;

        if (best.distanceToFeasibility == 0 &&
            best.softConstraintPenalty  == 0) {
            std::cout << "[TS] Optimal at iter=" << iter << "\n";
            break;
        }

        if (noImprov >= params.stagnationLimit) {
            ++perturbCount;  // يزيد هنا مش بيتـ reset

            tabuListLength = std::min(
                prob.numEvents / params.tabuListK + perturbCount * 5,
                prob.numEvents / 3);

            std::cout << "[TS] Stagnation at iter=" << iter
                      << "  Df=" << best.distanceToFeasibility
                      << "  -> Perturbation #" << perturbCount
                      << "  TabuLen=" << tabuListLength << "\n";

            current = perturbAndRestart(best, ls);
            eval.evaluate(current);
            clearTabu();
            noImprov = 0;
            ++iter;
            continue;
        }

        auto neighbors = generateNeighbors(current, params.maxNeighbors);

        if (neighbors.empty()) { ++iter; ++noImprov; continue; }

        const Move* bestImproving = nullptr;
        const Move* bestNonTabu   = nullptr;

        for (const auto& m : neighbors) {
            if (m.resultFitness < best.fitness) {
                if (!bestImproving ||
                    m.resultFitness < bestImproving->resultFitness)
                    bestImproving = &m;
            }
            if (!isTabu(m)) {
                if (!bestNonTabu ||
                    m.resultFitness < bestNonTabu->resultFitness)
                    bestNonTabu = &m;
            }
        }

        const Move* chosen = nullptr;
        if      (bestImproving) chosen = bestImproving;
        else if (bestNonTabu)   chosen = bestNonTabu;
        else                    chosen = &neighbors[0];

        current = applyMove(current, *chosen);
        addToTabu(*chosen);
        eval.evaluate(current);

        if (current.fitness < best.fitness) {
            best     = current;
            noImprov = 0;

            auto now2 = std::chrono::steady_clock::now();
            double el = std::chrono::duration<double>(now2 - startTime).count();
            std::cout << "[TS] Iter=" << iter
                      << "  Df="   << best.distanceToFeasibility
                      << "  SCP="  << best.softConstraintPenalty
                      << "  Time=" << std::fixed << std::setprecision(1)
                      << el << "s\n";
        } else {
            ++noImprov;
        }

        ++iter;
    }

    return best;
}

Individual TabuSearch::perturbAndRestart(const Individual& best,
                                          LocalSearch& ls_ref) {
    Individual perturbed = best;
    std::uniform_int_distribution<int> de(0, prob.numEvents - 1);
    int numMoves = std::max(5, prob.numEvents / 20);
    for (int i = 0; i < numMoves; ++i) {
        int e = de(rng);
        const auto& avail = prob.events[e].hasTimeRestriction
                            ? prob.events[e].availableTimeSlots
                            : prob.T0_slots;
        if (!avail.empty()) {
            std::uniform_int_distribution<int>
                da(0, (int)avail.size() - 1);
            perturbed.assignment[e].timeslot = avail[da(rng)];
            perturbed.assignment[e].room = -1;
        }
    }
    // Assign rooms via matching BEFORE local search runs
    ls_ref.assignRoomsViaMatching(perturbed);
    ls_ref.applyLS3(perturbed, 300);
    ls_ref.applyLS4(perturbed);
    eval.evaluate(perturbed);
    return perturbed;
}

std::vector<TabuSearch::Move>
TabuSearch::generateNeighbors(const Individual& ind,
                               int numToCheck) const {
    std::vector<Move> moves;
    moves.reserve(numToCheck);

    std::vector<std::vector<int>> tsIdx(prob.numTimeslots);
    for (int e = 0; e < prob.numEvents; ++e) {
        int ts = ind.assignment[e].timeslot;
        if (ts >= 0 && ts < prob.numTimeslots)
            tsIdx[ts].push_back(e);
    }

    auto findRoom = [&](int event, int ts, int exclude1 = -1, int exclude2 = -1) -> int {
        if (ts < 0 || ts >= prob.numTimeslots) return -1;
        bool taken[1000] = {};
        for (int f : tsIdx[ts]) {
            if (f == event || f == exclude1 || f == exclude2) continue;
            if (ind.assignment[f].room >= 0)
                taken[ind.assignment[f].room] = true;
        }
        for (int r : prob.events[event].suitableRooms)
            if (!taken[r] && prob.rooms[r].capacity >= prob.events[event].numStudents)
                return r;
        return -1;
    };

    std::uniform_real_distribution<double> uni(0.0, 1.0);
    std::uniform_int_distribution<int>
        de(0, prob.numEvents - 1);
    std::uniform_int_distribution<int>
        dt(0, prob.numTimeslots - 1);

    std::vector<int> violatedEvents;
    for (int e = 0; e < prob.numEvents; ++e) {
        const auto& a = ind.assignment[e];
        if (a.timeslot < 0) { violatedEvents.push_back(e); continue; }
        for (int f : tsIdx[a.timeslot]) {
            if (f == e) continue;
            if (ind.assignment[f].room == a.room || prob.conflict[e][f]) {
                violatedEvents.push_back(e);
                break;
            }
        }
    }
    std::shuffle(violatedEvents.begin(), violatedEvents.end(), rng);

    int generated = 0;
    int maxTry    = numToCheck * 12;

    for (int attempt = 0;
         attempt < maxTry && generated < numToCheck;
         ++attempt) {

        Move m;
        m.event2 = -1;
        double r = uni(rng);

        if (r < params.probN1) {
            m.type   = 1;
            if (!violatedEvents.empty() && uni(rng) < 0.7) {
                std::uniform_int_distribution<int>
                    dv(0, (int)violatedEvents.size()-1);
                m.event1 = violatedEvents[dv(rng)];
            } else {
                m.event1 = de(rng);
            }

            const auto& avail = prob.events[m.event1].hasTimeRestriction
                                ? prob.events[m.event1].availableTimeSlots
                                : prob.T0_slots;
            if (avail.empty()) continue;
            std::uniform_int_distribution<int>
                da(0, (int)avail.size() - 1);
            m.newTs1 = avail[da(rng)];
            if (m.newTs1 == ind.assignment[m.event1].timeslot) continue;

            m.newRoom1 = findRoom(m.event1, m.newTs1);
            if (m.newRoom1 < 0) continue;

            Individual tmp = ind;
            tmp.assignment[m.event1] = {m.newTs1, m.newRoom1};
            m.resultFitness = eval.evaluate(tmp);

        } else if (r < params.probN1 + params.probN2) {
            m.type   = 2;
            if (!violatedEvents.empty() && uni(rng) < 0.7) {
                std::uniform_int_distribution<int>
                    dv(0, (int)violatedEvents.size()-1);
                m.event1 = violatedEvents[dv(rng)];
            } else {
                m.event1 = de(rng);
            }
            m.event2 = de(rng);
            if (m.event1 == m.event2) continue;

            m.newTs1 = ind.assignment[m.event2].timeslot;
            m.newTs2 = ind.assignment[m.event1].timeslot;
            if (m.newTs1 == m.newTs2) continue;

            m.newRoom1 = findRoom(m.event1, m.newTs1, m.event2);
            if (m.newRoom1 < 0) continue;
            m.newRoom2 = findRoom(m.event2, m.newTs2, m.event1);
            if (m.newRoom2 < 0) continue;

            Individual tmp = ind;
            tmp.assignment[m.event1] = {m.newTs1, m.newRoom1};
            tmp.assignment[m.event2] = {m.newTs2, m.newRoom2};
            m.resultFitness = eval.evaluate(tmp);

        } else {
            if (prob.E0_events.size() < 2 ||
                prob.T0_slots.size()  < 2) continue;

            m.type = 4;
            std::uniform_int_distribution<int>
                de0(0, (int)prob.E0_events.size() - 1);
            std::uniform_int_distribution<int>
                dt0(0, (int)prob.T0_slots.size()  - 1);

            m.event1 = prob.E0_events[de0(rng)];
            m.event2 = prob.E0_events[de0(rng)];
            if (m.event1 == m.event2) continue;

            m.newTs1 = prob.T0_slots[dt0(rng)];
            m.newTs2 = prob.T0_slots[dt0(rng)];

            m.newRoom1 = findRoom(m.event1, m.newTs1);
            if (m.newRoom1 < 0) continue;
            m.newRoom2 = findRoom(m.event2, m.newTs2);
            if (m.newRoom2 < 0) continue;

            Individual tmp = ind;
            tmp.assignment[m.event1] = {m.newTs1, m.newRoom1};
            tmp.assignment[m.event2] = {m.newTs2, m.newRoom2};
            m.resultFitness = eval.evaluate(tmp);
        }

        moves.push_back(m);
        ++generated;
    }

    return moves;
}

Individual TabuSearch::applyMove(const Individual& ind,
                                  const Move& m) const {
    Individual result = ind;
    result.assignment[m.event1] = {m.newTs1, m.newRoom1};
    if (m.event2 >= 0)
        result.assignment[m.event2] = {m.newTs2, m.newRoom2};
    return result;
}

bool TabuSearch::isTabu(const Move& m) const {
    TabuEntry e1{m.event1, m.newTs1};
    if (tabuSet.count(e1)) return true;
    if (m.event2 >= 0) {
        TabuEntry e2{m.event2, m.newTs2};
        if (tabuSet.count(e2)) return true;
    }
    return false;
}

void TabuSearch::addToTabu(const Move& m) {
    auto addEntry = [&](int e, int ts) {
        if (e < 0) return;
        TabuEntry entry{e, ts};
        tabuList.push_back(entry);
        tabuSet.insert(entry);
        while ((int)tabuList.size() > tabuListLength) {
            tabuSet.erase(tabuList.front());
            tabuList.pop_front();
        }
    };
    addEntry(m.event1, m.newTs1);
    if (m.type != 1) addEntry(m.event2, m.newTs2);
}

void TabuSearch::clearTabu() {
    tabuList.clear();
    tabuSet.clear();
}
