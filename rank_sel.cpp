#include <cstddef>
#include <cstdint>
#include <sdsl/bit_vectors.hpp>
#include <iostream>
#include <chrono>

using namespace std;
using namespace sdsl;

const std::uint64_t SEED = 1234;
const std::size_t QUERIES = 1000000;

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

void print_ns(const char* label, double time) {
    cout << "time / " << label << " [ns]: " << time << endl;
}

template<class T>
void print_space_overhead(const char* label, const bit_vector& b, const T& sth) {
    cout<< label << " space overhead: " << size_in_bytes(sth) * 100 / size_in_bytes(b) << '%' << endl;
}

std::vector<std::size_t> rand_queries(std::size_t query_universe) {
    std::vector<std::size_t> result;
    result.reserve(QUERIES);
    auto seed = SEED;
    for (std::size_t i = 0; i < QUERIES; ++i) {
        result.push_back(xor_shift64(seed) % query_universe);
    }
    return result;
}

template<class F>
inline double measure(F f) {   // warm + measure; result is in ns
    unsigned iters = 1;
    auto start = chrono::steady_clock::now();
    while (chrono::steady_clock::now() - start < 5s) {
        f();
        iters += 1;
    }
    start = chrono::steady_clock::now();
    for (auto i = 0; i < iters; ++i) f();
    return static_cast<double>(chrono::duration_cast<chrono::nanoseconds>(chrono::steady_clock::now() - start).count()) / iters;
}

template<class rs>
void benchmark_rank(const char* label, const bit_vector& b, const rs& rb, const std::vector<std::size_t>& queries) {
    print_space_overhead(label, b, rb);
    double ns = measure([&] {
        for (auto index: queries) black_box(rb(index));
    });
    print_ns(label, ns / queries.size());
}

template<class ss>
void benchmark_select(const char* label, const bit_vector& b, const ss& sb, const std::vector<std::size_t>& queries) {
    print_space_overhead(label, b, sb);
    double ns = measure([&] {
        for (auto index: queries) black_box(sb(index));
    });
    print_ns(label, ns / queries.size());
}

int main(int argc, char *argv[]) 
{
    uint64_t universe = 10000000000;
    uint64_t num = universe / 10;
    std::cout << "The bit vector consists of " << universe << " bits, including " << num << " ones." << std::endl;

    bit_vector b(universe, 0);
    uint64_t xs_seed = SEED;
    auto to_insert = num;
    while (to_insert > 0) {
        auto bit_nr = xor_shift64(xs_seed) % universe;
        if (!b[bit_nr]) {
            b[bit_nr] = true;
            to_insert--;
        }
    }

    {
        std::vector<std::size_t> rank_queries = rand_queries(universe);
        benchmark_rank("rank_v", b, rank_support_v<>(&b), rank_queries);
        benchmark_rank("rank_v5", b, rank_support_v5<>(&b), rank_queries);
    }
    benchmark_select("select1 mcl", b, select_support_mcl<1>(&b), rand_queries(num));
    benchmark_select("select0 mcl", b, select_support_mcl<0>(&b), rand_queries(universe-num));
}