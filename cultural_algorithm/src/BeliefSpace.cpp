#include "BeliefSpace.hpp"
#include "Problem.hpp"

#include <algorithm>
#include <numeric>

void BeliefSpace::init(const Problem& p, const std::vector<Individual>& pop) {
    // Situational: copy the best individual of the initial population.
    int bestIdx = 0;
    for (size_t i = 1; i < pop.size(); ++i) {
        if (pop[i].fitness < pop[bestIdx].fitness) bestIdx = (int)i;
    }
    leader = pop[bestIdx];
    leaderInitialised = true;

    // Normative matrix: timeslot of each event in each individual of S_g
    // (above-average individuals).  We initialise it directly from the whole
    // initial population.
    normativeM.assign(p.numEvents, {});
    update(p, pop);
}

void BeliefSpace::update(const Problem& p, const std::vector<Individual>& pop) {
    if (pop.empty()) return;

    // Update situational knowledge with the best of the current population if
    // it improves the leader.
    int bestIdx = 0;
    for (size_t i = 1; i < pop.size(); ++i) {
        if (pop[i].fitness < pop[bestIdx].fitness) bestIdx = (int)i;
    }
    if (!leaderInitialised || pop[bestIdx].fitness < leader.fitness) {
        leader = pop[bestIdx];
        leaderInitialised = true;
    }

    // Normative knowledge: select the above-average (better-than-mean
    // fitness, which means lower fitness here) individuals and rebuild M.
    long long sum = 0;
    for (const auto& ind : pop) sum += ind.fitness;
    double avg = (double)sum / (double)pop.size();

    normativeM.assign(p.numEvents, {});
    for (const auto& ind : pop) {
        if (ind.fitness <= avg) {
            for (int e = 0; e < p.numEvents; ++e) {
                int t = ind.eventSlot[e];
                if (t >= 0) normativeM[e].push_back(t);
            }
        }
    }
}
