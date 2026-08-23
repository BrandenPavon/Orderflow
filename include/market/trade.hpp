#ifndef MARKET_TRADE_HPP
#define MARKET_TRADE_HPP

#include "common/types.hpp"
#include "market/types.hpp"

namespace market {



class MarketTrade {
public:
  TradeID trade_id = 0;

  OrderID buy_order_id = 0;
  OrderID sell_order_id = 0;

  Price price = 0;
  Quantity quantity = 0;

  Timestamp timestamp_ns = 0;
   
  Side aggressor_side = Side::Buy;
};

}
#endif
