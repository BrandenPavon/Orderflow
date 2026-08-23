#include "market/event.hpp"

namespace market {

namespace {

bool valid_side(Side side) noexcept {
  return side == Side::Buy || side == Side::Sell;
}

}  // namespace

std::string_view side_to_string(Side side) noexcept {
  switch (side) {
    case Side::Buy:
      return "BUY";
    case Side::Sell:
      return "SELL";
  }
  return "UNKNOWN";
}

bool is_valid(const MarketEvent& event) noexcept {
  if (event.order_id == 0) {
    return false;
  }

  switch (event.type) {
    case EventType::Add:
      return event.price > 0 && event.quantity > 0 && valid_side(event.side);
    case EventType::Cancel:
      return true;
    case EventType::Modify:
      return event.quantity == 0 ||
             (event.price > 0 && valid_side(event.side));
    case EventType::Execute:
      return event.quantity > 0;
  }
  return false;
}

}  // namespace market
