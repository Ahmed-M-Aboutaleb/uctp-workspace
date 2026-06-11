#ifndef INDIVIDUAL_HPP
#define INDIVIDUAL_HPP

#include "Problem.hpp"
#include <vector>

// Hard-violation penalty C used in the UCTP objective function
//     f(s) = #hcv(s) * C + #scv(s)
// As specified in the formulation, C = 10^6 — larger than the maximum
// possible number of soft-constraint violations.
constexpr long long HARD_PENALTY = 1000000;

class Individual {
public:
    // chrom[room][slot] = event id, or -1 if cell is free.
    std::vector<std::vector<int>> chrom;

    // eventRoom[e] / eventSlot[e] = (-1,-1) if event e is unassigned.
    std::vector<int> eventRoom;
    std::vector<int> eventSlot;

    int unassigned = 0;
    int hardViol   = 0;
    int softViol   = 0;
    long long fitness = 0;

    void init(const Problem& p);

    // Random feasibility-aware initial schedule.
    void randomize(const Problem& p);

    // Place an event into (room, slot). Cell must be empty and event must be
    // currently unassigned. No constraint check here — caller is responsible.
    void place(int e, int room, int slot);

    // Remove event from its current cell, marking it unassigned.
    void unplace(int e);

    // Hard-constraint check helpers.
    // Can event e be legally placed in (room, slot)? Considers H1, H2, H3.
    bool canPlace(const Problem& p, int e, int room, int slot) const;

    // Recompute fitness, hardViol, softViol, unassigned from scratch.
    void evaluate(const Problem& p);
};

#endif
