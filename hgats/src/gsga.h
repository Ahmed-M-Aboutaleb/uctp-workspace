#pragma once
#include <vector>
#include <random>

#include "problem.h"
#include "individual.h"
#include "mem.h"
#include "local_search.h"

using namespace std;

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

        int    stagnationLimit = 4000;

        int    ls3InitSteps    = 4000;
        int    ls3ChildSteps   = 2000;

        int    ls4PercentTS    = 40;

        unsigned seed           = 42;
    };

    GSGA(const Problem& p, const Params& par);
    Individual run();

    const vector<Individual>& getPopulation() const {
        return population;
    }

private:
    const Problem& prob;
    Params         params;
    Evaluator      eval;
    LocalSearch    ls;
    MEM            mem;

    vector<Individual> population;
    int generation = 0;

    mutable mt19937 rng;

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
                         const vector<vector<int>>& tsIdx) const;
};