#pragma once
#include <vector>
#include <random>

#include "problem.h"
#include "individual.h"
#include "mem.h"
#include "local_search.h"

class GSGA {
public:
    struct Params {
        int    populationSize  = 50;
        double alpha           = 0.2;
        double beta            = 0.3;
        double gamma           = 0.8;
        int    tau             = 20;
        double Pm              = 0.5;
        int    maxGenerations  = 100000;
        int    timeLimitSec    = 600;
        int    stagnationLimit = 2000;
        int    ls3InitSteps    = 5000;
        int    ls3ChildSteps   = 2000;
        int    ls4PercentTS    = 40;
    };

    GSGA(const Problem& p, const Params& par);
    Individual run();

    const std::vector<Individual>& getPopulation() const {
        return population;
    }

private:
    const Problem& prob;
    Params         params;
    Evaluator      eval;
    LocalSearch    ls;
    MEM            mem;

    std::vector<Individual> population;
    int generation = 0;

    mutable std::mt19937 rng{std::random_device{}()};

    Individual initializeIndividual();
    void       constructMEM();
    Individual guidedSearch();
    Individual biasedCrossover();
    void       mutate(Individual& ind);
    void       partialRestart();
    int        tournamentSelect() const;
    int        worstIndex()       const;
    bool       isOptimal(const Individual& ind) const;

    int eventPenaltyFast(const Individual& ind, int e,
                         const std::vector<std::vector<int>>& tsIdx) const;
};