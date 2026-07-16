#include "../src/order_book.hpp"
#include "../src/matching.hpp"
#include <cassert>
#include <iostream>

void test_basic_order_insertion() {
    OrderBook book;
    book.add_limit_order(Side::BUY, 100.00, 500);
    book.add_limit_order(Side::SELL, 100.10, 300);

    assert(book.get_best_bid() == 100.00);
    assert(book.get_best_ask() == 100.10);
    assert(book.get_spread() == 0.10);
    std::cout << "  ✓ Basic order insertion and book state\n";
}

void test_market_order_fills_against_best() {
    OrderBook book;
    book.add_limit_order(Side::BUY, 100.00, 500);
    book.add_limit_order(Side::SELL, 99.90, 200);
    book.add_market_order(Side::BUY, 100);

    assert(book.trades().size() >= 1);
    std::cout << "  ✓ Market order fills against best ask\n";
}

void test_limit_order_crosses_spread() {
    OrderBook book;
    book.add_limit_order(Side::BUY, 100.00, 500);
    book.add_limit_order(Side::SELL, 100.10, 300);

    book.add_limit_order(Side::BUY, 100.15, 200);
    assert(book.trades().size() >= 1);

    book.add_limit_order(Side::SELL, 99.95, 100);
    assert(book.trades().size() >= 2);
    std::cout << "  ✓ Limit orders crossing the spread generate trades\n";
}

void test_cancel_order() {
    OrderBook book;
    uint64_t id = book.add_limit_order(Side::BUY, 100.00, 500);
    assert(book.get_best_bid() == 100.00);

    bool cancelled = book.cancel_order(id);
    assert(cancelled);
    assert(book.get_best_bid() == 0.0);
    std::cout << "  ✓ Order cancellation works\n";
}

void test_partial_fill() {
    OrderBook book;
    book.add_limit_order(Side::SELL, 100.00, 500);
    book.add_market_order(Side::BUY, 200);
    assert(book.trades().size() == 1);
    assert(book.trades()[0].quantity == 200);

    uint64_t remaining = book.get_volume_at_price(100.00, Side::SELL);
    assert(remaining == 300);
    std::cout << "  ✓ Partial fills leave remaining quantity\n";
}

void test_best_bid_ask_update() {
    OrderBook book;
    book.add_limit_order(Side::BUY, 100.00, 500);
    book.add_limit_order(Side::BUY, 100.05, 300);
    assert(book.get_best_bid() == 100.05);

    book.add_limit_order(Side::SELL, 100.10, 300);
    book.add_limit_order(Side::SELL, 100.05, 200);
    book.add_market_order(Side::SELL, 300);
    assert(book.get_best_bid() == 100.00);
    std::cout << "  ✓ Best bid/ask update after fills\n";
}

void test_spread_calculation() {
    OrderBook book;
    assert(book.get_spread() == 0.0);

    book.add_limit_order(Side::BUY, 100.00, 500);
    assert(book.get_spread() == 0.0);

    book.add_limit_order(Side::SELL, 100.10, 300);
    assert(book.get_spread() == 0.10);
    std::cout << "  ✓ Spread calculation with empty sides\n";
}

int main() {
    std::cout << "Running C++ Order Book Tests...\n\n";

    test_basic_order_insertion();
    test_market_order_fills_against_best();
    test_limit_order_crosses_spread();
    test_cancel_order();
    test_partial_fill();
    test_best_bid_ask_update();
    test_spread_calculation();

    std::cout << "\nAll order book tests passed!\n";
    return 0;
}
