#ifndef MARKET_ORDER_HPP
#define MARKET_ORDER_HPP

#include <cstdint>
#include "common/types.hpp"
#include "market/types.hpp"

namespace market {

enum class OrderStatus : std::uint8_t {
  Active          = 0,
  PartiallyFilled = 1,
  Filled          = 2,
  Cancelled       = 3

};

class MarketOrder {
public:
  Timestamp timestamp_ns = 0;
  OrderID order_id = 0;
  Price price = 0;

  Quantity quantity = 0;
  SymbolID symbol_id = 0;
  
  Side side = Side::Buy;
  OrderStatus status = OrderStatus::Active;
  
  std::uint64_t sequence = 0;
  
 
  
};

}

#endif
