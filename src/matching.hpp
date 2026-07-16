#ifndef MATCHING_HPP
#define MATCHING_HPP

#include "order.hpp"
#include <vector>
#include <chrono>

struct MatchResult {
    std::vector<Trade> trades;
    uint64_t execution_time_ns;

    double avg_latency_us() const {
        if (trades.empty()) return 0.0;
        return static_cast<double>(execution_time_ns) / trades.size() / 1000.0;
    }
};

class MatchingEngineBenchmark {
public:
    struct BenchmarkResult {
        uint64_t num_operations;
        double total_time_sec;
        double throughput;
        double p50_latency_us;
        double p95_latency_us;
        double p99_latency_us;
    };

    static BenchmarkResult run_benchmark(uint64_t num_orders = 1000000) {
        OrderBook book;
        std::vector<double> latencies;
        auto start = std::chrono::high_resolution_clock::now();

        for (uint64_t i = 0; i < num_orders; ++i) {
            auto op_start = std::chrono::high_resolution_clock::now();

            Side side = (i % 2 == 0) ? Side::BUY : Side::SELL;
            double price = 100.0 + (static_cast<double>(i % 100) - 50) * 0.1;
            uint64_t qty = (i % 10 + 1) * 100;

            if (i % 100 == 0) {
                book.cancel_order(i > 50 ? i - 50 : 1);
            } else {
                book.add_limit_order(side, price, qty);
            }

            auto op_end = std::chrono::high_resolution_clock::now();
            double latency_us = std::chrono::duration<double, std::micro>(op_end - op_start).count();
            latencies.push_back(latency_us);
        }

        auto end = std::chrono::high_resolution_clock::now();
        double total_sec = std::chrono::duration<double>(end - start).count();
        std::sort(latencies.begin(), latencies.end());

        size_t n = latencies.size();
        BenchmarkResult result;
        result.num_operations = num_orders;
        result.total_time_sec = total_sec;
        result.throughput = num_orders / total_sec;
        result.p50_latency_us = latencies[n / 2];
        result.p95_latency_us = latencies[static_cast<size_t>(n * 0.95)];
        result.p99_latency_us = latencies[static_cast<size_t>(n * 0.99)];

        return result;
    }

    static void print_benchmark(const BenchmarkResult& r) {
        std::cout << "\n=== Benchmarks ===\n";
        std::cout << "  " << r.num_operations << " operations in "
                  << std::fixed << std::setprecision(2) << r.total_time_sec << "s\n";
        std::cout << "  Throughput: " << std::fixed << std::setprecision(0)
                  << r.throughput << " ops/sec\n";
        std::cout << "  Latency: p50=" << std::fixed << std::setprecision(1)
                  << r.p50_latency_us << "us, p95=" << r.p95_latency_us
                  << "us, p99=" << r.p99_latency_us << "us\n";
    }
};

#endif
