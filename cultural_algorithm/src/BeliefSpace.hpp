#ifndef BELIEF_SPACE_HPP
#define BELIEF_SPACE_HPP

#include "Individual.hpp"
#include "Problem.hpp"
#include <vector>

// Belief space of the cultural algorithm.
//
//  - Situational knowledge: the best individual ever seen ("the leader").
//  - Normative knowledge:   matrix M where M[event][k] = timeslot assigned
//                           to that event in the k-th above-average individual
//                           of the current generation.  Used by the
//                           normative-influence cultural mutation.
//  - Domain knowledge:      a-priori knowledge that the last timeslot of
//                           each day should be avoided (used by the repair
//                           operator) and a ranking of events by number of
//                           conflicting events (precomputed in Problem).
class BeliefSpace {
public:
    Individual leader;                          // situational
    std::vector<std::vector<int>> normativeM;   // normativeM[event] = list of timeslots in S_g
    bool leaderInitialised = false;

    void init(const Problem& p, const std::vector<Individual>& pop);

    // Update with the accepted (above-average) individuals of the current
    // generation.
    void update(const Problem& p, const std::vector<Individual>& pop);
};

#endif
