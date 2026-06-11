#include "Matching.hpp"

#include <functional>
#include <vector>

namespace Matching {

void repairSlot(Individual& ind, const Problem& p, int slot) {
    // 1. Collect the events currently scheduled in this slot.
    std::vector<int> events;
    events.reserve(p.numRooms);
    for (int r = 0; r < p.numRooms; ++r) {
        int e = ind.chrom[r][slot];
        if (e >= 0) events.push_back(e);
    }
    int n = (int)events.size();
    if (n == 0) return;

    // 2. Augmenting-path bipartite matching (Hungarian / Kuhn algorithm).
    //    Left side  : event index 0..n-1
    //    Right side : room index 0..numRooms-1
    //    Edge (u, r): exists iff p.roomSuitable[events[u]][r]   <-> matrix G
    std::vector<int> matchE2R(n,           -1);
    std::vector<int> matchR2E(p.numRooms,  -1);

    std::function<bool(int, std::vector<char>&)> dfs =
        [&](int u, std::vector<char>& visited) -> bool {
            for (int r : p.suitableRooms[events[u]]) {
                if (visited[r]) continue;
                visited[r] = 1;
                if (matchR2E[r] == -1 || dfs(matchR2E[r], visited)) {
                    matchE2R[u]  = r;
                    matchR2E[r]  = u;
                    return true;
                }
            }
            return false;
        };

    for (int u = 0; u < n; ++u) {
        std::vector<char> visited(p.numRooms, 0);
        dfs(u, visited);
    }

    // 3. Clear the slot and re-place the matched events.  Any event without
    //    a feasible room becomes unassigned (counted as a hard violation in
    //    the fitness function).
    for (int r = 0; r < p.numRooms; ++r) {
        int e = ind.chrom[r][slot];
        if (e >= 0) {
            ind.chrom[r][slot] = -1;
            ind.eventRoom[e]   = -1;
            ind.eventSlot[e]   = -1;
            ind.unassigned    += 1;
        }
    }
    for (int u = 0; u < n; ++u) {
        if (matchE2R[u] >= 0) {
            ind.place(events[u], matchE2R[u], slot);
        }
    }
}

}  // namespace Matching
