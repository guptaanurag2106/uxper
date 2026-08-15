#include <emmintrin.h>
#include <pthread.h>
#include <sched.h>
#include <sys/types.h>
#include <x86intrin.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <random>
#include <unordered_map>

bool check_password(const std::string &pass) {
    static constexpr std::string_view SECRET_PASSWORD = "B7BR5*8WAj4PT@2k";

    if (pass.size() != SECRET_PASSWORD.size()) return false;

    size_t i = 0;
    while (i < SECRET_PASSWORD.size()) {
        if (pass[i] != SECRET_PASSWORD[i]) return false;

        i++;
    }
    return true;
}

void randomize(std::string &possibilities) {
    std::shuffle(possibilities.begin(), possibilities.end(),
                 std::default_random_engine(0));
}

constexpr unsigned int core_id = 1;

uint64_t start_tsc() {
    _mm_lfence();
    return __rdtsc();
}

inline void do_no_optimize_please(bool val) {
    asm volatile("" : : "r,m"(val) : "memory");
}

uint64_t end_tsc() {
    unsigned int id;
    // invariant_tsc flag, so even if id changes tsc read should be
    // constant, though cache warmup ig?
    uint64_t end = __rdtscp(&id);
    assert(id == core_id && "cpu affinity didn't work?");
    _mm_lfence();
    return end;
}

int main() {
    std::string possibilities =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&"
        "*";

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    int status = sched_setaffinity(0, sizeof(cpuset), &cpuset);
    if (status != 0) {
        perror("cannot set sched_setaffinity\n");
    }

    constexpr size_t MAX_LEN = 20;
    std::unordered_map<size_t, uint64_t> len_times;
    for (int i = 0; i < 50000; i++) {
        for (size_t l = 1; l <= MAX_LEN; l++) {
            std::string pass(l, 'a');
            uint64_t start = start_tsc();
            bool res = check_password(pass);
            do_no_optimize_please(res);
            uint64_t end = end_tsc();
            len_times[l] += end - start;
        }
    }

    auto max_entry = std::max_element(
        len_times.begin(), len_times.end(),
        [](const auto &p1, const auto &p2) { return p1.second < p2.second; });
    size_t estimated_len = max_entry->first;
    std::cout << "Estimated length " << estimated_len << std::endl;

    // estimated_len = 16;

    size_t iter = 0;
    bool found = false;
    std::string pass(estimated_len, 'a');

    while (iter < estimated_len) {
        // warmup + checking average to reject outlier?
        std::unordered_map<char, uint64_t> init_times(possibilities.size());
        constexpr int WARMUP_COUNT = 200;
        for (int i = 0; i < WARMUP_COUNT; i++) {
            randomize(possibilities);
            for (char c : possibilities) {
                pass[iter] = c;

                uint64_t start = start_tsc();
                if (check_password(pass)) {
                    found = true;
                    break;
                }
                uint64_t end = end_tsc();
                init_times[c] += end - start;
            }
            if (found) break;
        }
        if (found) break;

        for (char c : possibilities) {
            init_times[c] /=
                WARMUP_COUNT;  // should probably take median not mean
        }

        std::unordered_map<char, uint64_t> times(possibilities.size());
        constexpr int RUN_COUNT = 100000;
        for (int i = 0; i < RUN_COUNT; i++) {
            // shuffle POSSIBILITIES for branch prediction?
            randomize(possibilities);
            for (char c : possibilities) {
                pass[iter] = c;

                uint64_t start = start_tsc();
                bool res = check_password(pass);
                do_no_optimize_please(res);

                uint64_t end = end_tsc();
                uint64_t elapsed = end - start;

                // is this needed? can it be faster than before?
                // what if spike during warmup??
                // elapsed = std::max(elapsed, uint64_t(init_times[c] * 0.8));
                elapsed = std::min(elapsed, uint64_t(init_times[c] * 1.20));
                times[c] += elapsed;
            }
        }

        auto max_entry = std::max_element(times.begin(), times.end(),
                                          [](const auto &p1, const auto &p2) {
                                              return p1.second < p2.second;
                                          });

        pass[iter] = max_entry->first;
        std::cout << pass << std::endl;

        iter++;
    }

    if (check_password(pass)) {
        std::cout << "password is '" << pass << "'" << std::endl;
    } else {
        std::cout << "womp womp, didn't work. Last attempt '" << pass << "'"
                  << std::endl;
    }

    return 0;
}
