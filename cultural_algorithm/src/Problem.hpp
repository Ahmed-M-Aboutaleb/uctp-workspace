#ifndef PROBLEM_HPP
#define PROBLEM_HPP

#include <string>
#include <vector>

class Problem {
public:
    static constexpr int NUM_TIMESLOTS = 45;
    static constexpr int NUM_DAYS      = 5;
    static constexpr int SLOTS_PER_DAY = 9;

    int numEvents   = 0;
    int numRooms    = 0;
    int numFeatures = 0;
    int numStudents = 0;

    std::vector<int> roomSize;                              // [numRooms]

    // ---------------------------------------------------------------------
    // The five matrices of the UCTP formulation.
    //   A_{k,n}  Student-Event   -- studentAttends[student][event]
    //   B_{n,n}  Event-Conflict  -- conflict[event][event]   (computed from A)
    //   C_{m,l}  Room-Features   -- roomFeatures[room][feature]
    //   D_{n,l}  Event-Features  -- eventFeatures[event][feature]
    //   G_{n,m}  Event-Room      -- roomSuitable[event][room] (capacity AND features)
    // ---------------------------------------------------------------------
    std::vector<std::vector<bool>> studentAttends;          // A
    std::vector<std::vector<bool>> roomFeatures;            // C
    std::vector<std::vector<bool>> eventFeatures;           // D

    std::vector<std::vector<bool>> conflict;                // B
    std::vector<std::vector<bool>> roomSuitable;            // G
    std::vector<std::vector<int>>  suitableRooms;           // G as adjacency lists
    std::vector<int> eventStudentCount;                     // [events] -> #students attending
    std::vector<int> eventConflictCount;                    // [events] -> #other events sharing students
    std::vector<std::vector<int>> studentEvents;            // [students] -> events attending
    std::vector<int> rankingByConflict;                     // events sorted by descending eventConflictCount

    bool load(const std::string& path);

    // Helpers ------------------------------------------------------------
    static int dayOfSlot(int slot)        { return slot / SLOTS_PER_DAY; }
    static int hourOfSlot(int slot)       { return slot % SLOTS_PER_DAY; }
    static bool isLastSlotOfDay(int slot) { return hourOfSlot(slot) == SLOTS_PER_DAY - 1; }
};

#endif
