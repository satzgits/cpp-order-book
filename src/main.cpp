#include "order_book.hpp"
#include "matching.hpp"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

void run_demo() {
    OrderBook book;

    std::cout << "\n╔══════════════════════════════════════════════╗\n";
    std::cout <<   "║        C++ ORDER BOOK — DEMO                 ║\n";
    std::cout <<   "╚══════════════════════════════════════════════╝\n";

    std::cout << "\n[1] Adding initial orders...\n";
    book.add_limit_order(Side::BUY, 100.00, 500);
    book.add_limit_order(Side::BUY, 99.95, 200);
    book.add_limit_order(Side::BUY, 100.05, 300);
    book.add_limit_order(Side::SELL, 100.10, 300);
    book.add_limit_order(Side::SELL, 100.15, 200);
    book.add_limit_order(Side::SELL, 100.20, 100);
    book.print_book();

    std::cout << "\n[2] Market sell 100 shares (matches best bid @ 100.05)...\n";
    book.add_market_order(Side::SELL, 100);
    book.print_book();

    std::cout << "\n[3] Market buy 50 shares (matches best ask @ 100.10)...\n";
    book.add_market_order(Side::BUY, 50);
    book.print_book();

    std::cout << "\n[4] Limit buy @ 100.15 (crosses spread, matches asks)...\n";
    book.add_limit_order(Side::BUY, 100.15, 400);
    book.print_book();

    std::cout << "\n[5] Limit sell @ 99.90 (crosses spread, matches bids)...\n";
    book.add_limit_order(Side::SELL, 99.90, 200);
    book.print_book();

    book.print_trade_log();

    std::cout << "\n[6] Running benchmark (1,000,000 operations)...\n";
    auto result = MatchingEngineBenchmark::run_benchmark(1000000);
    MatchingEngineBenchmark::print_benchmark(result);
}

int main() {
    run_demo();
    return 0;
}
