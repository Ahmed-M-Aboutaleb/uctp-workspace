#pragma once
#include <vector>
#include <array>
#include <chrono>
#include <random>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <climits>
#include "individual.h"
#include "problem.h"
#include "nsga2.h"
#include "gs_strategy.h"
#include "local_search.h"
#include "crossover.h"
#include "mutation.h"

struct GSNSGAParams
{
    int popSize = 48;   // population size
    double gamma = 0.6; // probability of GS strategy vs crossover
    double pm = 0.6;    // mutation probability
    int tau = 20;       // MEM reconstruction frequency (generations)
    double alpha = 0.2; // fraction of best individuals for MEM
    double beta = 0.4;  // fraction of events used in GS
    int smax = 100;     // max steps in local search
    int timeLimitSecs = 60;
    int seed = 45;
    bool greedyInit = true; // use greedy initialization
    bool verbose = true;    // print generation-by-generation progress
};

class GSNSGA2
{
public:
    static std::vector<Individual> run(const Problem &pb, const GSNSGAParams &params)
    {
        auto startTime = std::chrono::steady_clock::now();
        std::mt19937 rng(params.seed);

        auto elapsed = [&]()
        {
            return std::chrono::duration<double>(
                       std::chrono::steady_clock::now() - startTime)
                .count();
        };

        if (params.verbose)
            std::cout << "Initializing population of size " << params.popSize << "...\n";

        // Initialize population: mix of greedy and random
        std::vector<Individual> pop;
        pop.reserve(params.popSize);

        int greedyCount = params.greedyInit ? std::max(1, params.popSize * 2 / 3) : 0;

        for (int i = 0; i < params.popSize; i++)
        {
            Individual ind;
            if (i < greedyCount)
            {
                ind = Individual::greedy(pb, rng);
            }
            else
            {
                ind = Individual::random(pb, rng);
            }
            LocalSearch::applyLS(ind, pb, params.smax, rng);
            pop.push_back(ind);

            if (params.verbose && ((i + 1) % 5 == 0 || i == 0))
            {
                std::cout << "  Initialized " << (i + 1) << "/" << params.popSize
                          << " | HCV=" << pop.back().hcv
                          << " | SCV=" << (pop.back().f1 + pop.back().f2 + pop.back().f3)
                          << " | time=" << std::fixed << std::setprecision(1) << elapsed() << "s\n";
            }
        }

        NSGA2::assignRankAndCrowding(pop);
        if (params.verbose)
            std::cout << "Population initialized. Starting evolution...\n\n";

        std::array<MEM, GSStrategy::NUM_MEMS> mems;
        bool memsReady = false;
        int gen = 0;
        int bestFeasible = 0;
        int bestTotalSCV = INT_MAX;

        while (elapsed() < params.timeLimitSecs)
        {
            // Reconstruct MEMs every tau generations
            if (gen % params.tau == 0)
            {
                mems = GSStrategy::constructMEM(pop, pb, params.alpha, rng);
                memsReady = true;
            }

            // Generate child population
            std::vector<Individual> children;
            children.reserve(params.popSize);
            std::uniform_real_distribution<double> probDist(0.0, 1.0);

            for (int i = 0; i < params.popSize; i++)
            {
                Individual child;
                double rho = probDist(rng);

                // Check any MEM is non-empty (has useful entries)
                bool memUsable = memsReady;
                if (memUsable)
                {
                    bool anyEntry = false;
                    for (int mi = 0; mi < GSStrategy::NUM_MEMS && !anyEntry; mi++)
                        for (int e = 0; e < pb.n && !anyEntry; e++)
                            if (!mems[mi][e].empty())
                                anyEntry = true;
                    if (!anyEntry)
                        memUsable = false;
                }

                if (memUsable && rho < params.gamma)
                {
                    child = GSStrategy::guidedSearch(mems, pb, params.beta, rng);
                }
                else
                {
                    int p1 = NSGA2::tournamentSelect(pop, rng);
                    int p2 = NSGA2::tournamentSelect(pop, rng);
                    while (p2 == p1)
                        p2 = NSGA2::tournamentSelect(pop, rng);
                    child = Crossover::uniformCrossover(pop[p1], pop[p2], pb, rng);
                }

                Mutation::mutate(child, pb, params.pm, rng);
                LocalSearch::applyLS(child, pb, params.smax, rng);
                children.push_back(child);
            }

            // Merge and select next generation
            std::vector<Individual> combined;
            combined.reserve(2 * params.popSize);
            for (auto &ind : pop)
                combined.push_back(std::move(ind));
            for (auto &ind : children)
                combined.push_back(std::move(ind));
            pop = NSGA2::selectNextGen(combined, params.popSize);

            gen++;

            // Compute stats
            int feasible = 0, rank1 = 0, minSCV = INT_MAX;
            for (auto &ind : pop)
            {
                if (ind.rank == 1)
                {
                    rank1++;
                    if (ind.feasible())
                    {
                        feasible++;
                        int scv = ind.f1 + ind.f2 + ind.f3;
                        minSCV = std::min(minSCV, scv);
                    }
                }
            }
            // Best HCV across all
            int minHCV = INT_MAX;
            for (auto &ind : pop)
                minHCV = std::min(minHCV, ind.hcv);

            bool newBest = (feasible > bestFeasible) ||
                           (feasible == bestFeasible && feasible > 0 && minSCV < bestTotalSCV);
            if (newBest)
            {
                bestFeasible = feasible;
                bestTotalSCV = minSCV;
            }

            if (params.verbose && (gen % 10 == 0 || newBest))
            {
                std::cout << "Gen " << std::setw(4) << gen
                          << " | time=" << std::fixed << std::setprecision(1) << elapsed() << "s"
                          << " | rank1=" << rank1
                          << " | feasible=" << feasible
                          << " | minHCV=" << minHCV;
                if (feasible > 0)
                    std::cout << " | bestSCV=" << minSCV;
                std::cout << "\n";
            }
        }

        // Return non-dominated front
        std::vector<Individual> result;
        for (auto &ind : pop)
            if (ind.rank == 1)
                result.push_back(ind);
        return result;
    }
};
