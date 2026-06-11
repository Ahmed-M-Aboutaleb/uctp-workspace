# HGATS for PECTP (ITC 2007)

## A Professional Technical Overview

---

## 1. Problem Definition

This project addresses the **Post-Enrolment Course Timetabling Problem (PECTP)** as defined in the ITC 2007 benchmark.

The goal is to assign each event (lecture) to a specific **timeslot** and **room** while:

* Satisfying all **hard constraints** (feasibility)
* Minimizing **soft constraint penalties** (solution quality)

---

## 2. Input Data Structure (.tim files)

The dataset consists of the following core components:

### 2.1 Events

Each event represents a single lecture that must be scheduled.

### 2.2 Students

Each student is uniquely identified and enrolled in a subset of events.

### 2.3 Enrolment Data

Defines which students attend which events:

```
Student → {Event1, Event2, ...}
```

This is the foundation for computing **conflicts**.

### 2.4 Rooms

Each room has:

* A fixed capacity
* Suitability constraints for events

### 2.5 Timeslots

Discrete time periods available for scheduling.

---

## 3. Conflict Matrix

A key derived structure:

```
conflict[e1][e2] = number of students enrolled in both events
```

If conflict > 0 → the two events cannot be scheduled in the same timeslot.

---

## 4. Solution Representation

### Assignment Structure

```cpp
struct Assignment {
    int timeslot;
    int room;
};
```

### Individual (Solution)

```cpp
std::vector<Assignment> assignment;
```

Each index corresponds to an event, and the value specifies its scheduled time and room.

---

## 5. Evaluation Function

### 5.1 Hard Constraints (Distance to Feasibility - Df)

A solution is **feasible only if Df = 0**.

Typical constraints:

1. **Student Conflict**: Events sharing students cannot occur simultaneously
2. **Room Occupancy**: Only one event per room per timeslot
3. **Room Suitability**: Event must be assigned to a valid room
4. **Time Availability**: Event must respect allowed timeslots

---

### 5.2 Soft Constraints (SCP)

Soft constraints evaluate solution quality:

* Spread of events
* Late scheduling penalties
* Student isolation
* Room stability

Total soft penalty:

```
SCP = Σ(weight × violations)
```

---

## 6. Fitness Function

To enforce feasibility priority:

```
fitness = Df × M + SCP
```

Where **M** is a large constant.

This ensures:

* Feasible solutions are always preferred
* Optimization focuses on SCP after feasibility is achieved

---

## 7. HGATS Algorithm Overview

HGATS combines:

* **Genetic Search (GSGA)**
* **Tabu Search (TS)**

---

### Phase I: GSGA (Genetic Algorithm)

* Initializes a population of solutions
* Applies crossover, mutation, and local search
* Focuses on reaching feasibility

Time allocation: ~50% of total runtime

---

### Phase II: Tabu Search

* Starts from the best solution of Phase I
* Explores neighborhood solutions
* Avoids cycling using a tabu list
* Focuses on improving SCP

Time allocation: remaining runtime

---

## 8. Execution Flow

```
Load Problem
   ↓
Run GSGA (Phase I)
   ↓
Best Intermediate Solution
   ↓
Run Tabu Search (Phase II)
   ↓
Final Optimized Solution
```

---

## 9. Code Structure Explanation

### 9.1 HGATS::solve()

* Initializes timing
* Executes GSGA
* Evaluates feasibility
* Runs Tabu Search if needed
* Returns best solution

---

### 9.2 printResults()

Outputs:

* Distance to Feasibility (Df)
* Soft Constraint Penalty (SCP)
* Fitness value

---

### 9.3 main()

* Handles user input
* Loads datasets
* Configures parameters
* Executes solver per instance
* Reports results

---

## 10. Memory Structure (MEM)

MEM stores promising assignments to guide search.

### Key Idea:

* Extract good (room, timeslot) pairs from high-quality individuals
* Reuse them in future generations

---

### MEM Construction

* Sort population by fitness
* Select top α portion
* For each event:

  * If no violations → store assignment

---

### Fast Evaluation (eventPenaltyFast)

Efficiently checks:

* Room suitability
* Time validity
* Conflicts (via timeslot index)
* Room clashes

---

## 11. System-Level View

```
Input Data
↓
Conflict Matrix
↓
Solution Representation
↓
Evaluation Function
↓
GSGA (Exploration)
↓
Tabu Search (Exploitation)
↓
Final Solution
```

---

## 12. Common Issues

### SCP Always Zero

* Soft constraints not implemented or activated

### High Df Values

* Weak mutation/local search
* Incorrect constraint evaluation

---

## 13. Possible Extensions

* Instructor constraints
* Multi-objective optimization
* Adaptive parameter tuning
* Hybrid local search strategies

---

## 14. Conclusion

This system models a complex real-world scheduling problem using a hybrid metaheuristic approach.

Key principles:

* Separate feasibility and optimization
* Use hybrid search (global + local)
* Leverage memory for intensification

---

**This implementation reflects a scalable and research-level solution for PECTP.**
