#ifndef ORDERBOOK_PRICE_LEVEL_HPP
#define ORDERBOOK_PRICE_LEVEL_HPP

#include <cstdint>
#include "common/types.hpp"

namespace book {

using OrderSlot = std::uint32_t;

class PriceLevel {
public:
  Quantity total_quantity;

  OrderSlot head_order;
  OrderSlot tail_order;

  std::uint32_t order_count;

};

}
#endif
