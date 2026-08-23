#ifndef MARKET_EVENT_HPP
#define MARKET_EVENT_HPP

#include <string_view>

#include "common/types.hpp"
#include "market/types.hpp"

namespace market {

enum class EventType : std::uint8_t {
    Add     = 0,
    Cancel  = 1,
    Modify  = 2,
    Execute = 3
};


class MarketEvent {
public:
  Timestamp timestamp_ns = 0;
  OrderID order_id = 0;
  Price price = 0;

  Quantity quantity = 0;
  SymbolID symbol_id = 0;
  
  EventType type = EventType::Add;
  Side side = Side::Buy;
  
  std::uint64_t sequence = 0;
  
  // No constructor because MarketEvent is simple aggregate, easy to seralize, copy & batch, across CPU/FPGA boundary
};

std::string_view side_to_string(Side side) noexcept;
bool is_valid(const MarketEvent& event) noexcept;

}
#endif
