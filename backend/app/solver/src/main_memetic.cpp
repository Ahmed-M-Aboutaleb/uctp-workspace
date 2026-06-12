#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <random>
#include <numeric>
#include <queue>
#include <set>
#include <chrono>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <bitset>
#include <cstdint>
#include <cassert>
#include <iomanip>
#include <ctime>
#include <filesystem>
#include <future>
#include <thread>
#include <tuple>

// ============================================================================
// 1. UTILITIES & LOGGING
// ============================================================================
enum class LogLevel { INFO, WARN, ERROR, DEBUG };

class Logger {
private:
    bool debug_enabled = false;
    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();

    const char* levelToStr(LogLevel level) const {
        switch (level) {
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARN: return "WARN";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::DEBUG: return "DEBUG";
            default: return "LOG";
        }
    }

public:
    void setDebug(bool enabled) { debug_enabled = enabled; }
    bool isDebugEnabled() const { return debug_enabled; }

    long long elapsedMs() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count();
    }

    void log(LogLevel level, const std::string& msg) {
        if (level == LogLevel::DEBUG && !debug_enabled) return;
        std::cerr << "[" << levelToStr(level) << "][+" << elapsedMs() << "ms] " << msg << "\n";
    }
};

static Logger g_logger;

// ============================================================================
// 2. CONSTANTS & CONFIGURATION
// ============================================================================
constexpr size_t MAX_EVENTS = 400;   
constexpr size_t MAX_ROOMS = 100;    
constexpr size_t MAX_FEATURES = 64;  
constexpr size_t MAX_STUDENTS = 1024; 
constexpr int NUM_TIMESLOTS = 45;    
constexpr int HARD_CONSTRAINT_PENALTY = 1000000; 
constexpr int DAYS = 5;
constexpr int SLOTS_PER_DAY = 9;

// ============================================================================
// 3. DOMAIN STRUCTURES
// ============================================================================
struct Event {
    int id;
    int num_students;
    std::vector<int> students; 
    std::bitset<MAX_FEATURES> features;
};

struct Room {
    int id;
    int capacity;
    std::bitset<MAX_FEATURES> features;
};

struct Fitness {
    long long score = 0; 
    int hcv = 0; 
    int scv = 0; 
    bool is_feasible() const { return hcv == 0; }
    bool operator<(const Fitness& other) const { return score < other.score; }
};

struct Individual {
    int16_t room_id[MAX_EVENTS];
    int16_t timeslot_id[MAX_EVENTS];
    int16_t head[NUM_TIMESLOTS];
    int16_t next_evt[MAX_EVENTS];
    Fitness fitness;

    Individual() {
        for(int t = 0; t < NUM_TIMESLOTS; ++t) head[t] = -1;
        for(int e = 0; e < MAX_EVENTS; ++e) {
            room_id[e] = -1;
            timeslot_id[e] = -1;
            next_evt[e] = -1;
        }
    }

    bool operator<(const Individual& other) const { return fitness < other.fitness; }

    void update_slot(int e_idx, int old_t, int new_t) {
        if (old_t != -1 && old_t != new_t) {
            if (head[old_t] == e_idx) {
                head[old_t] = next_evt[e_idx];
            } else {
                int curr = head[old_t];
                while (curr != -1 && next_evt[curr] != e_idx) curr = next_evt[curr];
                if (curr != -1) next_evt[curr] = next_evt[e_idx];
            }
        }
        if (new_t != -1 && old_t != new_t) {
            next_evt[e_idx] = head[new_t];
            head[new_t] = e_idx;
        }
    }
};

// ============================================================================
// 4. STATIC PROBLEM STATE
// ============================================================================
class UCTPInstance {
public:
    int num_events = 0, num_rooms = 0, num_students = 0, num_features = 0;
    bool is_itc2007 = false;
    std::vector<Event> events; std::vector<Room> rooms;
    std::vector<std::bitset<MAX_EVENTS>> event_conflict; 
    std::vector<std::vector<int>> student_events;        
    std::vector<std::bitset<MAX_ROOMS>> event_room_possible; 

    std::vector<std::bitset<NUM_TIMESLOTS>> event_timeslot_available;
    std::vector<std::pair<int, int>> precedence_rules;

    inline bool can_room_host_event(int r_idx, int e_idx) const {
        if (rooms[r_idx].capacity < events[e_idx].num_students) return false;
        return (events[e_idx].features & ~rooms[r_idx].features).none();
    }
    inline bool share_students(int e1, int e2) const { return event_conflict[e1].test(e2); }
};

// ============================================================================
// 5. MATCHING ALGORITHM
// ============================================================================
class BipartiteMatcher {
private:
    const UCTPInstance& instance;
    bool dfs_augment(int u, bool seen[], int matchR[]) const {
        for (int v = 0; v < instance.num_rooms; ++v) {
            if (instance.event_room_possible[u].test(v) && !seen[v]) {
                seen[v] = true;
                if (matchR[v] < 0 || dfs_augment(matchR[v], seen, matchR)) { matchR[v] = u; return true; }
            }
        }
        return false;
    }
public:
    BipartiteMatcher(const UCTPInstance& inst) : instance(inst) {}

    void match_timeslot(Individual& ind, int timeslot) const {
        if (ind.head[timeslot] == -1) return;

        int matchR[MAX_ROOMS];
        for (int i = 0; i < instance.num_rooms; ++i) matchR[i] = -1;
        
        int unplaced[MAX_EVENTS]; int unplaced_count = 0;

        int e = ind.head[timeslot];
        while(e != -1) {
            ind.room_id[e] = -1;
            bool seen[MAX_ROOMS] = {false};
            if (!dfs_augment(e, seen, matchR)) unplaced[unplaced_count++] = e;
            e = ind.next_evt[e];
        }

        int room_occupancy[MAX_ROOMS] = {0};
        for (int v = 0; v < instance.num_rooms; ++v) {
            if (matchR[v] != -1) { ind.room_id[matchR[v]] = v; room_occupancy[v]++; }
        }

        for (int i = 0; i < unplaced_count; ++i) {
            int u = unplaced[i]; int selected_room = -1, min_occ = 999999;
            for (int v = 0; v < instance.num_rooms; ++v) {
                if (instance.event_room_possible[u].test(v) && room_occupancy[v] < min_occ) {
                    min_occ = room_occupancy[v]; selected_room = v;
                }
            }
            if (selected_room != -1) { ind.room_id[u] = selected_room; room_occupancy[selected_room]++; }
        }
    }
    void match_all(Individual& ind) const { for (int t = 0; t < NUM_TIMESLOTS; ++t) match_timeslot(ind, t); }
};

// ============================================================================
// 6. FITNESS & DELTA EVALUATOR 
// ============================================================================
class Evaluator {
private:
    const UCTPInstance& instance;

    Fitness get_partial_fitness(const Individual& ind, const std::bitset<NUM_TIMESLOTS>& t_mask, const std::bitset<DAYS>& d_mask, const std::bitset<MAX_STUDENTS>& s_mask) const {
        Fitness fit;
        for (int t = 0; t < NUM_TIMESLOTS; ++t) {
            if (!t_mask.test(t)) continue;
            int e1 = ind.head[t];
            while(e1 != -1) {
                int r1 = ind.room_id[e1];
                if (r1 == -1 || !instance.can_room_host_event(r1, e1)) fit.hcv++; 
                
                int e2 = ind.next_evt[e1];
                while(e2 != -1) {
                    int r2 = ind.room_id[e2];
                    if (r1 != -1 && r1 == r2) fit.hcv++; 
                    if (instance.share_students(e1, e2)) fit.hcv++; 
                    e2 = ind.next_evt[e2];
                }
                
                if (t % SLOTS_PER_DAY == SLOTS_PER_DAY - 1) fit.scv += instance.events[e1].num_students;

                // ==========================================
                // ITC-2007 CHECKS MUST BE INSIDE THE e1 LOOP
                // ==========================================
                if (instance.is_itc2007) {
                    // 1. Timeslot Availability Check
                    if (!instance.event_timeslot_available[e1].test(t)) {
                        fit.hcv++;
                    }
                    
                }
                // ==========================================

                e1 = ind.next_evt[e1]; // Advance to the next event
            }
        }
        for (int s = 0; s < instance.num_students; ++s) {
            if (!s_mask.test(s)) continue; 
            for (int d = 0; d < DAYS; ++d) {
                if (!d_mask.test(d)) continue;
                int classes_today = 0, consecutive = 0;
                for (int h = 0; h < SLOTS_PER_DAY; ++h) {
                    int t = d * SLOTS_PER_DAY + h;
                    bool attends = false;
                    for (int e : instance.student_events[s]) {
                        if (ind.timeslot_id[e] == t) { attends = true; break; }
                    }
                    if (attends) { classes_today++; consecutive++; if (consecutive > 2) fit.scv++; } 
                    else { consecutive = 0; }
                }
                if (classes_today == 1) fit.scv++; 
            }
        }
        fit.score = fit.hcv > 0 ? ((fit.hcv * HARD_CONSTRAINT_PENALTY) + fit.scv) : fit.scv;
        return fit;
    }

public:
    Evaluator(const UCTPInstance& inst) : instance(inst) {}

    int get_scv_delta(const Individual& old_ind, const Individual& new_ind, const std::vector<int>& moved_events, const std::vector<int>& affected_slots) const {
        std::bitset<DAYS> d_mask;
        std::bitset<MAX_STUDENTS> s_mask;
        for(int t : affected_slots) d_mask.set(t/SLOTS_PER_DAY);
        for(int e : moved_events) {
            for(int s : instance.events[e].students) s_mask.set(s);
        }

        int scv_before = 0, scv_after = 0;
        
        for (int t : affected_slots) {
            if (t % SLOTS_PER_DAY == SLOTS_PER_DAY - 1) {
                int e = old_ind.head[t];
                while(e != -1) { scv_before += instance.events[e].num_students; e = old_ind.next_evt[e]; }
                e = new_ind.head[t];
                while(e != -1) { scv_after += instance.events[e].num_students; e = new_ind.next_evt[e]; }
            }
        }

        for (int s = 0; s < instance.num_students; ++s) {
            if (!s_mask.test(s)) continue; 
            for (int d = 0; d < DAYS; ++d) {
                if (!d_mask.test(d)) continue;
                int cls_b = 0, cons_b = 0;
                int cls_a = 0, cons_a = 0;
                for (int h = 0; h < SLOTS_PER_DAY; ++h) {
                    int t = d * SLOTS_PER_DAY + h;
                    bool att_b = false, att_a = false;
                    for (int ev : instance.student_events[s]) {
                        if (old_ind.timeslot_id[ev] == t) att_b = true;
                        if (new_ind.timeslot_id[ev] == t) att_a = true;
                        if(att_b && att_a) break;
                    }
                    if (att_b) { cls_b++; cons_b++; if (cons_b > 2) scv_before++; } else cons_b = 0;
                    if (att_a) { cls_a++; cons_a++; if (cons_a > 2) scv_after++; } else cons_a = 0;
                }
                if (cls_b == 1) scv_before++; 
                if (cls_a == 1) scv_after++;
            }
        }
        return scv_after - scv_before;
    }
    int count_precedence_hcv(const Individual& ind) const {
        if (!instance.is_itc2007) return 0;
        int hcv = 0;
        for (const auto& rule : instance.precedence_rules) {
            int t1 = ind.timeslot_id[rule.first];
            int t2 = ind.timeslot_id[rule.second];
            if (t1 != -1 && t2 != -1 && t1 >= t2) { // Violation
                hcv++;
            }
        }
        return hcv;
    }
    Fitness evaluate(const Individual& ind) const {
        Fitness fit;
        for (int t = 0; t < NUM_TIMESLOTS; ++t) {
            int e1 = ind.head[t];
            while (e1 != -1) {
                int r1 = ind.room_id[e1];
                if (r1 == -1 || !instance.can_room_host_event(r1, e1)) fit.hcv++;
                
                int e2 = ind.next_evt[e1];
                while (e2 != -1) {
                    if (r1 != -1 && r1 == ind.room_id[e2]) fit.hcv++;
                    if (instance.share_students(e1, e2)) fit.hcv++;
                    e2 = ind.next_evt[e2];
                }

                // ==========================================
                // ITC-2007 CHECKS MUST BE INSIDE THE e1 LOOP
                // ==========================================
                if (instance.is_itc2007) {
                    // 1. Timeslot Availability Check
                    if (!instance.event_timeslot_available[e1].test(t)) {
                        fit.hcv++;
                    }
                }
                // ==========================================

                e1 = ind.next_evt[e1]; // Advance to the next event
            }
        }
        for (int d = 0; d < DAYS; ++d) {
            int last_slot = (d * SLOTS_PER_DAY) + (SLOTS_PER_DAY - 1);
            int e = ind.head[last_slot];
            while(e != -1) { fit.scv += instance.events[e].num_students; e = ind.next_evt[e]; } 
        }

        std::vector<std::vector<bool>> student_schedule(instance.num_students, std::vector<bool>(NUM_TIMESLOTS, false));
        for (int e = 0; e < instance.num_events; ++e) {
            int t = ind.timeslot_id[e];
            if(t != -1) { for (int s : instance.events[e].students) student_schedule[s][t] = true; } 
            else { fit.hcv++; }
        }

        for (int s = 0; s < instance.num_students; ++s) {
            for (int d = 0; d < DAYS; ++d) {
                int classes_today = 0, consecutive = 0;
                for (int h = 0; h < SLOTS_PER_DAY; ++h) {
                    if (student_schedule[s][(d * SLOTS_PER_DAY) + h]) { classes_today++; consecutive++; if (consecutive > 2) fit.scv++; } 
                    else { consecutive = 0; }
                }
                if (classes_today == 1) fit.scv++; 
            }
        }
        fit.hcv += count_precedence_hcv(ind);
        fit.score = fit.hcv > 0 ? ((fit.hcv * HARD_CONSTRAINT_PENALTY) + fit.scv) : fit.scv;
        return fit;
    }

    void calculate_delta_N1(Fitness& fit, const Individual& old_ind, const Individual& new_ind, int e, int t1, int t2) const {
        std::bitset<NUM_TIMESLOTS> t_mask; std::bitset<DAYS> d_mask; std::bitset<MAX_STUDENTS> s_mask;
        t_mask.set(t1); t_mask.set(t2); d_mask.set(t1/SLOTS_PER_DAY); d_mask.set(t2/SLOTS_PER_DAY);
        for(int s : instance.events[e].students) s_mask.set(s);
        int prec_before = count_precedence_hcv(old_ind);
        int prec_after = count_precedence_hcv(new_ind);
        Fitness before = get_partial_fitness(old_ind, t_mask, d_mask, s_mask);
        Fitness after  = get_partial_fitness(new_ind, t_mask, d_mask, s_mask);
        fit.hcv += (after.hcv - before.hcv) + (prec_after - prec_before); 
        fit.scv += (after.scv - before.scv);
        fit.score = fit.is_feasible() ? fit.scv : ((fit.hcv * HARD_CONSTRAINT_PENALTY) + fit.scv);
    }

    void calculate_delta_N2(Fitness& fit, const Individual& old_ind, const Individual& new_ind, int e1, int e2, int t1, int t2) const {
        std::bitset<NUM_TIMESLOTS> t_mask; std::bitset<DAYS> d_mask; std::bitset<MAX_STUDENTS> s_mask;
        t_mask.set(t1); t_mask.set(t2); d_mask.set(t1/SLOTS_PER_DAY); d_mask.set(t2/SLOTS_PER_DAY);
        for(int s : instance.events[e1].students) s_mask.set(s);
        for(int s : instance.events[e2].students) s_mask.set(s);
        int prec_before = count_precedence_hcv(old_ind);
        int prec_after = count_precedence_hcv(new_ind);
        Fitness before = get_partial_fitness(old_ind, t_mask, d_mask, s_mask);
        Fitness after  = get_partial_fitness(new_ind, t_mask, d_mask, s_mask);

        fit.hcv += (after.hcv - before.hcv) + (prec_after - prec_before); 
        fit.scv += (after.scv - before.scv);
        fit.score = fit.is_feasible() ? fit.scv : ((fit.hcv * HARD_CONSTRAINT_PENALTY) + fit.scv);
    }

    void calculate_delta_N3(Fitness& fit, const Individual& old_ind, const Individual& new_ind, int e1, int e2, int e3, int t1, int t2, int t3) const {
        std::bitset<NUM_TIMESLOTS> t_mask; std::bitset<DAYS> d_mask; std::bitset<MAX_STUDENTS> s_mask;
        t_mask.set(t1); t_mask.set(t2); t_mask.set(t3);
        d_mask.set(t1/SLOTS_PER_DAY); d_mask.set(t2/SLOTS_PER_DAY); d_mask.set(t3/SLOTS_PER_DAY);
        for(int s : instance.events[e1].students) s_mask.set(s);
        for(int s : instance.events[e2].students) s_mask.set(s);
        for(int s : instance.events[e3].students) s_mask.set(s);
        int prec_before = count_precedence_hcv(old_ind);
        int prec_after = count_precedence_hcv(new_ind);
        Fitness before = get_partial_fitness(old_ind, t_mask, d_mask, s_mask);
        Fitness after  = get_partial_fitness(new_ind, t_mask, d_mask, s_mask);
        fit.hcv += (after.hcv - before.hcv) + (prec_after - prec_before); 
        fit.scv += (after.scv - before.scv);
        fit.score = fit.is_feasible() ? fit.scv : ((fit.hcv * HARD_CONSTRAINT_PENALTY) + fit.scv);
    }
};

// ============================================================================
// 7. MEMETIC ALGORITHM CORE
// ============================================================================
class MemeticAlgorithm {
private:
    long long C; long long time_limit_sec; const UCTPInstance& instance;
    double tp; unsigned int current_seed;
    std::mt19937 rng; BipartiteMatcher matcher; Evaluator evaluator;
    std::vector<Individual> population;
    long long smax;

    // STRICT DESCENT: Evaluates until improvement is found, strictly requiring delta < 0
    bool apply_N1(Individual& ind, int e_idx, bool target_scv) {
        int old_t = ind.timeslot_id[e_idx];

        for (int new_t = 0; new_t < NUM_TIMESLOTS; ++new_t) {
            if (new_t == old_t) continue;
            
            bool h1_conflict = false;
            int curr = ind.head[new_t];
            while(curr != -1) {
                if (instance.share_students(e_idx, curr)) { h1_conflict = true; break; }
                curr = ind.next_evt[curr];
            }
            if (h1_conflict) continue;

            Individual next = ind; 
            next.timeslot_id[e_idx] = new_t; next.update_slot(e_idx, old_t, new_t);
            
            if (target_scv) {
                int delta_scv = evaluator.get_scv_delta(ind, next, {e_idx}, {old_t, new_t});
                // MUST strictly improve to avoid infinite plateau loops
                if (delta_scv >= 0) continue; 
            }

            matcher.match_timeslot(next, old_t); matcher.match_timeslot(next, new_t);
            evaluator.calculate_delta_N1(next.fitness, ind, next, e_idx, old_t, new_t);
            
            if (target_scv && next.fitness.hcv > 0) continue; 
            if (!target_scv && next.fitness.hcv >= ind.fitness.hcv) continue; 

            ind = next; return true;
        }
        return false;
    }

    bool apply_N2(Individual& ind, int e1_idx, bool target_scv) {
        int t1 = ind.timeslot_id[e1_idx];

        for (int e2_idx = 0; e2_idx < instance.num_events; ++e2_idx) {
            int t2 = ind.timeslot_id[e2_idx];
            if (t1 == t2) continue;


            bool h1_conflict = false;
            int curr = ind.head[t2];
            while(curr != -1) {
                if (curr != e2_idx && instance.share_students(e1_idx, curr)) { h1_conflict = true; break; }
                curr = ind.next_evt[curr];
            }
            if (h1_conflict) continue;
            curr = ind.head[t1];
            while(curr != -1) {
                if (curr != e1_idx && instance.share_students(e2_idx, curr)) { h1_conflict = true; break; }
                curr = ind.next_evt[curr];
            }
            if (h1_conflict) continue;


            Individual next = ind; std::swap(next.timeslot_id[e1_idx], next.timeslot_id[e2_idx]);
            next.update_slot(e1_idx, t1, t2); next.update_slot(e2_idx, t2, t1);
            
            if (target_scv) {
                int delta_scv = evaluator.get_scv_delta(ind, next, {e1_idx, e2_idx}, {t1, t2});
                if (delta_scv >= 0) continue; // STRICT IMPROVEMENT
            }

            matcher.match_timeslot(next, t1); matcher.match_timeslot(next, t2);
            evaluator.calculate_delta_N2(next.fitness, ind, next, e1_idx, e2_idx, t1, t2);
            
            if (target_scv && next.fitness.hcv > 0) continue; 
            if (!target_scv && next.fitness.hcv >= ind.fitness.hcv) continue;

            ind = next; return true;
        }
        return false;
    }

    bool apply_N3(Individual& ind, int e1_idx, bool target_scv) {
        int t1 = ind.timeslot_id[e1_idx];
        
        for(int attempt = 0; attempt < 50; ++attempt) {
            std::uniform_int_distribution<> ev_dis(0, instance.num_events - 1);
            int e2_idx = ev_dis(rng); int t2 = ind.timeslot_id[e2_idx]; if (t1 == t2) continue;
            int e3_idx = ev_dis(rng); int t3 = ind.timeslot_id[e3_idx]; if (t3 == t1 || t3 == t2) continue;

            if (target_scv) {
                bool h1_conflict = false;
                int curr = ind.head[t2];
                while(curr != -1) { if (curr != e2_idx && instance.share_students(e1_idx, curr)) { h1_conflict = true; break; } curr = ind.next_evt[curr]; }
                if (h1_conflict) continue;
                
                curr = ind.head[t3];
                while(curr != -1) { if (curr != e3_idx && instance.share_students(e2_idx, curr)) { h1_conflict = true; break; } curr = ind.next_evt[curr]; }
                if (h1_conflict) continue;
                
                curr = ind.head[t1];
                while(curr != -1) { if (curr != e1_idx && instance.share_students(e3_idx, curr)) { h1_conflict = true; break; } curr = ind.next_evt[curr]; }
                if (h1_conflict) continue;
            }

            Individual next = ind;
            next.timeslot_id[e1_idx] = t2; next.timeslot_id[e2_idx] = t3; next.timeslot_id[e3_idx] = t1;
            next.update_slot(e1_idx, t1, t2); next.update_slot(e2_idx, t2, t3); next.update_slot(e3_idx, t3, t1);
            
            if (target_scv) {
                int delta_scv = evaluator.get_scv_delta(ind, next, {e1_idx, e2_idx, e3_idx}, {t1, t2, t3});
                if (delta_scv >= 0) continue; // STRICT IMPROVEMENT
            }

            matcher.match_timeslot(next, t1); matcher.match_timeslot(next, t2); matcher.match_timeslot(next, t3);
            evaluator.calculate_delta_N3(next.fitness, ind, next, e1_idx, e2_idx, e3_idx, t1, t2, t3);
            
            if (target_scv && next.fitness.hcv > 0) continue;
            if (!target_scv && next.fitness.hcv >= ind.fitness.hcv) continue;

            ind = next; return true;
        }
        return false;
    }

    void LS1(Individual& ind) {
        int ev_order[MAX_EVENTS];
        std::iota(std::begin(ev_order), std::begin(ev_order) + instance.num_events, 0);

        // Phase 1: Hard Constraints strict descent (with safety bound against infinite numerical loops)
        bool improved = true;
        int safety_hcv = 0;
        while (improved && !ind.fitness.is_feasible() && safety_hcv++ < smax) {
            improved = false;
            std::shuffle(std::begin(ev_order), std::begin(ev_order) + instance.num_events, rng);
            for (int i = 0; i < instance.num_events; ++i) {
                int e = ev_order[i];
                if (apply_N1(ind, e, false)) { improved = true; continue; }
                if (apply_N2(ind, e, false)) { improved = true; continue; }
            }
        }

        // Phase 2: Soft Constraints strict descent
        if (ind.fitness.is_feasible()) {
            improved = true;
            int safety_scv = 0;
            while (improved && ind.fitness.scv > 0 && safety_scv++ < smax) {
                improved = false;
                std::shuffle(std::begin(ev_order), std::begin(ev_order) + instance.num_events, rng);
                for (int i = 0; i < instance.num_events; ++i) {
                    int e = ev_order[i];
                    if (apply_N1(ind, e, true)) { improved = true; continue; }
                    if (apply_N2(ind, e, true)) { improved = true; continue; }
                    if (apply_N3(ind, e, true)) { improved = true; continue; }
                }
            }
        }
    }

    void LS2(Individual& ind) {
        if (!ind.fitness.is_feasible()) return; 
        
        int worst_t = -1, max_penalty = -1;
        for (int t = 0; t < NUM_TIMESLOTS; ++t) {
            int count = 0, e = ind.head[t];
            while(e != -1) { count++; e = ind.next_evt[e]; }
            if (count > max_penalty) { max_penalty = count; worst_t = t; }
        }
        if (worst_t == -1) return;
        
        int ev = ind.head[worst_t];
        while(ev != -1) {
            int nxt = ind.next_evt[ev]; 
            apply_N1(ind, ev, true); 
            ev = nxt;
        }
    }

    Individual uniform_crossover(const Individual& p1, const Individual& p2) {
        Individual child; std::uniform_real_distribution<> dis(0.0, 1.0);
        for (int i = 0; i < instance.num_events; ++i) {
            int t = (dis(rng) < 0.5) ? p1.timeslot_id[i] : p2.timeslot_id[i];
            child.timeslot_id[i] = t; child.update_slot(i, -1, t);
        }
        matcher.match_all(child); child.fitness = evaluator.evaluate(child); return child;
    }

    // THE FIX: Randomly utilize N1, N2, or N3 to ensure destructive structural mutation!
    void mutate(Individual& ind) {
        std::uniform_int_distribution<> ev_dis(0, instance.num_events - 1);
        std::uniform_int_distribution<> t_dis(0, NUM_TIMESLOTS - 1);
        
        int move_type = std::uniform_int_distribution<>(1, 3)(rng);
        
        if (move_type == 1) {
            int e = ev_dis(rng), old_t = ind.timeslot_id[e], t = t_dis(rng);
            if (old_t != t) {
                ind.timeslot_id[e] = t; ind.update_slot(e, old_t, t);
                matcher.match_timeslot(ind, old_t); matcher.match_timeslot(ind, t);
            }
        } else if (move_type == 2) {
            int e1 = ev_dis(rng), e2 = ev_dis(rng);
            int t1 = ind.timeslot_id[e1], t2 = ind.timeslot_id[e2];
            if (t1 != t2) {
                std::swap(ind.timeslot_id[e1], ind.timeslot_id[e2]);
                ind.update_slot(e1, t1, t2); ind.update_slot(e2, t2, t1);
                matcher.match_timeslot(ind, t1); matcher.match_timeslot(ind, t2);
            }
        } else {
            int e1 = ev_dis(rng), e2 = ev_dis(rng), e3 = ev_dis(rng);
            int t1 = ind.timeslot_id[e1], t2 = ind.timeslot_id[e2], t3 = ind.timeslot_id[e3];
            if (t1 != t2 && t2 != t3 && t1 != t3) {
                ind.timeslot_id[e1] = t2; ind.timeslot_id[e2] = t3; ind.timeslot_id[e3] = t1;
                ind.update_slot(e1, t1, t2); ind.update_slot(e2, t2, t3); ind.update_slot(e3, t3, t1);
                matcher.match_timeslot(ind, t1); matcher.match_timeslot(ind, t2); matcher.match_timeslot(ind, t3);
            }
        }
        ind.fitness = evaluator.evaluate(ind);
    }

public:
    MemeticAlgorithm(long long c, long long time, const UCTPInstance& inst, double prob, unsigned int seed, long long smax_val)
        : C(c), time_limit_sec(time), instance(inst), tp(prob), current_seed(seed), rng(seed), matcher(inst), evaluator(inst), smax(smax_val) {}

    bool is_duplicate(const Individual& child) const {
        for (const auto& ind : population) {
            if (ind.fitness.score == child.fitness.score) {
                bool identical = true;
                for(int e=0; e<instance.num_events; ++e) {
                    if (ind.timeslot_id[e] != child.timeslot_id[e]) { identical = false; break; }
                }
                if (identical) return true;
            }
        }
        return false;
    }
    
    std::tuple<Individual, long long, std::vector<long long>> run() {
        int pop_size = 50; population.clear();
        std::uniform_int_distribution<> t_dis(0, NUM_TIMESLOTS - 1);
        
        for (int i = 0; i < pop_size; ++i) {
            Individual ind;
            for (int e = 0; e < instance.num_events; ++e) {
                int t = t_dis(rng); ind.timeslot_id[e] = t; ind.update_slot(e, -1, t);
            }
            matcher.match_all(ind); ind.fitness = evaluator.evaluate(ind);
            LS1(ind); LS2(ind); ind.fitness = evaluator.evaluate(ind);
            population.push_back(ind);
        }
        std::sort(population.begin(), population.end());
        auto start_time = std::chrono::steady_clock::now(); long long iterations = 0;

        long long best_score_ever = population[0].fitness.score;
        int stagnant_generations = 0;

        auto tournament_selection = [&]() {
            std::uniform_int_distribution<> pop_dis(0, pop_size - 1);
            int best_idx = pop_dis(rng);
            for(int i=0; i<3; ++i) {
                int idx = pop_dis(rng);
                if (population[idx].fitness < population[best_idx].fitness) best_idx = idx;
            }
            return best_idx;
        };
        
        std::vector<long long> fitness_history;

        while (true) {
            if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time).count() >= time_limit_sec) break;
            
            fitness_history.push_back(population[0].fitness.score);
            
            if (population[0].fitness.score < best_score_ever) {
                best_score_ever = population[0].fitness.score;
                stagnant_generations = 0;
            } else {
                stagnant_generations++;
            }

            if (stagnant_generations > 500) {
                for (int i = 5; i < pop_size; ++i) {
                    Individual fresh;
                    for (int e = 0; e < instance.num_events; ++e) {
                        int t = t_dis(rng); fresh.timeslot_id[e] = t; fresh.update_slot(e, -1, t);
                    }
                    matcher.match_all(fresh); fresh.fitness = evaluator.evaluate(fresh);
                    LS1(fresh); LS2(fresh); fresh.fitness = evaluator.evaluate(fresh);
                    population[i] = fresh;
                }
                std::sort(population.begin(), population.end());
                stagnant_generations = 0;
            }

            ++iterations;
            if(iterations % 500 == 0){
                g_logger.log(LogLevel::INFO, "Generation " + std::to_string(iterations) + 
                             " | Best HCV: " + std::to_string(population[0].fitness.hcv) + 
                             " | Best SCV: " + std::to_string(population[0].fitness.scv));
            }

            int p1 = tournament_selection();
            int p2 = tournament_selection();

            std::uniform_real_distribution<> prob(0.0, 1.0);
            Individual child = population[p1];
            if (prob(rng) < 0.8) child = uniform_crossover(population[p1], population[p2]);
            if (prob(rng) < 0.5) mutate(child);
            
            LS1(child); LS2(child); child.fitness = evaluator.evaluate(child);

            if (child.fitness < population.back().fitness && !is_duplicate(child)) {
                population.back() = child; std::sort(population.begin(), population.end());
            }
        }
        return {population.front(), iterations, fitness_history}; 
    }
};

// ============================================================================
// 8. FILE I/O AND ENTRY POINT
// ============================================================================
void loadInstance(const std::string& filename, UCTPInstance& instance) {
    std::ifstream file(filename);
    if (!file.is_open()) return;

    int num_features = 0;
    file >> instance.num_events >> instance.num_rooms >> num_features >> instance.num_students;
    instance.num_features = num_features;
    
    instance.events.resize(instance.num_events);
    for (int i = 0; i < instance.num_events; ++i) { instance.events[i].id = i; instance.events[i].num_students = 0; }
    instance.rooms.resize(instance.num_rooms);
    for (int i = 0; i < instance.num_rooms; ++i) instance.rooms[i].id = i;
    for (int r = 0; r < instance.num_rooms; ++r) file >> instance.rooms[r].capacity;

    instance.student_events.resize(instance.num_students);
    for (int s = 0; s < instance.num_students; ++s) {
        for (int e = 0; e < instance.num_events; ++e) {
            int attends; file >> attends;
            if (attends == 1) { instance.student_events[s].push_back(e); instance.events[e].students.push_back(s); instance.events[e].num_students++; }
        }
    }

    for (int r = 0; r < instance.num_rooms; ++r) {
        for (int f = 0; f < num_features; ++f) {
            int has_feature; file >> has_feature; if (has_feature == 1) instance.rooms[r].features.set(f);
        }
    }

    for (int e = 0; e < instance.num_events; ++e) {
        for (int f = 0; f < num_features; ++f) {
            int req_feature; file >> req_feature; if (req_feature == 1) instance.events[e].features.set(f);
        }
    }
    // ========================================================================
    // ITC-2007 EXTENSION: Detect EOF and parse additional matrices if present
    // ========================================================================

    instance.is_itc2007 = false;
    instance.event_timeslot_available.assign(instance.num_events, std::bitset<NUM_TIMESLOTS>().set()); // All 1s

    int peek_val;
    if (file >> peek_val) {
        instance.is_itc2007 = true;
        g_logger.log(LogLevel::INFO, "Detected additional matrices (ITC-2007 format).");

        // --- Parse Timeslot Availability ---
        for (int e = 0; e < instance.num_events; ++e) {
            for (int t = 0; t < NUM_TIMESLOTS; ++t) {
                int val = (e == 0 && t == 0) ? peek_val : 0;
                if (!(e == 0 && t == 0)) file >> val;
                
                if (val == 0) {
                    instance.event_timeslot_available[e].reset(t);
                }
            }
        }

        // --- Parse Precedence Matrix ---
        for (int e1 = 0; e1 < instance.num_events; ++e1) {
            for (int e2 = 0; e2 < instance.num_events; ++e2) {
                int val; file >> val;
                if (val == 1) { // 1 means e1 must be strictly BEFORE e2
                    instance.precedence_rules.push_back({e1, e2});
                }
            }
        }
    } else {
        g_logger.log(LogLevel::INFO, "No additional matrices detected (Standard format).");
    }
    file.close();

    // ========================================================================
    // Post-Processing
    // ========================================================================
    
    instance.event_conflict.resize(instance.num_events);
    for (int s = 0; s < instance.num_students; ++s) {
        const auto& student_classes = instance.student_events[s];
        for (size_t i = 0; i < student_classes.size(); ++i) {
            for (size_t j = i + 1; j < student_classes.size(); ++j) {
                instance.event_conflict[student_classes[i]].set(student_classes[j]);
                instance.event_conflict[student_classes[j]].set(student_classes[i]);
            }
        }
    }

    instance.event_room_possible.resize(instance.num_events);
    for (int e = 0; e < instance.num_events; ++e) {
        for (int r = 0; r < instance.num_rooms; ++r) {
            if (instance.rooms[r].capacity >= instance.events[e].num_students && (instance.events[e].features & ~instance.rooms[r].features).none()) {
                instance.event_room_possible[e].set(r);
            }
        }
    }
}

// ============================================================================
// 9. JSON EXPORT UTILITIES
// ============================================================================

void saveTimetableJSON(const Individual& ind, const std::string& filename, unsigned int seed, long long generations, const UCTPInstance& inst, const std::vector<long long>& history) {
    std::ofstream out(filename);
    if (!out.is_open()) return;
    
    out << "{\n  \"seed\": " << seed << ",\n  \"generations_passed\": " << generations << ",\n  \"fitness\": {\n    \"score\": " << ind.fitness.score << ",\n    \"hcv\": " << ind.fitness.hcv << ",\n    \"scv\": " << ind.fitness.scv << "\n  },\n";
    
    out << "  \"convergence_history\": [";
    for (size_t i = 0; i < history.size(); ++i) {
        out << history[i] << (i < history.size() - 1 ? "," : "");
    }
    out << "],\n";

    out << "  \"schedule\": [\n";
    for (size_t i = 0; i < inst.num_events; ++i) {
        out << "    {\"event_id\": " << i << ", \"room_id\": " << ind.room_id[i] << ", \"timeslot_id\": " << ind.timeslot_id[i] << "}";
        if (i < inst.num_events - 1) out << ","; 
        out << "\n";
    }
    out << "  ]\n}\n"; out.close();
}

struct StatResult { long long best, worst; double mean, stdev; };
StatResult calcStats(const std::vector<long long>& data) {
    long long sum = std::accumulate(data.begin(), data.end(), 0LL); double mean = static_cast<double>(sum) / data.size();
    double sq_sum = std::inner_product(data.begin(), data.end(), data.begin(), 0.0);
    double stdev = std::sqrt(std::max(0.0, sq_sum / data.size() - mean * mean));
    auto minmax = std::minmax_element(data.begin(), data.end()); return {*minmax.first, *minmax.second, mean, stdev};
}

void printStatistics(const std::string& label, const std::vector<long long>& data) {
    StatResult stats = calcStats(data);
    std::cout << label << " -> Best: " << stats.best << " | Worst: " << stats.worst << " | Mean: " << stats.mean << " | StdDev: " << stats.stdev << "\n";
}

void saveSummaryJSON(const std::string& filename, const std::string& dataset, const UCTPInstance& inst, long long time_limit, double tp, int runs, const std::vector<long long>& fit, const std::vector<long long>& hcv, const std::vector<long long>& scv, const std::vector<long long>& gens) {
    std::ofstream out(filename); if (!out.is_open()) return;
    StatResult fit_stats = calcStats(fit), hcv_stats = calcStats(hcv), scv_stats = calcStats(scv), gen_stats = calcStats(gens);
    out << "{\n  \"configuration\": {\n    \"dataset\": \"" << dataset << "\",\n    \"events\": " << inst.num_events << ",\n    \"rooms\": " << inst.num_rooms << ",\n    \"students\": " << inst.num_students << ",\n    \"timeslots\": " << NUM_TIMESLOTS << ",\n    \"time_limit_sec\": " << time_limit << ",\n    \"tp_percentage\": " << tp << ",\n    \"independent_runs\": " << runs << "\n  },\n  \"statistical_results\": {\n";
    auto writeStat = [&out](const std::string& key, const StatResult& s, bool last) {
        out << "    \"" << key << "\": {\n      \"best\": " << s.best << ",\n      \"worst\": " << s.worst << ",\n      \"mean\": " << s.mean << ",\n      \"stddev\": " << s.stdev << "\n    }" << (last ? "" : ",") << "\n";
    };
    writeStat("overall_fitness", fit_stats, false); writeStat("hard_constraint_violations", hcv_stats, false);
    writeStat("soft_constraint_violations", scv_stats, false); writeStat("generations_passed", gen_stats, true);
    out << "  }\n}\n"; out.close();
}

std::string getTimestampString() {
    auto now = std::chrono::system_clock::now(); std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&now_time); std::ostringstream oss; oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S"); return oss.str();
}

int main(int argc, char* argv[]) {
    std::string filename = "";
    std::string base_folder = "timetables/" + getTimestampString();
    bool debug_logging = false; 

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--debug") debug_logging = true;
        else if (arg == "--input" && i + 1 < argc) filename = argv[++i];
        else if (arg == "--output" && i + 1 < argc) base_folder = argv[++i];
        else if (filename.empty() && arg[0] != '-') filename = arg;
    }

    if (filename.empty()) { std::cerr << "Usage: ./uctp_solver --input <instance_file.dat> [--output <dir>]" << std::endl; return 1; }
    
    g_logger.setDebug(debug_logging);
    
    UCTPInstance instance; loadInstance(filename, instance);

    long long time_limit_sec = 90; 
    long long C = HARD_CONSTRAINT_PENALTY; double tp = 0.2; 
    int num_independent_runs = 1; 
    long long smax = 200;

    if(instance.num_events >= 300 && instance.num_students >= 200 && instance.num_rooms >= 10 && instance.num_students < 400 && instance.num_features < 6) {
        time_limit_sec = 900; num_independent_runs = 1; smax = 1000;
    } else if (instance.num_events >= 400 && instance.num_students >= 200 && instance.num_rooms >= 10) {
        time_limit_sec = 9000; num_independent_runs = 1; smax = 2000;
    }

    std::cout << "Configured settings\n  dataset: " << filename << "\n  time limit (sec): " << time_limit_sec << "\n  independent runs: " << num_independent_runs << "\n  max iterations (smax): " << smax << "\n\n";

    std::filesystem::create_directories(base_folder);
    std::vector<long long> fitness_results, hcv_results, scv_results, generation_results;
    
    std::cout << "Starting " << num_independent_runs << " independent runs...\n";

    std::vector<std::future<std::tuple<Individual, long long, std::vector<long long>>>> futures;
    std::vector<unsigned int> seeds(num_independent_runs);

    for (int run = 0; run < num_independent_runs; ++run) {
        seeds[run] = std::random_device{}();
        
        futures.push_back(std::async(std::launch::async, [run, C, time_limit_sec, &instance, tp, seed = seeds[run], smax]() {
            MemeticAlgorithm ma(C, time_limit_sec, instance, tp, seed, smax);
            return ma.run();
        }));
    }

    for (int run = 0; run < num_independent_runs; ++run) {
        auto [run_best, total_generations, fitness_history] = futures[run].get();
        unsigned int seed = seeds[run];

        std::string json_file = base_folder + "/run_" + std::to_string(run + 1) + "_timetable.json";
        saveTimetableJSON(run_best, json_file, seed, total_generations, instance, fitness_history);

        fitness_results.push_back(run_best.fitness.score);
        hcv_results.push_back(run_best.fitness.hcv);
        scv_results.push_back(run_best.fitness.scv);
        generation_results.push_back(total_generations);

        std::cout << "Run " << (run + 1) << " Finished! -> HCV: " << run_best.fitness.hcv 
                  << " | SCV: " << run_best.fitness.scv 
                  << " | Gens: " << total_generations << "\n";
    }

    std::cout << "\n==================================================\n=== FINAL STATISTICAL RESULTS ACROSS " << num_independent_runs << " RUNS ===\n==================================================\n";
    printStatistics("Hard Constraint Violations (HCV)", hcv_results); printStatistics("Soft Constraint Violations (SCV)", scv_results);
    printStatistics("Overall Fitness", fitness_results); printStatistics("Generations Passed", generation_results);
    std::string summary_file = base_folder + "/final_summary.json";
    saveSummaryJSON(summary_file, filename, instance, time_limit_sec, tp, num_independent_runs, fitness_results, hcv_results, scv_results, generation_results);
    std::cout << "\nAll settings and statistical results exported to: " << summary_file << "\n";
    return 0;
}