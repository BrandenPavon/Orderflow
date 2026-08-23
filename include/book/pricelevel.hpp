#ifndef ORDERBOOK_PRICE_LEVEL_HPP
#define ORDERBOOK_PRICE_LEVEL_HPP

#include <cstdint>
#include "common/types.hpp"

namespace book {

using OrderSlot = std::uint32_t;
inline constexpr OrderSlot INVALID_ORDER_SLOT = 65535;

class PriceLevel {
public:
  Quantity total_quantity = 0;

  OrderSlot head_order = INVALID_ORDER_SLOT;
  OrderSlot tail_order = INVALID_ORDER_SLOT;

  std::uint32_t order_count = 0;

};

}
#endif
