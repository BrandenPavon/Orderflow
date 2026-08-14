#include <string_view>
#include "market/event.hpp"

namespace market {

std::string_view side_to_string(Side side) {
  switch(side) {
    case Side::Buy: return "BUY";
    case Side::Sell: return "Sell";
  }
  return "UNKNOWN";
}

bool is_valid(const MarketEvent& event) {
  if(event.order_id <= 0) return false;
  if(event.quantity <= 0) return false;
  return true;
}

}
