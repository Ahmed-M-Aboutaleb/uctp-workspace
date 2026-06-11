# Cultural Algorithm — University Course Timetabling

C++ implementation of:

> Soza, Landa, Riff, Coello — *A Cultural Algorithm with Operator
> Parameters Control for Solving Timetabling Problems.*

Solves the **University Course Timetabling Problem (UCTP)** of the
Metaheuristics Network competition (45 timeslots = 5 days × 9 hours).

## Build

```bash
make            # builds ./cultural_ca
make quick      # 30 s smoke test on the bundled medium3 instance
make run        # full 360 s run as in the paper
make clean
```

## Usage

```
./cultural_ca <dataset> [options]

  -t, --time   <seconds>   wall-clock time limit (default 360)
  -g, --gens   <num>       max generations cap
  -p, --pop    <size>      population size (default 10)
  -s, --seed   <uint>      RNG seed (0 = random)
      --quiet              less verbose output
```

Example:

```bash
./cultural_ca ../attached_assets/medium3_1777391097129.txt -t 360 -s 42
```

## What is implemented

Every component of the paper is in here:

| Paper section                          | File                          |
|----------------------------------------|-------------------------------|
| Problem statement (H1–H3, S1–S3)       | `Problem.{hpp,cpp}` + `Individual::evaluate` |
| Matrix representation (rooms × slots)  | `Individual.hpp`              |
| Algorithm 1 main loop                  | `CulturalAlgorithm::run`      |
| Cultural mutation + Algorithm 2        | `culturalMutation`, `applyMutationMove` |
| **Situational** influence (leader)     | `pickBySituational` + `BeliefSpace::leader` |
| **Normative** influence (matrix `M`)   | `pickByNormative` + `BeliefSpace::normativeM` |
| **Domain** knowledge (event ranking & last-slot avoidance) | `Problem::rankingByConflict`, `repairOperator` |
| Interchange / Sequencing / Simple-mutation operators | `interchangeOperator`, `sequencingOperator`, `simpleMutationOperator` |
| Roulette-wheel parameter control with α factor and `[0.1, 0.8]` bounds | `chooseExplorationOperator`, `updateOperatorRates` |
| Repair operator (a-priori domain knowledge) | `repairOperator`         |
| Selection (μ+λ elitist replacement)    | `selection`                   |

## Output

After running the program prints a summary and writes the solution as
`event_id → (room, timeslot)` pairs to `solution.txt`:

```
Hard violations: 0
Soft violations: 127
Fitness        : 127
Feasible       : YES
```

## Dataset format (Ben Paechter, ITC-2002)

```
events rooms features students
<room sizes>                      (rooms lines)
<student attends event>           (students × events lines, 0/1)
<room has feature>                (rooms × features lines, 0/1)
<event needs feature>             (events × features lines, 0/1)
```

The bundled `medium3` instance has 400 events, 10 rooms, 5 features, 200
students.
