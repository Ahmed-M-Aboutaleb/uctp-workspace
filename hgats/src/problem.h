#pragma once
#include <vector>
#include <string>
#include <set>

using namespace std;

struct Room {
    int id;
    int capacity;
};

struct Event {
    int              id;
    int              numStudents;
    vector<int> suitableRooms;
    vector<int> availableTimeSlots;
    vector<int> students;
    vector<int> mustBeBefore;
    bool             hasTimeRestriction = false;
};

struct Problem {
    string name;
    int         numEvents    = 0;
    int         numRooms     = 0;
    int         numFeatures  = 0;
    int         numTimeslots = 45;  // ITC-2007: 9 slots/day × 5 days
    int         numStudents  = 0;
    int         slotsPerDay = 9;

    vector<Room>  rooms;
    vector<Event> events;

    vector<vector<bool>> conflict;

    vector<int> E0_events;
    vector<int> T0_slots;
    vector<int> EE;

    bool loadFromFile(const string& path);

private:
    void buildConflictMatrix(const vector<vector<int>>& studentEvents);
    void buildE0andT0();
    void buildEE();
};