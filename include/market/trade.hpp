#ifndef MARKET_TRADE_HPP
#define MARKET_TRADE_HPP

#include "common/types.hpp"
#include "market/types.hpp"

namespace market {



class MarketTrade {
  TradeID trade_id;

  OrderID buy_order_id; 
  OrderID sell_order_id; 

  Price price;
  Quantity quantity;

  Timestamp timestamp_ns;
   
  Side aggressor_side;
};

}
#endif

