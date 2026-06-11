#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

struct Problem {
    int n;  // number of events
    int m;  // number of rooms
    int l;  // number of features
    int k;  // number of students
    int p;  // number of time slots (45 = 5 days x 9 hours)

    std::vector<int> roomSize;                   // m
    std::vector<std::vector<int>> studentEvent;  // k x n  (A)
    std::vector<std::vector<int>> eventConflict; // n x n  (B) - derived
    std::vector<std::vector<int>> roomFeatures;  // m x l  (C)
    std::vector<std::vector<int>> eventFeatures; // n x l  (D)
    std::vector<std::vector<int>> eventRoom;     // n x m  (G) - derived
    std::vector<int> eventSize;                  // n - number of students per event

    // Sparse adjacency: conflictAdj[e] = list of events that conflict with e
    std::vector<std::vector<int>> conflictAdj;

    // For each event, which students attend it
    std::vector<std::vector<int>> eventStudents; // n -> list of student ids

    static Problem load(const std::string& filename) {
        std::ifstream fin(filename);
        if (!fin.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }

        Problem pb;
        pb.p = 45; // standard UCTP: 5 days * 9 hours

        fin >> pb.n >> pb.m >> pb.l >> pb.k;

        // Room sizes
        pb.roomSize.resize(pb.m);
        for (int i = 0; i < pb.m; i++) fin >> pb.roomSize[i];

        // Student-Event matrix (k x n)
        pb.studentEvent.assign(pb.k, std::vector<int>(pb.n, 0));
        for (int i = 0; i < pb.k; i++)
            for (int j = 0; j < pb.n; j++)
                fin >> pb.studentEvent[i][j];

        // Room-Features matrix (m x l)
        pb.roomFeatures.assign(pb.m, std::vector<int>(pb.l, 0));
        for (int i = 0; i < pb.m; i++)
            for (int j = 0; j < pb.l; j++)
                fin >> pb.roomFeatures[i][j];

        // Event-Features matrix (n x l)
        pb.eventFeatures.assign(pb.n, std::vector<int>(pb.l, 0));
        for (int i = 0; i < pb.n; i++)
            for (int j = 0; j < pb.l; j++)
                fin >> pb.eventFeatures[i][j];

        fin.close();

        // Derive event sizes
        pb.eventSize.assign(pb.n, 0);
        for (int j = 0; j < pb.n; j++)
            for (int i = 0; i < pb.k; i++)
                pb.eventSize[j] += pb.studentEvent[i][j];

        // Build event->students list
        pb.eventStudents.resize(pb.n);
        for (int e = 0; e < pb.n; e++)
            for (int s = 0; s < pb.k; s++)
                if (pb.studentEvent[s][e]) pb.eventStudents[e].push_back(s);

        // Derive event-conflict matrix B (n x n): share a student
        pb.eventConflict.assign(pb.n, std::vector<int>(pb.n, 0));
        for (int s = 0; s < pb.k; s++) {
            std::vector<int> evts;
            for (int e = 0; e < pb.n; e++)
                if (pb.studentEvent[s][e]) evts.push_back(e);
            for (int i = 0; i < (int)evts.size(); i++)
                for (int j = i + 1; j < (int)evts.size(); j++) {
                    pb.eventConflict[evts[i]][evts[j]] = 1;
                    pb.eventConflict[evts[j]][evts[i]] = 1;
                }
        }

        // Build sparse conflict adjacency list
        pb.conflictAdj.resize(pb.n);
        for (int e = 0; e < pb.n; e++)
            for (int e2 = 0; e2 < pb.n; e2++)
                if (pb.eventConflict[e][e2]) pb.conflictAdj[e].push_back(e2);

        // Derive event-room suitability matrix G (n x m)
        pb.eventRoom.assign(pb.n, std::vector<int>(pb.m, 0));
        for (int e = 0; e < pb.n; e++) {
            for (int r = 0; r < pb.m; r++) {
                if (pb.roomSize[r] >= pb.eventSize[e]) {
                    bool ok = true;
                    for (int f = 0; f < pb.l; f++)
                        if (pb.eventFeatures[e][f] && !pb.roomFeatures[r][f]) { ok = false; break; }
                    if (ok) pb.eventRoom[e][r] = 1;
                }
            }
        }

        return pb;
    }

    void printInfo() const {
        std::cout << "Problem loaded:\n";
        std::cout << "  Events (n)    = " << n << "\n";
        std::cout << "  Rooms  (m)    = " << m << "\n";
        std::cout << "  Features (l)  = " << l << "\n";
        std::cout << "  Students (k)  = " << k << "\n";
        std::cout << "  TimeSlots (p) = " << p << "\n";
        int totalPairs = 0;
        for (int e = 0; e < n; e++)
            for (int r = 0; r < m; r++)
                totalPairs += eventRoom[e][r];
        std::cout << "  Feasible event-room pairs = " << totalPairs << "\n";
        // Average conflict degree
        long long deg = 0;
        for (int e = 0; e < n; e++) deg += conflictAdj[e].size();
        std::cout << "  Avg conflict degree = " << (double)deg / n << "\n";
    }
};
