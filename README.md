# C++ Order Book

Low-latency limit order book and matching engine in C++.

## Features

- Price-time priority order matching
- Limit orders, market orders, and cancellations
- Order book with bid/ask levels and volume aggregation
- Event-driven: order added, filled, cancelled, modified
- Performance benchmarks (orders/sec, latency percentiles)

## Motivation

C++ is the language of low-latency trading systems. Most production trading systems at firms like Five Rings are built in C++. This project demonstrates you can write performant, latency-sensitive code.

## Getting Started

```bash
cmake -B build && cmake --build build
./build/order_book
```

## Project Structure

```
├── src/
│   ├── order.hpp        # Order types
│   ├── order_book.hpp   # Order book engine
│   ├── matching.hpp     # Matching engine
│   └── main.cpp         # Demo / benchmarks
├── tests/
├── CMakeLists.txt
└── README.md
```

## Example Output

```
Order Book:
  Bid: 100.00 (500)   Ask: 100.10 (300)
  Bid:  99.90 (200)   Ask: 100.20 (150)

Match: Sell 200 @ 100.00 — filled against bid
Latency: 1.2μs (p50), 3.8μs (p99)
```
