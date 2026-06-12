#include "problem.h"
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std;

bool Problem::loadFromFile(const string& path) {
    ifstream fin(path);
    if (!fin.is_open()) {
        cerr << "[ERROR] Cannot open: " << path << "\n";
        return false;
    }

    size_t slash = path.find_last_of("/\\");
    name = (slash == string::npos) ? path : path.substr(slash + 1);
    size_t dot = name.find_last_of('.');
    if (dot != string::npos) name = name.substr(0, dot);

    fin >> numEvents >> numRooms >> numFeatures >> numStudents;
    numTimeslots = 45;

    cout << "[INFO] Loading: " << name
              << "  E=" << numEvents
              << "  R=" << numRooms
              << "  F=" << numFeatures
              << "  S=" << numStudents << "\n";

    events.resize(numEvents);
    for (int e = 0; e < numEvents; ++e) events[e].id = e;

    rooms.resize(numRooms);
    for (int r = 0; r < numRooms; ++r) {
        rooms[r].id = r;
        fin >> rooms[r].capacity;
    }

    // student-event: build per-event student list
    // and per-student event list for fast conflict detection
    vector<vector<int>> studentEvents(numStudents);

    for (int e = 0; e < numEvents; ++e) {
        events[e].students.clear();
        events[e].numStudents = 0;
    }
    for (int s = 0; s < numStudents; ++s) {
        for (int e = 0; e < numEvents; ++e) {
            int val; fin >> val;
            if (val == 1) {
                events[e].students.push_back(s);
                ++events[e].numStudents;
                studentEvents[s].push_back(e);
            }
        }
    }

    // room features
    vector<vector<bool>> roomFeature(
        numRooms, vector<bool>(numFeatures, false));
    for (int r = 0; r < numRooms; ++r)
        for (int f = 0; f < numFeatures; ++f) {
            int val; fin >> val;
            roomFeature[r][f] = (val == 1);
        }

    // event features
    vector<vector<bool>> eventFeature(
        numEvents, vector<bool>(numFeatures, false));
    for (int e = 0; e < numEvents; ++e)
        for (int f = 0; f < numFeatures; ++f) {
            int val; fin >> val;
            eventFeature[e][f] = (val == 1);
        }

    // room suitability
    for (int e = 0; e < numEvents; ++e) {
        events[e].suitableRooms.clear();
        for (int r = 0; r < numRooms; ++r) {
            bool suitable = true;
            for (int f = 0; f < numFeatures; ++f) {
                if (eventFeature[e][f] && !roomFeature[r][f]) {
                    suitable = false;
                    break;
                }
            }
            if (suitable && rooms[r].capacity >= events[e].numStudents)
                events[e].suitableRooms.push_back(r);
        }
    }

    // availability
    for (int e = 0; e < numEvents; ++e) {
        events[e].availableTimeSlots.clear();
        for (int t = 0; t < numTimeslots; ++t) {
            int val; fin >> val;
            if (val != 0)
                events[e].availableTimeSlots.push_back(t);
        }
        events[e].hasTimeRestriction =
            ((int)events[e].availableTimeSlots.size() < numTimeslots);
    }

    // precedence
    for (int e1 = 0; e1 < numEvents; ++e1) {
        events[e1].mustBeBefore.clear();
        for (int e2 = 0; e2 < numEvents; ++e2) {
            int val; fin >> val;
            if (val == 1)
                events[e1].mustBeBefore.push_back(e2);
        }
    }

    fin.close();

    buildConflictMatrix(studentEvents);
    buildE0andT0();
    buildEE();

    cout << "[INFO] Loaded OK. "
              << "E0=" << E0_events.size()
              << "  T0=" << T0_slots.size()
              << "  EE=" << EE.size() << "\n";

    return true;
}

void Problem::buildConflictMatrix(
    const vector<vector<int>>& studentEvents) {

    conflict.assign(numEvents, vector<bool>(numEvents, false));

    // O(S × E_per_student²) بدل O(E² × S²)
    for (int s = 0; s < numStudents; ++s) {
        const auto& evts = studentEvents[s];
        for (int i = 0; i < (int)evts.size(); ++i)
            for (int j = i + 1; j < (int)evts.size(); ++j) {
                conflict[evts[i]][evts[j]] = true;
                conflict[evts[j]][evts[i]] = true;
            }
    }
}

void Problem::buildE0andT0() {
    E0_events.clear();
    T0_slots.clear();

    for (int e = 0; e < numEvents; ++e)
        if (!events[e].hasTimeRestriction)
            E0_events.push_back(e);

    // T0_slots = timeslots مش مقيدة بأي event
    vector<bool> isRestricted(numTimeslots, false);
    for (int e = 0; e < numEvents; ++e)
        if (events[e].hasTimeRestriction)
            for (int t : events[e].availableTimeSlots)
                isRestricted[t] = true;

    for (int t = 0; t < numTimeslots; ++t)
        if (!isRestricted[t])
            T0_slots.push_back(t);

    if (T0_slots.empty()) {
        for (int t = 0; t < numTimeslots; ++t)
            T0_slots.push_back(t);
        cout << "[INFO] T0_slots empty, using all timeslots\n";
    }

    if (E0_events.empty()) {
        for (int e = 0; e < numEvents; ++e)
            E0_events.push_back(e);
        cout << "[INFO] E0_events empty, using all events\n";
    }
}

void Problem::buildEE() {
    EE.clear();
    for (int e = 0; e < numEvents; ++e)
        if (!events[e].mustBeBefore.empty())
            EE.push_back(e);
}