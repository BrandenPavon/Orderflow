#ifndef BOOK_ORDERBOOK_HPP
#define BOOK_ORDERBOOK_HPP

#include <cstddef>

#include "common/types.hpp"

#include "market/event.hpp"

#include "book/pricelevel.hpp"
#include "book/bookorder.hpp"

namespace book {

class Orderbook {
public:
  Orderbook();

  void apply(const market::MarketEvent& event);

  Price best_bid() const;
  Price best_ask() const;
  
  Quantity bid_quantity_at(Price price) const; 
  Quantity ask_quantity_at(Price price) const; 

  void reset();
private:

  void add(const market::MarketEvent& event);
  void cancel(const market::MarketEvent& event);
  void modify(const market::MarketEvent& event);
  void execute(const market::MarketEvent& event);

  static constexpr Price MAX_PRICE = 10000; // $1,000.00
  static constexpr Price MIN_PRICE = 1000; // $100.00
  static constexpr std::size_t NUM_LEVELS = MAX_PRICE - MIN_PRICE + 1;
  static constexpr std::size_t MAX_ORDERS = INVALID_ORDER_SLOT;
  // Order IDs 1 through 65535 are accepted; zero is reserved as invalid.
  static constexpr std::size_t MAX_ORDERS_ID = 65536;

  static constexpr OrderSlot INVALID_SLOT = INVALID_ORDER_SLOT;
  static constexpr Price NO_PRICE = 0;

  OrderSlot price_to_index(Price price) const;
  bool valid_order_id(OrderID order_id) const;
  OrderSlot find_order(OrderID order_id) const;

  PriceLevel& level_for(market::Side side, OrderSlot price_index);
  const PriceLevel& level_for(market::Side side, OrderSlot price_index) const;
  void append_to_level(OrderSlot slot);
  void remove_from_level(OrderSlot slot);
  void update_best_after_add(market::Side side, Price price);
  void update_best_after_remove(market::Side side, Price price);
  
  PriceLevel bids[NUM_LEVELS];
  PriceLevel asks[NUM_LEVELS];

  BookOrder orders[MAX_ORDERS];
  OrderSlot order_index[MAX_ORDERS_ID];
  OrderSlot allocate_order();
  void release_order(OrderSlot slot);
  OrderSlot free_head_;

  Price best_bid_;
  Price best_ask_;
};


}

#endif
