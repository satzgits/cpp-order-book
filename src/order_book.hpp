#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include "order.hpp"
#include <map>
#include <queue>
#include <vector>
#include <mutex>
#include <algorithm>
#include <iostream>
#include <iomanip>

class OrderBook {
public:
    using OrderQueue = std::queue<Order>;

    OrderBook() : next_order_id_(1), next_trade_id_(1) {}

    uint64_t add_limit_order(Side side, double price, uint64_t quantity) {
        uint64_t id = next_order_id_++;
        uint64_t ts = current_timestamp();
        Order order(id, side, OrderType::LIMIT, price, quantity, ts);

        auto trades = match_order(order);
        trades_.insert(trades_.end(), trades.begin(), trades.end());

        if (order.quantity > 0) {
            if (side == Side::BUY) {
                bids_[price].push(order);
            } else {
                asks_[price].push(order);
            }
        }

        return id;
    }

    uint64_t add_market_order(Side side, uint64_t quantity) {
        uint64_t id = next_order_id_++;
        uint64_t ts = current_timestamp();
        Order order(id, side, OrderType::MARKET, 0.0, quantity, ts);

        auto trades = match_order(order);
        trades_.insert(trades_.end(), trades.begin(), trades.end());

        return id;
    }

    bool cancel_order(uint64_t order_id) {
        for (auto& [price, queue] : bids_) {
            std::queue<Order> filtered;
            bool found = false;
            while (!queue.empty()) {
                Order o = queue.front();
                queue.pop();
                if (o.id == order_id) {
                    found = true;
                    continue;
                }
                filtered.push(o);
            }
            queue = filtered;
            if (found) return true;
        }

        for (auto& [price, queue] : asks_) {
            std::queue<Order> filtered;
            bool found = false;
            while (!queue.empty()) {
                Order o = queue.front();
                queue.pop();
                if (o.id == order_id) {
                    found = true;
                    continue;
                }
                filtered.push(o);
            }
            queue = filtered;
            if (found) return true;
        }

        return false;
    }

    double get_best_bid() const {
        if (bids_.empty()) return 0.0;
        return bids_.rbegin()->first;
    }

    double get_best_ask() const {
        if (asks_.empty()) return 0.0;
        return asks_.begin()->first;
    }

    double get_spread() const {
        double bid = get_best_bid();
        double ask = get_best_ask();
        if (bid == 0.0 || ask == 0.0) return 0.0;
        return ask - bid;
    }

    double get_midpoint() const {
        double bid = get_best_bid();
        double ask = get_best_ask();
        if (bid == 0.0 || ask == 0.0) return 0.0;
        return 0.5 * (bid + ask);
    }

    uint64_t get_total_depth(Side side) const {
        const auto& book = (side == Side::BUY) ? bids_ : asks_;
        uint64_t total = 0;
        for (const auto& [price, queue] : book) {
            std::queue<Order> q = queue;
            while (!q.empty()) {
                total += q.front().quantity;
                q.pop();
            }
        }
        return total;
    }

    bool has_enough_depth(Side side, uint64_t quantity) const {
        return get_total_depth(side) >= quantity;
    }

    struct DepthLevel {
        double price;
        uint64_t volume;
        size_t order_count;
    };

    std::vector<DepthLevel> get_depth(size_t levels, Side side) const {
        std::vector<DepthLevel> depth;
        const auto& book = (side == Side::BUY) ? bids_ : asks_;
        if (side == Side::BUY) {
            auto it = book.rbegin();
            while (it != book.rend() && depth.size() < levels) {
                uint64_t vol = 0;
                std::queue<Order> q = it->second;
                while (!q.empty()) {
                    vol += q.front().quantity;
                    q.pop();
                }
                depth.push_back({it->first, vol, it->second.size()});
                ++it;
            }
        } else {
            auto it = book.begin();
            while (it != book.end() && depth.size() < levels) {
                uint64_t vol = 0;
                std::queue<Order> q = it->second;
                while (!q.empty()) {
                    vol += q.front().quantity;
                    q.pop();
                }
                depth.push_back({it->first, vol, it->second.size()});
                ++it;
            }
        }
        return depth;
    }

    uint64_t get_volume_at_price(double price, Side side) const {
        auto& book = (side == Side::BUY) ? bids_ : asks_;
        auto it = book.find(price);
        if (it == book.end()) return 0;

        uint64_t volume = 0;
        std::queue<Order> q = it->second;
        while (!q.empty()) {
            volume += q.front().quantity;
            q.pop();
        }
        return volume;
    }

    void print_book() const {
        std::cout << "\n=== Order Book ===\n";

        std::cout << "\n  Bids (descending price):\n";
        for (auto it = bids_.rbegin(); it != bids_.rend(); ++it) {
            uint64_t vol = 0;
            std::queue<Order> q = it->second;
            size_t count = q.size();
            while (!q.empty()) {
                vol += q.front().quantity;
                q.pop();
            }
            std::cout << "    " << std::fixed << std::setprecision(2)
                      << it->first << "  — " << vol << " shares ("
                      << count << " orders)\n";
        }

        std::cout << "\n  Asks (ascending price):\n";
        for (auto it = asks_.begin(); it != asks_.end(); ++it) {
            uint64_t vol = 0;
            std::queue<Order> q = it->second;
            size_t count = q.size();
            while (!q.empty()) {
                vol += q.front().quantity;
                q.pop();
            }
            std::cout << "    " << std::fixed << std::setprecision(2)
                      << it->first << "  — " << vol << " shares ("
                      << count << " orders)\n";
        }

        std::cout << "\n  Spread: " << get_spread() << "\n";
    }

    void print_trade_log() const {
        std::cout << "\n=== Trade Log ===\n";
        for (const auto& t : trades_) {
            std::cout << "  BUY " << t.buy_order_id << " <-> SELL "
                      << t.sell_order_id << " @ " << std::fixed
                      << std::setprecision(2) << t.price
                      << " x " << t.quantity << "\n";
        }
    }

    const std::vector<Trade>& trades() const { return trades_; }

    void clear() {
        bids_.clear();
        asks_.clear();
        trades_.clear();
    }

    void remove_empty_levels() {
        for (auto it = bids_.begin(); it != bids_.end();) {
            if (it->second.empty()) it = bids_.erase(it);
            else ++it;
        }
        for (auto it = asks_.begin(); it != asks_.end();) {
            if (it->second.empty()) it = asks_.erase(it);
            else ++it;
        }
    }

private:
    std::map<double, OrderQueue, std::greater<double>> bids_;
    std::map<double, OrderQueue> asks_;
    std::vector<Trade> trades_;
    uint64_t next_order_id_;
    uint64_t next_trade_id_;

    std::vector<Trade> match_order(Order& incoming) {
        std::vector<Trade> trades;

        if (incoming.type == OrderType::LIMIT) {
            if (incoming.side == Side::BUY) {
                match_bid_limit(incoming, trades);
            } else {
                match_ask_limit(incoming, trades);
            }
        } else {
            if (incoming.side == Side::BUY) {
                match_bid_market(incoming, trades);
            } else {
                match_ask_market(incoming, trades);
            }
        }

        return trades;
    }

    void match_bid_limit(Order& incoming, std::vector<Trade>& trades) {
        auto it = asks_.begin();
        while (it != asks_.end() && incoming.price >= it->first && incoming.quantity > 0) {
            auto& queue = it->second;
            while (!queue.empty() && incoming.quantity > 0) {
                Order& best_ask = queue.front();
                uint64_t fill_qty = std::min(incoming.quantity, best_ask.quantity);
                trades.emplace_back(incoming.id, best_ask.id, best_ask.price,
                                    fill_qty, current_timestamp());
                incoming.quantity -= fill_qty;
                best_ask.quantity -= fill_qty;
                if (best_ask.quantity == 0) queue.pop();
            }
            if (queue.empty()) ++it;
            else break;
        }
        remove_empty_levels();
    }

    void match_ask_limit(Order& incoming, std::vector<Trade>& trades) {
        auto it = bids_.begin();
        while (it != bids_.end() && incoming.price <= it->first && incoming.quantity > 0) {
            auto& queue = it->second;
            while (!queue.empty() && incoming.quantity > 0) {
                Order& best_bid = queue.front();
                uint64_t fill_qty = std::min(incoming.quantity, best_bid.quantity);
                trades.emplace_back(best_bid.id, incoming.id, best_bid.price,
                                    fill_qty, current_timestamp());
                incoming.quantity -= fill_qty;
                best_bid.quantity -= fill_qty;
                if (best_bid.quantity == 0) queue.pop();
            }
            if (queue.empty()) ++it;
            else break;
        }
        remove_empty_levels();
    }

    void match_bid_market(Order& incoming, std::vector<Trade>& trades) {
        auto it = asks_.begin();
        while (it != asks_.end() && incoming.quantity > 0) {
            auto& queue = it->second;
            while (!queue.empty() && incoming.quantity > 0) {
                Order& best_ask = queue.front();
                uint64_t fill_qty = std::min(incoming.quantity, best_ask.quantity);
                trades.emplace_back(incoming.id, best_ask.id, best_ask.price,
                                    fill_qty, current_timestamp());
                incoming.quantity -= fill_qty;
                best_ask.quantity -= fill_qty;
                if (best_ask.quantity == 0) queue.pop();
            }
            if (queue.empty()) ++it;
            else break;
        }
        remove_empty_levels();
    }

    void match_ask_market(Order& incoming, std::vector<Trade>& trades) {
        auto it = bids_.begin();
        while (it != bids_.end() && incoming.quantity > 0) {
            auto& queue = it->second;
            while (!queue.empty() && incoming.quantity > 0) {
                Order& best_bid = queue.front();
                uint64_t fill_qty = std::min(incoming.quantity, best_bid.quantity);
                trades.emplace_back(best_bid.id, incoming.id, best_bid.price,
                                    fill_qty, current_timestamp());
                incoming.quantity -= fill_qty;
                best_bid.quantity -= fill_qty;
                if (best_bid.quantity == 0) queue.pop();
            }
            if (queue.empty()) ++it;
            else break;
        }
        remove_empty_levels();
    }

    static uint64_t current_timestamp() {
        return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()
        ).count();
    }
};

#endif
