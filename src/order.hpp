#ifndef ORDER_HPP
#define ORDER_HPP

#include <cstdint>
#include <string>
#include <chrono>

enum class Side { BUY, SELL };
enum class OrderType { LIMIT, MARKET };

struct Order {
    uint64_t id;
    Side side;
    OrderType type;
    double price;
    uint64_t quantity;
    uint64_t timestamp;

    Order() : id(0), side(Side::BUY), type(OrderType::LIMIT),
              price(0.0), quantity(0), timestamp(0) {}

    Order(uint64_t id, Side side, OrderType type, double price,
          uint64_t quantity, uint64_t timestamp)
        : id(id), side(side), type(type), price(price),
          quantity(quantity), timestamp(timestamp) {}

    std::string side_str() const {
        return side == Side::BUY ? "BUY" : "SELL";
    }

    std::string type_str() const {
        return type == OrderType::LIMIT ? "LIMIT" : "MARKET";
    }
};

struct Trade {
    uint64_t buy_order_id;
    uint64_t sell_order_id;
    double price;
    uint64_t quantity;
    uint64_t timestamp;

    Trade(uint64_t buy_id, uint64_t sell_id, double p, uint64_t q, uint64_t ts)
        : buy_order_id(buy_id), sell_order_id(sell_id),
          price(p), quantity(q), timestamp(ts) {}
};

#endif
