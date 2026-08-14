#ifndef EVENT_HPP
#define EVENT_HPP

#include <cstdint>

using Timestamp = std::uint64_t; // Unix timestamp in nano seconds UTC
using OrderID = std::uint64_t; // start at 1, 
using Price = std::int64_t; // $10993.32 -> 1099332
using Quantity = std::uint64_t; 
using SymbolID = std::uint64_t; 

enum class Side : std::uint8_t {
    Buy  = 0, // Bid
    Sell = 1  // Ask
};

enum class EventType : std::uint8_t {
    Add     = 0,
    Cancel  = 1,
    Modify  = 2,
    Execute = 3,
    Trade   = 4,
    Reset   = 5
};


class MarketEvent {
  Timestamp timestamp_ns;
  OrderID order_id;
  Price price;

  Quantity quantity;
  SymbolID symbol_id;

};
#endif
