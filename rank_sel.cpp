#include <cstdint>
#include <sdsl/bit_vectors.hpp>
#include <iostream>
#include <chrono>
#include <algorithm>

using namespace std;
using namespace sdsl;

template <class T>
inline void black_box(T&& datum) {
    asm volatile("" : "+r"(datum));
}

uint64_t xor_shift64(uint64_t& seed) {
    seed ^= seed << 13;
    seed ^= seed >> 7;
    seed ^= seed << 17;
    return seed;
}

void print_ns(const char* label, double total_time, uint64_t runs = 1) {
    cout << "time / " << label << " [ns]: " << total_time / runs << endl;
}

template<class T>
void print_space_overhead(const char* label, const bit_vector& b, const T& sth) {
    cout<< label << " space overhead: " << size_in_bytes(sth) * 100 / size_in_bytes(b) << '%' << endl;
}

template<class F>
inline double measure(F f) {   // warm + measure; result is in ns
    unsigned iters = 1;
    auto start = chrono::steady_clock::now();
    while (chrono::steady_clock::now() - start < 3s) {
        f();
        iters += 1;
    }
    start = chrono::steady_clock::now();
    for (auto i = 0; i < iters; ++i) f();
    return static_cast<double>(chrono::duration_cast<chrono::nanoseconds>(chrono::steady_clock::now() - start).count()) / iters;
}

template<class rs>
void benchmark_rank(const char* label, const bit_vector& b, const rs& rb, uint64_t universe) {
    print_space_overhead(label, b, rb);
    auto step_by = max(universe / 1000000, uint64_t(1));
    double ns = measure([&] {
        for (uint64_t index = 0; index < universe; index += step_by) {
            black_box(rb(index));
        }
    });
    print_ns(label, ns, universe / step_by);
}

template<class ss>
void benchmark_select(const char* label, const bit_vector& b, const ss& sb, uint64_t num) {
    print_space_overhead(label, b, sb);
    auto step_by = max(num / 1000000, uint64_t(1));
    double ns = measure([&] {
        for (uint64_t index = 1; index <= num; index += step_by) {
            black_box(sb(index));
        }
    });
    print_ns(label, ns, num / step_by);
}

int main(int argc, char *argv[]) 
{
    uint64_t universe = 1024 * 1024 * 1024;
    uint64_t num = universe / 2;

    bit_vector b(universe, 0);
    uint64_t xs_seed = 1234;
    auto to_insert = num;
    while (to_insert > 0) {
        auto bit_nr = xor_shift64(xs_seed) % universe;
        if (!b[bit_nr]) {
            b[bit_nr] = true;
            to_insert--;
        }
    }

    benchmark_rank("rank_v", b, rank_support_v<>(&b), universe);
    benchmark_rank("rank_v5", b, rank_support_v5<>(&b), universe);
    benchmark_select("select1 mcl", b, select_support_mcl<1>(&b), num);
    benchmark_select("select0 mcl", b, select_support_mcl<0>(&b), universe-num);
}