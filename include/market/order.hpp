#ifndef MARKET_ORDER_HPP
#define MARKET_ORDER_HPP

#include <cstdint>
#include "market/types.hpp"
#include "market/event.hpp"

namespace market {

enum class OrderStatus : std::uint8_t {
  Active          = 0,
  PartiallyFilled = 1,
  Filled          = 2,
  Cancelled       = 3

};

class MarketOrder {
public:
  Timestamp timestamp_ns;
  OrderID order_id;
  Price price;

  Quantity quantity;
  SymbolID symbol_id;
  
  Side side;
  OrderStatus status;
  
  std::uint64_t sequence;
  
 
  
};

}

#endif

