#pragma once
#include <vector>
#include <algorithm>
#include <numeric>
#include <limits>
#include <random>
#include "individual.h"
#include "problem.h"

class NSGA2
{
public:
    // Constrained dominance: a dominates b if:
    // 1. a feasible, b infeasible
    // 2. Both feasible, a Pareto-dominates b
    // 3. Both infeasible, a has fewer HCV and (same or better objectives)
    static bool dominates(const Individual &a, const Individual &b)
    {
        if (a.hcv == 0 && b.hcv > 0)
            return true;
        if (a.hcv > 0 && b.hcv == 0)
            return false;

        if (a.hcv > 0 && b.hcv > 0)
        {
            // Both infeasible: compare by HCV first, then soft
            if (a.hcv < b.hcv)
                return true;
            if (a.hcv > b.hcv)
                return false;
        }

        // Both feasible or same HCV: standard Pareto dominance on objectives
        bool strictBetter = false; // Ensures strict improvement exists
        if (a.f1 > b.f1)
            return false;
        if (a.f2 > b.f2)
            return false;
        if (a.f3 > b.f3)
            return false;
        if (a.f1 < b.f1)
            strictBetter = true;
        if (a.f2 < b.f2)
            strictBetter = true;
        if (a.f3 < b.f3)
            strictBetter = true;
        return strictBetter;
    }

    // Non-dominated sorting: assigns rank to each individual
    static void nonDominatedSort(std::vector<Individual> &pop)
    {
        int N = (int)pop.size();
        for (auto &ind : pop)
        {
            ind.domCount = 0;      // how many dominate this solution
            ind.dominated.clear(); // list of solutions it dominates
        }

        std::vector<std::vector<int>> fronts;
        fronts.push_back({});

        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                if (i == j)
                    continue;
                if (dominates(pop[i], pop[j]))
                {
                    pop[i].dominated.push_back(j);
                }
                else if (dominates(pop[j], pop[i]))
                {
                    pop[i].domCount++;
                }
            }
            if (pop[i].domCount == 0)
            {
                pop[i].rank = 1;
                fronts[0].push_back(i);
            }
        }

        int fi = 0;
        while (!fronts[fi].empty())
        {
            std::vector<int> nextFront;
            for (int i : fronts[fi])
            {
                for (int j : pop[i].dominated)
                {
                    pop[j].domCount--;
                    if (pop[j].domCount == 0)
                    {
                        pop[j].rank = fi + 2;
                        nextFront.push_back(j);
                    }
                }
            }
            fronts.push_back(nextFront);
            fi++;
        }
    }

    // Compute crowding distance for a front
    static void computeCrowdingDistance(std::vector<Individual> &pop,
                                        const std::vector<int> &frontIdx)
    {
        int sz = (int)frontIdx.size();
        for (int i : frontIdx)
            pop[i].crowdingDist = 0.0;
        if (sz <= 2)
        {
            for (int i : frontIdx)
                pop[i].crowdingDist = std::numeric_limits<double>::infinity();
            return;
        }

        // Three objectives: f1, f2, f3
        auto computeForObj = [&](auto getObj)
        {
            std::vector<int> sorted = frontIdx;
            std::sort(sorted.begin(), sorted.end(), [&](int a, int b)
                      { return getObj(pop[a]) < getObj(pop[b]); });
            pop[sorted.front()].crowdingDist = std::numeric_limits<double>::infinity();
            pop[sorted.back()].crowdingDist = std::numeric_limits<double>::infinity();
            double range = getObj(pop[sorted.back()]) - getObj(pop[sorted.front()]);
            if (range == 0)
                range = 1;
            for (int i = 1; i < sz - 1; i++)
            {
                pop[sorted[i]].crowdingDist +=
                    (getObj(pop[sorted[i + 1]]) - getObj(pop[sorted[i - 1]])) / range;
            }
        };

        computeForObj([](const Individual &x)
                      { return x.f1; });
        computeForObj([](const Individual &x)
                      { return x.f2; });
        computeForObj([](const Individual &x)
                      { return x.f3; });
    }

    // Crowded comparison operator
    static bool crowdedBetter(const Individual &a, const Individual &b)
    {
        if (a.rank != b.rank)
            return a.rank < b.rank;
        return a.crowdingDist > b.crowdingDist;
    }

    // Crowded tournament selection (binary tournament)
    static int tournamentSelect(const std::vector<Individual> &pop, std::mt19937 &rng)
    {
        std::uniform_int_distribution<int> dist(0, (int)pop.size() - 1);
        int a = dist(rng);
        int b = dist(rng);
        while (b == a)
            b = dist(rng);
        return crowdedBetter(pop[a], pop[b]) ? a : b;
    }

    // Merge parent and child populations, apply NSGA-II selection to get N best
    static std::vector<Individual> selectNextGen(
        std::vector<Individual> &combined, int N)
    {
        nonDominatedSort(combined);

        // Group by ranks
        int maxRank = 0;
        for (auto &ind : combined)
            maxRank = std::max(maxRank, ind.rank);

        std::vector<Individual> next;
        next.reserve(N);

        for (int r = 1; r <= maxRank && (int)next.size() < N; r++)
        {
            std::vector<int> frontIdx;
            for (int i = 0; i < (int)combined.size(); i++)
            {
                if (combined[i].rank == r)
                    frontIdx.push_back(i);
            }

            computeCrowdingDistance(combined, frontIdx);

            if ((int)(next.size() + frontIdx.size()) <= N)
            {
                for (int i : frontIdx)
                    next.push_back(combined[i]);
            }
            else
            {
                // Sort by crowding distance descending and take what's needed
                std::sort(frontIdx.begin(), frontIdx.end(), [&](int a, int b)
                          { return combined[a].crowdingDist > combined[b].crowdingDist; });
                int needed = N - (int)next.size();
                for (int i = 0; i < needed; i++)
                {
                    next.push_back(combined[frontIdx[i]]);
                }
            }
        }

        return next;
    }

    // Assign rank and crowding distance to a population
    static void assignRankAndCrowding(std::vector<Individual> &pop)
    {
        nonDominatedSort(pop);

        int maxRank = 0;
        for (auto &ind : pop)
            maxRank = std::max(maxRank, ind.rank);

        for (int r = 1; r <= maxRank; r++)
        {
            std::vector<int> frontIdx;
            for (int i = 0; i < (int)pop.size(); i++)
            {
                if (pop[i].rank == r)
                    frontIdx.push_back(i);
            }
            computeCrowdingDistance(pop, frontIdx);
        }
    }
};
