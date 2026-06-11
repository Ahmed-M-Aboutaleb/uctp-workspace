#include "gsga.h"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>

GSGA::GSGA(const Problem& p, const Params& par)
    : prob(p), params(par), eval(p), ls(p, eval), mem(p) {}

Individual GSGA::run() {
    auto startTime = std::chrono::steady_clock::now();

    population.clear();
    population.reserve(params.populationSize);

    for (int i = 0; i < params.populationSize; ++i) {
        Individual ind = initializeIndividual();
        ls.applyLS3(ind, params.ls3InitSteps);
        ls.applyLS4(ind);
        eval.evaluate(ind);
        population.push_back(ind);
        std::cout << "[Init] Individual " << i
                  << "  Df=" << ind.distanceToFeasibility
                  << "  SCP=" << ind.softConstraintPenalty << "\n";
    }

    constructMEM();

    generation        = 0;
    int noImprov      = 0;
    int lastBestFit   = std::min_element(
        population.begin(), population.end())->fitness;

    while (true) {
        auto now = std::chrono::steady_clock::now();
        double elapsed =
            std::chrono::duration<double>(now - startTime).count();
        if (elapsed >= params.timeLimitSec) break;
        if (generation >= params.maxGenerations)  break;

        if (generation > 0 && generation % params.tau == 0)
            constructMEM();

        std::uniform_real_distribution<double> uni(0.0, 1.0);

        Individual child = (uni(rng) < params.gamma)
                           ? guidedSearch()
                           : biasedCrossover();

        // diversity-aware mutation
        {
            int bestDf  = std::min_element(
                population.begin(), population.end())->distanceToFeasibility;
            int worstDf = std::max_element(
                population.begin(), population.end())->distanceToFeasibility;
            bool lowDiv = (worstDf - bestDf) < 5;
            double pm   = lowDiv
                          ? std::min(0.9, params.Pm * 2.0)
                          : params.Pm;
            if (uni(rng) < pm) mutate(child);
        }

        ls.applyLS3(child, params.ls3ChildSteps);
        ls.applyLS4(child);
        eval.evaluate(child);

        population[worstIndex()] = child;

        // stagnation → partial restart
        int curBest = std::min_element(
            population.begin(), population.end())->fitness;
        if (curBest < lastBestFit) {
            lastBestFit = curBest;
            noImprov    = 0;
        } else {
            ++noImprov;
        }

        if (noImprov >= params.stagnationLimit) {
            partialRestart();
            noImprov    = 0;
            lastBestFit = std::min_element(
                population.begin(), population.end())->fitness;
            constructMEM();
        }

        if (isOptimal(child)) {
            std::cout << "[GSGA] Optimal found at gen="
                      << generation << "\n";
            break;
        }

        if (generation % 100 == 0) {
            const Individual& best = *std::min_element(
                population.begin(), population.end());
            auto now2 = std::chrono::steady_clock::now();
            double el =
                std::chrono::duration<double>(now2 - startTime).count();
            std::cout << "[GSGA] Gen=" << generation
                      << "  Df="  << best.distanceToFeasibility
                      << "  SCP=" << best.softConstraintPenalty
                      << "  Time=" << (int)el << "s\n";
        }

        ++generation;
    }

    return *std::min_element(population.begin(), population.end());
}

Individual GSGA::initializeIndividual() {
    Individual ind;
    ind.init(prob.numEvents);

    for (int e = 0; e < prob.numEvents; ++e) {
        const auto& avail = prob.events[e].hasTimeRestriction
                            ? prob.events[e].availableTimeSlots
                            : prob.T0_slots;
        if (!avail.empty()) {
            std::uniform_int_distribution<int> dt(0, (int)avail.size() - 1);
            ind.assignment[e] = {avail[dt(rng)], -1};
        } else {
            ind.assignment[e] = {0, -1};
        }
    }

    ls.assignRoomsViaMatching(ind);
    eval.evaluate(ind);
    return ind;
}

void GSGA::constructMEM() {
    mem.construct(population, params.alpha, eval);
}

Individual GSGA::guidedSearch() {
    Individual child;
    child.init(prob.numEvents);

    // pick random individual from top 30%
    std::vector<int> idx(population.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(), [&](int a, int b) {
        return population[a].fitness < population[b].fitness;
    });
    int topK = std::max(1, (int)(population.size() * 0.3));
    std::uniform_int_distribution<int> dtop(0, topK - 1);
    const Individual& base = population[idx[dtop(rng)]];

    int numToGuide = std::max(1,
        (int)(params.beta * (int)prob.E0_events.size()));

    std::vector<int> E0_copy = prob.E0_events;
    std::shuffle(E0_copy.begin(), E0_copy.end(), rng);
    if ((int)E0_copy.size() > numToGuide)
        E0_copy.resize(numToGuide);

    std::vector<bool> guided(prob.numEvents, false);
    for (int e : E0_copy) guided[e] = true;

    for (int e : E0_copy) {
        if (mem.hasEntry(e)) {
            auto p = mem.randomPairFor(e);
            child.assignment[e] = {p.timeslot, -1};
        } else {
            std::uniform_int_distribution<int>
                dt(0, (int)prob.T0_slots.size() - 1);
            child.assignment[e] = {prob.T0_slots[dt(rng)], -1};
        }
    }

    for (int e = 0; e < prob.numEvents; ++e) {
        if (guided[e]) continue;
        child.assignment[e] = {base.assignment[e].timeslot, -1};
    }

    ls.assignRoomsViaMatching(child);
    eval.evaluate(child);
    return child;
}

Individual GSGA::biasedCrossover() {
    int p1 = tournamentSelect();
    int p2 = tournamentSelect();
    while (p2 == p1) p2 = tournamentSelect();

    // build tsIndex once per parent
    std::vector<std::vector<int>> tsIdx1(prob.numTimeslots);
    std::vector<std::vector<int>> tsIdx2(prob.numTimeslots);
    for (int e = 0; e < prob.numEvents; ++e) {
        int ts1 = population[p1].assignment[e].timeslot;
        int ts2 = population[p2].assignment[e].timeslot;
        if (ts1 >= 0 && ts1 < prob.numTimeslots) tsIdx1[ts1].push_back(e);
        if (ts2 >= 0 && ts2 < prob.numTimeslots) tsIdx2[ts2].push_back(e);
    }

    Individual child;
    child.init(prob.numEvents);

    for (int e = 0; e < prob.numEvents; ++e) {
        int pen1 = eventPenaltyFast(population[p1], e, tsIdx1);
        int pen2 = eventPenaltyFast(population[p2], e, tsIdx2);
        child.assignment[e] = {
            (pen1 <= pen2)
                ? population[p1].assignment[e].timeslot
                : population[p2].assignment[e].timeslot,
            -1
        };
    }

    ls.assignRoomsViaMatching(child);
    eval.evaluate(child);
    return child;
}

void GSGA::mutate(Individual& ind) {
    std::uniform_int_distribution<int> dn(1, 4);
    std::uniform_int_distribution<int> de(0, prob.numEvents - 1);
    int nb       = dn(rng);
    int e        = de(rng);
    bool fixHard = (ind.distanceToFeasibility > 0);
    switch (nb) {
        case 1: ls.singleMoveN1(ind, e, fixHard); break;
        case 2: ls.singleMoveN2(ind, e, fixHard); break;
        case 3: ls.singleMoveN3(ind, e, fixHard); break;
        case 4: ls.singleMoveN4(ind, e, fixHard); break;
    }
    eval.evaluate(ind);
}

void GSGA::partialRestart() {
    int keepCount = std::max(1, (int)(population.size() * 0.3));

    std::vector<int> idx(population.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(), [&](int a, int b) {
        return population[a].fitness < population[b].fitness;
    });

    for (int i = keepCount; i < (int)population.size(); ++i) {
        Individual ind = initializeIndividual();
        ls.applyLS3(ind, params.ls3InitSteps);
        ls.applyLS4(ind);
        eval.evaluate(ind);
        population[idx[i]] = ind;
    }
}

int GSGA::tournamentSelect() const {
    std::uniform_int_distribution<int> di(0, (int)population.size() - 1);
    int a = di(rng);
    int b = di(rng);
    return (population[a].fitness <= population[b].fitness) ? a : b;
}

int GSGA::worstIndex() const {
    std::vector<int> idx(population.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(), [&](int a, int b) {
        return population[a].fitness > population[b].fitness;
    });
    int bottomK = std::max(1, (int)(population.size() * 0.5));
    std::uniform_int_distribution<int> d(0, bottomK - 1);
    return idx[d(rng)];
}

bool GSGA::isOptimal(const Individual& ind) const {
    return ind.distanceToFeasibility == 0 &&
           ind.softConstraintPenalty  == 0;
}

int GSGA::eventPenaltyFast(const Individual& ind, int e,
                            const std::vector<std::vector<int>>& tsIdx) const {
    const auto& a = ind.assignment[e];
    if (a.timeslot < 0 || a.room < 0) return 1000;

    int pen = 0;

    const auto& sr = prob.events[e].suitableRooms;
    if (std::find(sr.begin(), sr.end(), a.room) == sr.end()) ++pen;

    if (prob.events[e].hasTimeRestriction) {
        const auto& avail = prob.events[e].availableTimeSlots;
        if (std::find(avail.begin(), avail.end(), a.timeslot) == avail.end())
            ++pen;
    }

    for (int f : tsIdx[a.timeslot]) {
        if (f == e) continue;
        if (ind.assignment[f].room == a.room) ++pen;
        if (prob.conflict[e][f])              ++pen;
    }

    for (int after : prob.events[e].mustBeBefore)
        if (a.timeslot >= ind.assignment[after].timeslot) ++pen;

    return pen;
}