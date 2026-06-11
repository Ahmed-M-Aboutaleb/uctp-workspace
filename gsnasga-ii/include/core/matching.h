#pragma once
#include <vector>
#include <algorithm>
#include "problem.h"

// Bipartite matching (Hungarian/augmenting path) for room allocation
// Returns: assignment[event_index] = room_index, or -1 if unassigned
// Only events in 'events' list are matched; 'possible' is the eventRoom matrix

class Matching
{
public:
    // Augmenting path for bipartite matching
    static bool augment(int u, const std::vector<int> &events,
                        const Problem &pb,
                        std::vector<int> &matchEvent, // matchEvent[room] = event
                        std::vector<bool> &visited)
    {
        for (int r = 0; r < pb.m; r++)
        {
            if (!visited[r] && pb.eventRoom[u][r] == 1)
            {
                visited[r] = true;
                if (matchEvent[r] == -1 || augment(matchEvent[r], events, pb, matchEvent, visited))
                {
                    matchEvent[r] = u;
                    return true;
                }
            }
        }
        return false;
    }

    // Given a list of events in a time slot, assign rooms using max bipartite matching
    // Returns assignment: result[i] = room for events[i], or -1 if no room found
    static std::vector<int> assignRooms(const std::vector<int> &events, const Problem &pb)
    {
        int ne = (int)events.size();
        std::vector<int> matchEvent(pb.m, -1); // matchEvent[room] = event_idx_in_events
        std::vector<int> result(ne, -1);

        for (int i = 0; i < ne; i++)
        {
            std::vector<bool> visited(pb.m, false);
            augment(events[i], events, pb, matchEvent, visited);
        }

        // Build result
        for (int r = 0; r < pb.m; r++)
        {
            if (matchEvent[r] != -1)
            {
                // Find the index of this event in the events list
                for (int i = 0; i < ne; i++)
                {
                    if (events[i] == matchEvent[r])
                    {
                        result[i] = r;
                        break;
                    }
                }
            }
        }
        return result;
    }
};
