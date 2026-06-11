#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>
#include <cstdint>

class Random {
public:
    static std::mt19937& engine() {
        static thread_local std::mt19937 rng(std::random_device{}());
        return rng;
    }

    static void seed(uint32_t s) {
        engine().seed(s);
    }

    static int nextInt(int low, int high) {
        std::uniform_int_distribution<int> d(low, high);
        return d(engine());
    }

    static double nextReal() {
        std::uniform_real_distribution<double> d(0.0, 1.0);
        return d(engine());
    }

    static double nextReal(double low, double high) {
        std::uniform_real_distribution<double> d(low, high);
        return d(engine());
    }
};

#endif
