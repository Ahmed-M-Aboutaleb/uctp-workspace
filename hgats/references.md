# 📚 References – University Timetabling (ITC 2007)

This section presents key academic references related to the Post-Enrolment Course Timetabling Problem (PECTP) and metaheuristic approaches used in solving it.

---

## 🧠 Core Papers

### 1. ITC 2007 Problem Definition
**Lewis, Paechter, McCollum (2007)**  
*Post Enrolment based Course Timetabling: A Description of the Problem Model*  
- Defines the official PECTP model used in ITC 2007  
- Establishes constraints, datasets, and evaluation metrics  
- Serves as the primary reference for this project  

---

### 2. Winning Approach – ITC 2007
**Cambazard et al. (2008)** :contentReference[oaicite:0]{index=0}  
*Local Search and Constraint Programming for PECTP*  
- Winning solution of ITC 2007  
- Combines:
  - Local Search  
  - Constraint Programming  
  - Large Neighborhood Search  
- Demonstrates effectiveness of hybrid approaches  

---

### 3. Metaheuristics Survey
**Lewis (2007)** :contentReference[oaicite:1]{index=1}  
*A Survey of Metaheuristic-based Techniques for University Timetabling Problems*  
- Comprehensive overview of:
  - Genetic Algorithms  
  - Tabu Search  
  - Simulated Annealing  
- Highlights importance of hybrid strategies  

---

## 🔬 Additional Important Works

### 4. PhD Thesis
**Lewis (2006)** :contentReference[oaicite:2]{index=2}  
*Metaheuristics for University Course Timetabling*  
- In-depth study of optimization techniques  
- Foundation for many modern timetabling algorithms  

---

### 5. Evolutionary Scheduling Book Chapter
**Lewis, Paechter, Rossi-Doria (2007)** :contentReference[oaicite:3]{index=3}  
*Metaheuristics for University Course Timetabling*  
- Discusses evolutionary approaches  
- Focuses on balancing constraints  

---

### 6. Hybrid Tabu Search Approach
**Lu & Hao (2008)** :contentReference[oaicite:4]{index=4}  
*Solving the Course Timetabling Problem with a Hybrid Heuristic Algorithm*  
- Introduces Adaptive Tabu Search (ATS)  
- Combines:
  - Tabu Search  
  - Iterated Local Search  
- Improves solution quality via diversification  

---

### 7. Examination Timetabling Model
**McCollum et al. (2007–2008)** :contentReference[oaicite:5]{index=5}  
- Defines exam timetabling track in ITC 2007  
- Extends real-world modeling complexity  

---

### 8. ITC 2007 Overview
**McCollum et al. (2008)** :contentReference[oaicite:6]{index=6}  
*Setting the Research Agenda in Automated Timetabling*  
- Describes competition goals and structure  
- Highlights research challenges and future directions  

---

## 🧠 Summary

- University timetabling is an **NP-hard optimization problem**
- Hybrid metaheuristics (GA + Tabu + Local Search) are the most effective approaches
- ITC 2007 provides a **standard benchmark** for evaluating algorithms
- The proposed HGATS method aligns with state-of-the-art hybrid techniques

---

## 🔥 How This Relates to Our Work

This project adopts a **Hybrid Genetic Algorithm + Tabu Search (HGATS)** approach inspired by:

- Hybrid search strategies (Cambazard et al.)
- Metaheuristic frameworks (Lewis)
- Tabu-based optimization (Lu & Hao)

The goal is to balance:
- **Exploration (Genetic Algorithm)**
- **Exploitation (Tabu Search)**

to efficiently solve the PECTP problem.

---