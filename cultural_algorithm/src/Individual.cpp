#include "Individual.hpp"
#include "Random.hpp"

#include <algorithm>
#include <numeric>

void Individual::init(const Problem& p) {
    chrom.assign(p.numRooms, std::vector<int>(Problem::NUM_TIMESLOTS, -1));
    eventRoom.assign(p.numEvents, -1);
    eventSlot.assign(p.numEvents, -1);
    unassigned = p.numEvents;
    hardViol   = 0;
    softViol   = 0;
    fitness    = 0;
}

void Individual::place(int e, int room, int slot) {
    chrom[room][slot] = e;
    eventRoom[e]     = room;
    eventSlot[e]     = slot;
    --unassigned;
}

void Individual::unplace(int e) {
    int r = eventRoom[e];
    int s = eventSlot[e];
    if (r >= 0 && s >= 0) {
        chrom[r][s] = -1;
        eventRoom[e] = -1;
        eventSlot[e] = -1;
        ++unassigned;
    }
}

bool Individual::canPlace(const Problem& p, int e, int room, int slot) const {
    // H3: cell must be empty
    if (chrom[room][slot] != -1) return false;
    // H2: room suitable
    if (!p.roomSuitable[e][room]) return false;
    // H1: any other event in this column shares students with e?
    for (int r = 0; r < p.numRooms; ++r) {
        int g = chrom[r][slot];
        if (g >= 0 && p.conflict[e][g]) return false;
    }
    return true;
}

void Individual::randomize(const Problem& p) {
    init(p);

    std::vector<int> order(p.numEvents);
    std::iota(order.begin(), order.end(), 0);
    std::shuffle(order.begin(), order.end(), Random::engine());

    for (int e : order) {
        // Try a number of random placements respecting H1/H2/H3.
        bool placed = false;
        const auto& rooms = p.suitableRooms[e];
        if (!rooms.empty()) {
            for (int attempt = 0; attempt < 30 && !placed; ++attempt) {
                int room = rooms[Random::nextInt(0, (int)rooms.size() - 1)];
                int slot = Random::nextInt(0, Problem::NUM_TIMESLOTS - 1);
                if (canPlace(p, e, room, slot)) {
                    place(e, room, slot);
                    placed = true;
                }
            }
            if (!placed) {
                // Scan for first feasible cell.
                for (int room : rooms) {
                    for (int slot = 0; slot < Problem::NUM_TIMESLOTS && !placed; ++slot) {
                        if (canPlace(p, e, room, slot)) {
                            place(e, room, slot);
                            placed = true;
                        }
                    }
                    if (placed) break;
                }
            }
        }
        // If still not placed, leave unassigned (penalised in fitness as hard).
    }

    evaluate(p);
}

void Individual::evaluate(const Problem& p) {
    hardViol   = 0;
    softViol   = 0;

    // Hard constraints
    // H3 implicit. H2 and H1 verified by scanning.
    // H2: each placed event in a suitable room.
    for (int e = 0; e < p.numEvents; ++e) {
        int r = eventRoom[e];
        if (r < 0) { ++hardViol; continue; }       // unassigned counts as hard
        if (!p.roomSuitable[e][r]) ++hardViol;
    }
    // H1: for each timeslot, no pair of events shares students.
    for (int t = 0; t < Problem::NUM_TIMESLOTS; ++t) {
        for (int r1 = 0; r1 < p.numRooms; ++r1) {
            int e1 = chrom[r1][t];
            if (e1 < 0) continue;
            for (int r2 = r1 + 1; r2 < p.numRooms; ++r2) {
                int e2 = chrom[r2][t];
                if (e2 < 0) continue;
                if (p.conflict[e1][e2]) ++hardViol;
            }
        }
    }

    // Soft constraints — counted per student per violation as in the
    // International Timetabling Competition fitness function.
    for (int s = 0; s < p.numStudents; ++s) {
        // Build the slots this student attends.
        std::vector<int> slots;
        for (int e : p.studentEvents[s]) {
            int t = eventSlot[e];
            if (t >= 0) slots.push_back(t);
        }
        std::sort(slots.begin(), slots.end());

        // S1: lectures in last slot of any day.
        for (int t : slots) {
            if (Problem::isLastSlotOfDay(t)) ++softViol;
        }

        // S2: more than two lectures in a row (per day).
        // Count runs of consecutive same-day slots; each run of length L
        // contributes max(0, L - 2) to the cost (per ITC2002 definition).
        if (!slots.empty()) {
            int runStart = 0;
            for (size_t i = 1; i <= slots.size(); ++i) {
                bool brk = (i == slots.size())
                        || (slots[i] != slots[i-1] + 1)
                        || (Problem::dayOfSlot(slots[i]) != Problem::dayOfSlot(slots[i-1]));
                if (brk) {
                    int runLen = (int)i - runStart;
                    if (runLen > 2) softViol += (runLen - 2);
                    runStart = (int)i;
                }
            }
        }

        // S3: a single lecture in any given day.
        int perDay[Problem::NUM_DAYS] = {0,0,0,0,0};
        for (int t : slots) perDay[Problem::dayOfSlot(t)]++;
        for (int d = 0; d < Problem::NUM_DAYS; ++d) {
            if (perDay[d] == 1) ++softViol;
        }
    }

    fitness = (long long)hardViol * HARD_PENALTY + softViol;
}
