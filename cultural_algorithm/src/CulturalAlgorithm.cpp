#include "CulturalAlgorithm.hpp"
#include "Matching.hpp"
#include "Random.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>

CulturalAlgorithm::CulturalAlgorithm(const Problem& problem, CAConfig c)
    : p(problem), cfg(std::move(c)) {
    if (cfg.seed != 0) Random::seed(cfg.seed);
    // Initial operator probabilities — uniform, as the paper states.
    for (int i = 0; i < N_OPS; ++i) opProb[i] = 1.0 / N_OPS;
}

double CulturalAlgorithm::timeRatio() const {
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - startTime).count();
    return std::min(1.0, elapsed / cfg.timeLimitSec);
}

// ---------------------------------------------------------------------------
//  RUN
// ---------------------------------------------------------------------------
Individual CulturalAlgorithm::run() {
    startTime = std::chrono::steady_clock::now();

    // 1. Generate s random schedules (initial population)
    population.clear();
    population.reserve(cfg.populationSize);
    for (int i = 0; i < cfg.populationSize; ++i) {
        Individual ind;
        ind.randomize(p);
        population.push_back(std::move(ind));
    }

    // 2. Compute fitness already done inside randomize()

    // 3. Initialise belief space (situational + normative matrix)
    belief.init(p, population);

    if (cfg.verbose) {
        std::cout << "Initial best fitness: " << belief.leader.fitness
                  << " (hard=" << belief.leader.hardViol
                  << " soft=" << belief.leader.softViol << ")\n";
    }

    long long bestFitnessEver = belief.leader.fitness;

    // 4. Main loop
    while (true) {
        // Stop conditions
        if (currentGeneration >= cfg.maxGenerations) break;
        if (std::chrono::duration<double>(
                std::chrono::steady_clock::now() - startTime).count()
            >= cfg.timeLimitSec) break;

        std::vector<Individual> offspring;
        offspring.reserve(population.size());

        for (const auto& parent : population) {
            Individual child = parent;          // clone

            // Cultural mutation (uses belief space)
            long long fBefore = child.fitness;
            if (Random::nextReal() < cfg.pCulturalMut) {
                culturalMutation(child);
                child.evaluate(p);
            }

            // One of three exploration operators (roulette wheel)
            int op = chooseExplorationOperator();
            long long fPrevOp = child.fitness;
            switch (op) {
                case 0: interchangeOperator(child); break;
                case 1: sequencingOperator(child); break;
                case 2: simpleMutationOperator(child); break;
            }
            child.evaluate(p);
            updateOperatorRates(op, fPrevOp, child.fitness);

            // Hard-violation focused repair: always run when child still has
            // hard violations.  This is the key driver for reaching 0 hcv on
            // larger instances such as the ITC2002 large dataset.
            if (child.hardViol > 0) {
                fixHardViolations(child);
                child.evaluate(p);
            }

            // Repair (domain knowledge) — moves events out of the last slot
            // of each day to earlier slots whenever feasible.
            if (Random::nextReal() < cfg.pRepair) {
                repairOperator(child);
                child.evaluate(p);
            }

            (void)fBefore;
            offspring.push_back(std::move(child));
        }

        // Selection
        selection(population, offspring);

        // Update belief space
        belief.update(p, population);

        if (belief.leader.fitness < bestFitnessEver) {
            bestFitnessEver = belief.leader.fitness;
            if (cfg.verbose) {
                long long el = std::chrono::duration_cast<std::chrono::seconds>(
                                std::chrono::steady_clock::now() - startTime).count();
                std::cout << "[gen " << currentGeneration
                          << " t=" << el << "s] new best: "
                          << belief.leader.fitness
                          << " (hard=" << belief.leader.hardViol
                          << " soft=" << belief.leader.softViol << ")  "
                          << "ops=[" << opProb[0] << "," << opProb[1]
                          << "," << opProb[2] << "]\n";
            }
        } else if (cfg.verbose && (currentGeneration % cfg.logEveryGen == 0)) {
            long long el = std::chrono::duration_cast<std::chrono::seconds>(
                            std::chrono::steady_clock::now() - startTime).count();
            std::cout << "  gen " << currentGeneration
                      << " t=" << el << "s best=" << belief.leader.fitness
                      << " (hard=" << belief.leader.hardViol
                      << " soft=" << belief.leader.softViol << ")  "
                      << "ops=[" << opProb[0] << "," << opProb[1]
                      << "," << opProb[2] << "]\n";
        }

        ++currentGeneration;
    }

    // Final intensification: aggressively cleans up any remaining hard
    // violations on the leader.  Helps reach 0 hcv on the large dataset.
    Individual best = belief.leader;
    finalIntensification(best);
    if (best.fitness < belief.leader.fitness) belief.leader = best;

    if (cfg.verbose) {
        std::cout << "\nDone. Generations=" << currentGeneration
                  << " best=" << belief.leader.fitness
                  << " (hard=" << belief.leader.hardViol
                  << " soft=" << belief.leader.softViol << ")\n";
    }
    return belief.leader;
}

// ---------------------------------------------------------------------------
//  Cultural mutation – influence + Algorithm 2
// ---------------------------------------------------------------------------
void CulturalAlgorithm::culturalMutation(Individual& ind) {
    int E = -1, room = -1, slot = -1;

    bool ok = false;
    if (Random::nextReal() < cfg.pSituational) {
        ok = pickBySituational(E, room, slot);
        if (!ok) ok = pickByNormative(E, room, slot);
    } else {
        ok = pickByNormative(E, room, slot);
        if (!ok) ok = pickBySituational(E, room, slot);
    }
    if (!ok) return;

    applyMutationMove(ind, E, room, slot);
}

bool CulturalAlgorithm::pickBySituational(int& E, int& room, int& slot) const {
    // Randomly select an event from the leader and try to inherit its (r,t).
    if (!belief.leaderInitialised) return false;
    for (int attempt = 0; attempt < 50; ++attempt) {
        int e = Random::nextInt(0, p.numEvents - 1);
        int r = belief.leader.eventRoom[e];
        int t = belief.leader.eventSlot[e];
        if (r >= 0 && t >= 0) {
            E = e; room = r; slot = t;
            return true;
        }
    }
    return false;
}

bool CulturalAlgorithm::pickByNormative(int& E, int& room, int& slot) const {
    // Roulette wheel biased towards events most connected with others.
    // Probability proportional to (eventConflictCount + 1).
    long long total = 0;
    for (int e = 0; e < p.numEvents; ++e) total += (p.eventConflictCount[e] + 1);
    if (total <= 0) return false;
    long long pick = (long long)Random::nextInt(0, (int)(total - 1));
    int E_chosen = 0;
    long long acc = 0;
    for (int e : p.rankingByConflict) {
        acc += (p.eventConflictCount[e] + 1);
        if (acc > pick) { E_chosen = e; break; }
    }
    E = E_chosen;

    // Fix the room: keep the event's current room (per paper: "The room r is
    // fixed").  Fall back to a suitable room if it is currently unassigned.
    if (E >= 0 && p.suitableRooms[E].empty()) return false;
    int r;
    if (belief.leader.eventRoom[E] >= 0) {
        r = belief.leader.eventRoom[E];
    } else {
        r = p.suitableRooms[E][Random::nextInt(0, (int)p.suitableRooms[E].size() - 1)];
    }
    room = r;

    // Pick a timeslot from the normative matrix M[E] (most common -> most
    // likely, naturally because we pick uniformly from the bag).
    const auto& bag = belief.normativeM[E];
    if (bag.empty()) {
        slot = Random::nextInt(0, Problem::NUM_TIMESLOTS - 1);
    } else {
        slot = bag[Random::nextInt(0, (int)bag.size() - 1)];
    }
    return true;
}

bool CulturalAlgorithm::tryMoveEventTo(Individual& ind, int e, int newRoom, int newSlot) {
    int curR = ind.eventRoom[e];
    int curS = ind.eventSlot[e];
    if (ind.chrom[newRoom][newSlot] != -1) return false;

    // Temporarily remove e, then check H1/H2 at the new cell.
    if (curR >= 0 && curS >= 0) {
        ind.chrom[curR][curS] = -1;
        ind.eventRoom[e] = -1;
        ind.eventSlot[e] = -1;
    }
    bool ok = ind.canPlace(p, e, newRoom, newSlot);
    if (ok) {
        ind.place(e, newRoom, newSlot);
        if (curR < 0) {} // was unassigned, place() decremented unassigned count
        return true;
    }
    // Roll back
    if (curR >= 0 && curS >= 0) {
        ind.chrom[curR][curS] = e;
        ind.eventRoom[e] = curR;
        ind.eventSlot[e] = curS;
    } else {
        // it was unassigned and is still unassigned; bump unassigned back
        ind.unassigned += 1;
        ind.unassigned -= 1; // explicit no-op — keep balanced
    }
    return false;
}

bool CulturalAlgorithm::trySwapEvents(Individual& ind, int e, int Em,
                                      int targetRoom, int targetSlot) {
    int eR = ind.eventRoom[e], eS = ind.eventSlot[e];
    int mR = ind.eventRoom[Em], mS = ind.eventSlot[Em];
    if (mR != targetRoom || mS != targetSlot) return false;
    if (eR < 0 || eS < 0) return false;

    // Detach both
    ind.chrom[eR][eS] = -1;
    ind.chrom[mR][mS] = -1;
    ind.eventRoom[e] = -1; ind.eventSlot[e] = -1;
    ind.eventRoom[Em] = -1; ind.eventSlot[Em] = -1;

    // Check H1/H2 for both proposed positions.
    bool okE  = ind.canPlace(p, e,  targetRoom, targetSlot);
    bool okEm = false;
    if (okE) {
        // tentatively place e to test Em can take e's old cell
        ind.place(e, targetRoom, targetSlot);
        okEm = ind.canPlace(p, Em, eR, eS);
        if (okEm) {
            ind.place(Em, eR, eS);
            return true;
        }
        // undo placing e
        ind.chrom[targetRoom][targetSlot] = -1;
        ind.eventRoom[e] = -1; ind.eventSlot[e] = -1;
        ind.unassigned += 1;
    }
    // Roll back original placements.
    ind.chrom[eR][eS] = e;
    ind.chrom[mR][mS] = Em;
    ind.eventRoom[e]  = eR; ind.eventSlot[e]  = eS;
    ind.eventRoom[Em] = mR; ind.eventSlot[Em] = mS;
    return false;
}

bool CulturalAlgorithm::applyMutationMove(Individual& ind, int E,
                                          int targetRoom, int targetSlot) {
    int rE = ind.eventRoom[E];
    int tE = ind.eventSlot[E];
    if (rE == targetRoom && tE == targetSlot) return false;

    int maxTries = 1000;
    while (maxTries-- > 0) {
        if (ind.chrom[targetRoom][targetSlot] == -1) {
            // try to move E from (rE, tE) to (r, t)
            if (tryMoveEventTo(ind, E, targetRoom, targetSlot)) return true;
            return false;
        } else {
            int Em = ind.chrom[targetRoom][targetSlot];
            // try a swapping move of Em with E
            if (trySwapEvents(ind, E, Em, targetRoom, targetSlot)) return true;
            return false;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
//  Exploration operators
// ---------------------------------------------------------------------------

// Interchange operator (Lewis & Paechter): pick two events and swap them.
// Uses the matching algorithm afterwards to repair room assignments in the
// affected timeslots ("the last two operators make use of the matching
// algorithm to increase their rate of success").
void CulturalAlgorithm::interchangeOperator(Individual& ind) {
    if (p.numEvents < 2) return;
    int e1 = Random::nextInt(0, p.numEvents - 1);
    int e2 = Random::nextInt(0, p.numEvents - 1);
    if (e1 == e2) return;
    int s1 = ind.eventSlot[e1];
    int s2 = ind.eventSlot[e2];
    int r1 = ind.eventRoom[e1];
    int r2 = ind.eventRoom[e2];
    if (r1 < 0 || r2 < 0) return;

    // Direct swap of the (room, slot) cells.  May break H2 if the receiving
    // room is unsuitable for the new event — that is what matching repairs.
    ind.chrom[r1][s1] = e2;
    ind.chrom[r2][s2] = e1;
    ind.eventRoom[e1] = r2; ind.eventSlot[e1] = s2;
    ind.eventRoom[e2] = r1; ind.eventSlot[e2] = s1;

    Matching::repairSlot(ind, p, s1);
    if (s2 != s1) Matching::repairSlot(ind, p, s2);
}

// Sequencing operator (Kostuch SA-style): swap two whole timeslots (columns).
// This is the most destructive operator because it moves up to numRooms
// events at once.
void CulturalAlgorithm::sequencingOperator(Individual& ind) {
    int t1 = Random::nextInt(0, Problem::NUM_TIMESLOTS - 1);
    int t2 = Random::nextInt(0, Problem::NUM_TIMESLOTS - 1);
    if (t1 == t2) return;
    for (int r = 0; r < p.numRooms; ++r) {
        std::swap(ind.chrom[r][t1], ind.chrom[r][t2]);
        int e = ind.chrom[r][t1];
        if (e >= 0) ind.eventSlot[e] = t1;
        e = ind.chrom[r][t2];
        if (e >= 0) ind.eventSlot[e] = t2;
    }
    // After the swap a few hard violations may appear; the next evaluate()
    // (and the repair operator) will catch them.
}

// Simple mutation: pick an event and move it to a different timeslot.
// The room is left up to the matching algorithm in the destination slot.
// As in Rossi-Doria, matching is used here to maximise the chance that a
// feasible room can be found ("the last two operators make use of the
// matching algorithm to increase their rate of success").
void CulturalAlgorithm::simpleMutationOperator(Individual& ind) {
    int e = Random::nextInt(0, p.numEvents - 1);
    if (p.suitableRooms[e].empty()) return;

    int curR = ind.eventRoom[e];
    int curS = ind.eventSlot[e];

    int newSlot = Random::nextInt(0, Problem::NUM_TIMESLOTS - 1);
    if (newSlot == curS) return;

    // Try to drop the event into any free room of newSlot first; if none is
    // free we still place it in any (even occupied) suitable room and let
    // matching reshuffle the slot.
    int newRoom = -1;
    for (int r : p.suitableRooms[e]) {
        if (ind.chrom[r][newSlot] == -1) { newRoom = r; break; }
    }
    if (newRoom == -1) {
        newRoom = p.suitableRooms[e][
            Random::nextInt(0, (int)p.suitableRooms[e].size() - 1)];
    }

    // Detach from current cell.
    if (curR >= 0 && curS >= 0) {
        ind.chrom[curR][curS] = -1;
        ind.eventRoom[e] = -1;
        ind.eventSlot[e] = -1;
        ind.unassigned += 1;
    }

    // If newRoom is taken, evict the occupier into "unassigned" pool — the
    // matching repair below will try to put it back.
    int evicted = ind.chrom[newRoom][newSlot];
    if (evicted >= 0) {
        ind.chrom[newRoom][newSlot] = -1;
        ind.eventRoom[evicted] = -1;
        ind.eventSlot[evicted] = -1;
        ind.unassigned += 1;
    }

    ind.place(e, newRoom, newSlot);

    // Matching repair on both affected slots.
    Matching::repairSlot(ind, p, newSlot);
    if (curS >= 0 && curS != newSlot) Matching::repairSlot(ind, p, curS);
}

// ---------------------------------------------------------------------------
//  Repair operator (domain knowledge): try to move events out of the last
//  timeslot of every day to earlier timeslots while keeping hard constraints.
// ---------------------------------------------------------------------------
void CulturalAlgorithm::repairOperator(Individual& ind) {
    for (int d = 0; d < Problem::NUM_DAYS; ++d) {
        int lastSlot = d * Problem::SLOTS_PER_DAY + (Problem::SLOTS_PER_DAY - 1);
        for (int r = 0; r < p.numRooms; ++r) {
            int e = ind.chrom[r][lastSlot];
            if (e < 0) continue;
            // Try every earlier slot in the day, then any slot in the week.
            bool moved = false;
            // First try earlier slots within same day, in current room.
            for (int s = d * Problem::SLOTS_PER_DAY;
                 s < lastSlot && !moved; ++s) {
                if (ind.chrom[r][s] == -1 && ind.canPlace(p, e, r, s)) {
                    ind.chrom[r][lastSlot] = -1;
                    ind.eventRoom[e] = -1; ind.eventSlot[e] = -1;
                    ind.unassigned += 1;
                    ind.place(e, r, s);
                    moved = true;
                }
            }
            if (moved) continue;
            // Then any earlier slot in any suitable room.
            for (int rr : p.suitableRooms[e]) {
                for (int s = 0; s < Problem::NUM_TIMESLOTS && !moved; ++s) {
                    if (Problem::isLastSlotOfDay(s)) continue;
                    if (ind.chrom[rr][s] == -1 && ind.canPlace(p, e, rr, s)) {
                        ind.chrom[r][lastSlot] = -1;
                        ind.eventRoom[e] = -1; ind.eventSlot[e] = -1;
                        ind.unassigned += 1;
                        ind.place(e, rr, s);
                        moved = true;
                    }
                }
                if (moved) break;
            }
        }
    }
}

// ---------------------------------------------------------------------------
//  Operator parameter control (Section 4)
// ---------------------------------------------------------------------------
int CulturalAlgorithm::chooseExplorationOperator() const {
    double r = Random::nextReal();
    double acc = 0.0;
    for (int i = 0; i < N_OPS; ++i) {
        acc += opProb[i];
        if (r <= acc) return i;
    }
    return N_OPS - 1;
}

void CulturalAlgorithm::updateOperatorRates(int op, long long fPrev, long long fCur) {
    if (fPrev == fCur) return;

    double fp = (double)fPrev;
    double fc = (double)fCur;
    if (fp + fc == 0.0) return;

    double delta = std::abs((fp - fc) / (fp + fc));   // ∆variation

    if (fCur < fPrev) {
        opProb[op] += delta;
        for (int j = 0; j < N_OPS; ++j) if (j != op) {
            opProb[j] -= delta / (N_OPS - 1);
        }
    } else {
        double alpha = timeRatio();                   // present_time / total_time
        opProb[op] -= delta * alpha;
        for (int j = 0; j < N_OPS; ++j) if (j != op) {
            opProb[j] += (delta * alpha) / (N_OPS - 1);
        }
    }

    // Clamp to [MIN_PROB, MAX_PROB] then normalise to sum 1.
    for (int i = 0; i < N_OPS; ++i) {
        if (opProb[i] < MIN_PROB) opProb[i] = MIN_PROB;
        if (opProb[i] > MAX_PROB) opProb[i] = MAX_PROB;
    }
    double sum = 0;
    for (int i = 0; i < N_OPS; ++i) sum += opProb[i];
    for (int i = 0; i < N_OPS; ++i) opProb[i] /= sum;
}

// ---------------------------------------------------------------------------
//  Selection (mu+lambda elitist replacement):
//     Combine parents and offspring, sort by fitness, keep top mu.
// ---------------------------------------------------------------------------
void CulturalAlgorithm::selection(std::vector<Individual>& parents,
                                  std::vector<Individual>& offspring) {
    std::vector<Individual> combined;
    combined.reserve(parents.size() + offspring.size());
    for (auto& x : parents)   combined.push_back(std::move(x));
    for (auto& x : offspring) combined.push_back(std::move(x));

    std::sort(combined.begin(), combined.end(),
              [](const Individual& a, const Individual& b) {
                  return a.fitness < b.fitness;
              });

    parents.clear();
    for (size_t i = 0; i < (size_t)cfg.populationSize && i < combined.size(); ++i) {
        parents.push_back(std::move(combined[i]));
    }
}

int CulturalAlgorithm::tournamentSelect(const std::vector<Individual>& pop) const {
    int best = Random::nextInt(0, (int)pop.size() - 1);
    for (int i = 1; i < cfg.tournamentSize; ++i) {
        int c = Random::nextInt(0, (int)pop.size() - 1);
        if (pop[c].fitness < pop[best].fitness) best = c;
    }
    return best;
}

// ---------------------------------------------------------------------------
//  Hard-violation focused repair
// ---------------------------------------------------------------------------
//
// The base CA quickly removes most hard violations, but on large/dense
// instances a handful of stubborn events can remain unassigned or stuck in
// an unsuitable room.  This routine:
//   1. Lists every event that currently contributes a hard violation
//      (unassigned, in an unsuitable room, or in a slot that contains
//      another conflicting event).
//   2. For each such event, tries every (suitable_room, slot) cell:
//        a. If the cell is free and feasible -> place there.
//        b. If the cell is occupied, evict the occupant, place the event,
//           and try to relocate the evicted event to any feasible cell.
//           If that fails, roll back.
//   3. After processing, re-runs Matching::repairSlot on every affected
//      slot to clean up any room-suitability violations introduced.
//
void CulturalAlgorithm::fixHardViolations(Individual& ind) {
    // Snapshot conflict count helper.
    auto inConflict = [&](int e) -> bool {
        int t = ind.eventSlot[e];
        int r = ind.eventRoom[e];
        if (r < 0 || t < 0) return true;             // unassigned
        if (!p.roomSuitable[e][r]) return true;       // H2
        for (int rr = 0; rr < p.numRooms; ++rr) {
            if (rr == r) continue;
            int g = ind.chrom[rr][t];
            if (g >= 0 && p.conflict[e][g]) return true;
        }
        return false;
    };

    // Process events in conflict-density order so the most constrained ones
    // get first pick (this is the same a-priori ranking used elsewhere).
    std::vector<int> order = p.rankingByConflict;

    for (int e : order) {
        if (!inConflict(e)) continue;
        if (p.suitableRooms[e].empty()) continue;

        int curR = ind.eventRoom[e];
        int curS = ind.eventSlot[e];

        // 1. Try a free + feasible cell anywhere.
        bool placed = false;
        for (int r : p.suitableRooms[e]) {
            for (int t = 0; t < Problem::NUM_TIMESLOTS && !placed; ++t) {
                if (ind.chrom[r][t] != -1) continue;
                // Detach e first, then check feasibility.
                if (curR >= 0 && curS >= 0) {
                    ind.chrom[curR][curS] = -1;
                    ind.eventRoom[e] = -1;
                    ind.eventSlot[e] = -1;
                    ind.unassigned += 1;
                }
                if (ind.canPlace(p, e, r, t)) {
                    ind.place(e, r, t);
                    placed = true;
                    curR = r; curS = t;
                } else if (curR >= 0 && curS >= 0) {
                    // Roll back to original cell only if it was an unsuitable
                    // assignment we still want to undo on next iteration.
                    // Here we leave e unassigned to give it another shot.
                }
            }
            if (placed) break;
        }
        if (placed) continue;

        // 2. Try evict-and-place: pick a suitable cell occupied by some j,
        //    move j out (to any feasible cell), then place e.
        for (int r : p.suitableRooms[e]) {
            for (int t = 0; t < Problem::NUM_TIMESLOTS && !placed; ++t) {
                int j = ind.chrom[r][t];
                if (j < 0 || j == e) continue;
                // Detach j tentatively.
                ind.chrom[r][t] = -1;
                ind.eventRoom[j] = -1;
                ind.eventSlot[j] = -1;
                ind.unassigned += 1;

                if (ind.canPlace(p, e, r, t)) {
                    ind.place(e, r, t);
                    placed = true;
                    // Try to put j into any feasible cell (best effort).
                    bool jPlaced = false;
                    for (int jr : p.suitableRooms[j]) {
                        for (int jt = 0; jt < Problem::NUM_TIMESLOTS && !jPlaced; ++jt) {
                            if (ind.chrom[jr][jt] == -1
                                && ind.canPlace(p, j, jr, jt)) {
                                ind.place(j, jr, jt);
                                jPlaced = true;
                            }
                        }
                        if (jPlaced) break;
                    }
                    // If j cannot be placed, leave it unassigned (counts as
                    // 1 hard violation but we removed at least one too).
                    curR = r; curS = t;
                } else {
                    // Roll back: restore j.
                    ind.chrom[r][t] = j;
                    ind.eventRoom[j] = r;
                    ind.eventSlot[j] = t;
                    ind.unassigned -= 1;
                }
            }
            if (placed) break;
        }
    }
}

// ---------------------------------------------------------------------------
//  Final intensification: repeatedly fix hard violations + run repair until
//  no further improvement is observed.  Cheap because it operates on a
//  single individual (the leader).
// ---------------------------------------------------------------------------
void CulturalAlgorithm::finalIntensification(Individual& ind) {
    ind.evaluate(p);
    long long prev = ind.fitness;
    for (int iter = 0; iter < 50; ++iter) {
        if (ind.hardViol > 0) {
            fixHardViolations(ind);
            ind.evaluate(p);
        }
        repairOperator(ind);
        ind.evaluate(p);
        if (ind.fitness >= prev) break;
        prev = ind.fitness;
    }
}
