#pragma once
#include <vector>
#include <string>
#include <set>

struct Room {
    int id;
    int capacity;
};

struct Event {
    int              id;
    int              numStudents;
    std::vector<int> suitableRooms;
    std::vector<int> availableTimeSlots;
    std::vector<int> students;
    std::vector<int> mustBeBefore;
    bool             hasTimeRestriction = false;
};

struct Problem {
    std::string name;
    int         numEvents    = 0;
    int         numRooms     = 0;
    int         numFeatures  = 0;
    int         numTimeslots = 45;  // ITC-2007: 9 slots/day × 5 days
    int         numStudents  = 0;
    int         slotsPerDay = 9;

    std::vector<Room>  rooms;
    std::vector<Event> events;

    std::vector<std::vector<bool>> conflict;

    std::vector<int> E0_events;
    std::vector<int> T0_slots;
    std::vector<int> EE;

    bool loadFromFile(const std::string& path);

private:
    void buildConflictMatrix(const std::vector<std::vector<int>>& studentEvents);
    void buildE0andT0();
    void buildEE();
};