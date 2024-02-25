#include <cstddef>
#include <cstdint>
#include <ostream>
#include <sdsl/bit_vectors.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <string>

using namespace std;
using namespace sdsl;

std::uint64_t SEED = 1234;
std::size_t QUERIES = 1000000;
chrono::seconds TIME_PER_TEST = 5s;

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

template<class T>
void print_result(const char* label, const bit_vector& b, const T& sth, double time) {
    cout<< label << ":  space overhead " << size_in_bytes(sth) * 100.0 / size_in_bytes(b) << "\%  time/query " << time << "ns" << endl;
}

std::vector<std::size_t> rand_queries(std::size_t query_universe, uint64_t from = 0) {
    std::vector<std::size_t> result;
    result.reserve(QUERIES);
    auto seed = SEED;
    for (std::size_t i = 0; i < QUERIES; ++i) {
        result.push_back(xor_shift64(seed) % query_universe + from);
    }
    return result;
}

template<class F>
inline double measure(F f) {   // warm + measure; result is in ns
    std::this_thread::sleep_for(TIME_PER_TEST / 3);
    unsigned iters = 1;
    auto start = chrono::steady_clock::now();
    while (chrono::steady_clock::now() - start < TIME_PER_TEST) {
        f();
        iters += 1;
    }
    start = chrono::steady_clock::now();
    for (auto i = 0; i < iters; ++i) f();
    return static_cast<double>(chrono::duration_cast<chrono::nanoseconds>(chrono::steady_clock::now() - start).count()) / iters;
}

template<class rs>
void benchmark_rank(const char* label, const bit_vector& b, const rs& rb, const std::vector<std::size_t>& queries) {
    double ns = measure([&] {
        for (auto index: queries) black_box(rb(index));
    });
    print_result(label, b, rb, ns / queries.size());
}

template<class ss>
void benchmark_select(const char* label, const bit_vector& b, const ss& sb, const std::vector<std::size_t>& queries) {
    double ns = measure([&] {
       for (auto index: queries) black_box(sb(index));
    });
    print_result(label, b, sb, ns / queries.size());
}

int main(int argc, char *argv[]) 
{
    uint64_t universe = 1000000000;
    uint64_t num = universe / 2;

    if (argc > 1) {
        if (argc == 2 || argc > 6) {
            std::cout << "Usage: " << argv[0] << " [bitvec_length number_of_ones [time_per_test_sec [query_num [rand_seed]]]]" << std::endl;
            return 0;
        }
        universe = stoull(argv[1]);
        num = stoull(argv[2]);
        if (argc > 3) {
            TIME_PER_TEST = chrono::seconds(stoull(argv[3]));
            if (argc > 4) {
                QUERIES = stoull(argv[4]);
                if (argc > 5) SEED = stoull(argv[5]);
            }
        }
    }

    std::cout << "Time per warm up and test " << TIME_PER_TEST.count() << "s, cooling " << (TIME_PER_TEST / 3).count()
         << "s, number of random queries " << QUERIES
         << ", random seed " << SEED << "." << std::endl;

    bit_vector b(universe, 0);
    uint64_t xs_seed = SEED;

    uint64_t number_of_ones = 0;
    for (uint64_t i = 0; i < universe; ++i) {   // uniform
        uint64_t remain_universe = universe - i;
        uint64_t remain_num = num - number_of_ones;
        bool included = xor_shift64(xs_seed) % remain_universe < remain_num;
        if (included) {
            b[i] = true;
            number_of_ones += 1;
        } 
    }
    num = number_of_ones;   // for future

    std::cout << "The bit vector consists of " << universe << " bits, including " << num << " ones." << std::endl;
    {
        std::vector<std::size_t> rank_queries = rand_queries(universe);
        benchmark_rank("rank_v", b, rank_support_v<>(&b), rank_queries);
        benchmark_rank("rank_v5", b, rank_support_v5<>(&b), rank_queries);
    }
    benchmark_select("select1 mcl", b, select_support_mcl<1>(&b), rand_queries(num, 1));
    benchmark_select("select0 mcl", b, select_support_mcl<0>(&b), rand_queries(universe-num, 1));
}