#include "Problem.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>

bool Problem::load(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "Cannot open dataset: " << path << "\n";
        return false;
    }

    if (!(in >> numEvents >> numRooms >> numFeatures >> numStudents)) {
        std::cerr << "Bad header in dataset.\n";
        return false;
    }

    roomSize.assign(numRooms, 0);
    for (int r = 0; r < numRooms; ++r) {
        if (!(in >> roomSize[r])) { std::cerr << "Bad room sizes.\n"; return false; }
    }

    studentAttends.assign(numStudents, std::vector<bool>(numEvents, false));
    eventStudentCount.assign(numEvents, 0);
    studentEvents.assign(numStudents, {});
    for (int s = 0; s < numStudents; ++s) {
        for (int e = 0; e < numEvents; ++e) {
            int v; in >> v;
            if (v) {
                studentAttends[s][e] = true;
                eventStudentCount[e]++;
                studentEvents[s].push_back(e);
            }
        }
    }

    roomFeatures.assign(numRooms, std::vector<bool>(numFeatures, false));
    for (int r = 0; r < numRooms; ++r) {
        for (int f = 0; f < numFeatures; ++f) {
            int v; in >> v; roomFeatures[r][f] = v != 0;
        }
    }

    eventFeatures.assign(numEvents, std::vector<bool>(numFeatures, false));
    for (int e = 0; e < numEvents; ++e) {
        for (int f = 0; f < numFeatures; ++f) {
            int v; in >> v; eventFeatures[e][f] = v != 0;
        }
    }

    if (!in) {
        std::cerr << "Dataset truncated or malformed.\n";
        return false;
    }

    // Precompute room suitability (H2: features satisfied AND capacity OK)
    roomSuitable.assign(numEvents, std::vector<bool>(numRooms, false));
    suitableRooms.assign(numEvents, {});
    for (int e = 0; e < numEvents; ++e) {
        for (int r = 0; r < numRooms; ++r) {
            if (roomSize[r] < eventStudentCount[e]) continue;
            bool ok = true;
            for (int f = 0; f < numFeatures; ++f) {
                if (eventFeatures[e][f] && !roomFeatures[r][f]) { ok = false; break; }
            }
            if (ok) {
                roomSuitable[e][r] = true;
                suitableRooms[e].push_back(r);
            }
        }
    }

    // Precompute event conflicts (H1) using student->events lists.
    conflict.assign(numEvents, std::vector<bool>(numEvents, false));
    for (int s = 0; s < numStudents; ++s) {
        const auto& evs = studentEvents[s];
        for (size_t i = 0; i < evs.size(); ++i) {
            for (size_t j = i + 1; j < evs.size(); ++j) {
                conflict[evs[i]][evs[j]] = true;
                conflict[evs[j]][evs[i]] = true;
            }
        }
    }

    eventConflictCount.assign(numEvents, 0);
    for (int e = 0; e < numEvents; ++e) {
        int c = 0;
        for (int g = 0; g < numEvents; ++g) if (conflict[e][g]) ++c;
        eventConflictCount[e] = c;
    }

    // Domain knowledge: ranking of events ordered by # of events with shared
    // students among them (most connected first). Used by normative influence.
    rankingByConflict.resize(numEvents);
    std::iota(rankingByConflict.begin(), rankingByConflict.end(), 0);
    std::sort(rankingByConflict.begin(), rankingByConflict.end(),
              [this](int a, int b) {
                  return eventConflictCount[a] > eventConflictCount[b];
              });

    std::cout << "Loaded instance: events=" << numEvents
              << " rooms="    << numRooms
              << " features=" << numFeatures
              << " students=" << numStudents << "\n";

    long long totalSuitable = 0;
    for (auto& v : suitableRooms) totalSuitable += (long long)v.size();
    std::cout << "Average suitable rooms per event: "
              << (double)totalSuitable / numEvents << "\n";

    return true;
}
