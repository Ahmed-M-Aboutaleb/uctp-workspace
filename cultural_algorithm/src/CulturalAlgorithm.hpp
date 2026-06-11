#ifndef CULTURAL_ALGORITHM_HPP
#define CULTURAL_ALGORITHM_HPP

#include "BeliefSpace.hpp"
#include "Individual.hpp"
#include "Problem.hpp"

#include <chrono>
#include <vector>

struct CAConfig {
    int   populationSize  = 10;
    int   maxGenerations  = 100000;       // upper bound; time is the real limit
    double timeLimitSec   = 360.0;        // matches the paper / ITC2002
    double pCulturalMut   = 1.0;          // applied every individual per pseudo-code
    double pSituational   = 0.5;          // probability of using situational vs normative influence
    double pRepair        = 1.0;          // applied every individual per pseudo-code
    int   tournamentSize  = 3;            // for selection
    bool  verbose         = true;
    int   logEveryGen     = 25;
    uint32_t seed         = 0;            // 0 -> random
};

class CulturalAlgorithm {
public:
    explicit CulturalAlgorithm(const Problem& problem, CAConfig cfg = {});

    // Run the algorithm. Returns the best-so-far individual.
    Individual run();

private:
    // ---- Operators (each modifies a single individual in place) ----
    // Cultural mutation: chooses an event E and a target (r,t) using the
    // belief space and then calls applyMutationMove (Algorithm 2 in the paper).
    void culturalMutation(Individual& ind);

    // Three exploration operators
    void interchangeOperator(Individual& ind);
    void sequencingOperator(Individual& ind);
    void simpleMutationOperator(Individual& ind);

    // Repair operator (domain knowledge): move events from the last timeslot
    // of each day to earlier timeslots if the hard constraints allow it.
    void repairOperator(Individual& ind);

    // Hard-violation focused repair: locate every event that contributes a
    // hard violation (unassigned, in an unsuitable room, or sharing a slot
    // with a conflicting event) and try to relocate it to a feasible cell.
    // If no free feasible cell exists, evict the current occupant and try
    // again. Always called when hardViol > 0.
    void fixHardViolations(Individual& ind);

    // Final intensification: rerun fixHardViolations + repair on the leader
    // until no further improvement is found.
    void finalIntensification(Individual& ind);

    // Algorithm 2 in the paper -- try to actually realise the cultural
    // mutation move.  Returns true on success.
    bool applyMutationMove(Individual& ind, int E, int targetRoom, int targetSlot);

    // Influence functions
    bool pickBySituational(int& E, int& room, int& slot) const;
    bool pickByNormative (int& E, int& room, int& slot) const;

    // ---- Operator parameter control (Section 4 of the paper) ----
    int   chooseExplorationOperator() const;       // 0=interchange, 1=sequencing, 2=simpleMutation
    void  updateOperatorRates(int op, long long fPrev, long long fCur);

    // ---- Selection (mu+lambda elitist replacement) ----
    void selection(std::vector<Individual>& parents, std::vector<Individual>& offspring);

    int  tournamentSelect(const std::vector<Individual>& pop) const;

    // ---- helpers
    bool tryMoveEventTo(Individual& ind, int e, int newRoom, int newSlot);
    bool trySwapEvents (Individual& ind, int e, int Em, int targetRoom, int targetSlot);

    const Problem& p;
    CAConfig cfg;

    std::vector<Individual> population;
    BeliefSpace belief;

    // Operator rates for the parameter control
    static constexpr int   N_OPS    = 3;
    static constexpr double MIN_PROB = 0.1;
    static constexpr double MAX_PROB = 0.8;
    double opProb[N_OPS];

    std::chrono::steady_clock::time_point startTime;
    int currentGeneration = 0;

    // For α factor: present_time / total_time
    double timeRatio() const;
};

#endif
