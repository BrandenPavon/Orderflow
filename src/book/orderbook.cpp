
#include "book/pricelevel.hpp"
#include "market/event.hpp"

#include "book/orderbook.hpp"
#include "common/types.hpp"
#include "market/types.hpp"

namespace book {

  
void Orderbook::apply(const market::MarketEvent& event) {
  switch(event.type) {
    case market::EventType::Add:
      add(event);
      break;
    case market::EventType::Cancel:
      cancel(event);
      break;
    case market::EventType::Modify:
      modify(event);
      break;
    case market::EventType::Execute:
      execute(event);
      break;
  }
}

Price Orderbook::best_bid() const {
  return best_bid_;
}
Price Orderbook::best_ask() const {
  return best_ask_;
}
  
Quantity Orderbook::bid_quantity_at(Price price) const {

}
Quantity Orderbook::ask_quantity_at(Price price) const {

}

void Orderbook::reset() {

}

void Orderbook::add(const market::MarketEvent& event) {
  OrderSlot index = price_to_index(event.price);

  OrderSlot slot = allocate_order();
  // check if slot is valid

  if(slot == INVALID_SLOT) {
    // Full order book
    return;
  }
  
  BookOrder& order = orders[slot];

  order.id = event.order_id;
  order.price = event.price;
  order.quantity = event.quantity;
  order.side = event.side;
  
  order.prev = INVALID_SLOT;
  order.next = INVALID_SLOT;
  

  order.active = true;

  PriceLevel& level = (event.side == market::Side::Buy) ? bids[index] : asks[index];
  if(level.order_count == 0) {
    level.head_order = slot;
    level.tail_order = slot;
  } else {
    OrderSlot old_tail = level.tail_order;

    orders[old_tail].next = slot;
    order.prev = old_tail;
    
    level.tail_order = slot;
  }
  level.total_quantity += order.quantity;
  level.order_count += 1;

}

void Orderbook::cancel(const market::MarketEvent& event) {
  OrderSlot index = price_to_index(event.price);
  
  PriceLevel& level = (event.side == market::Side::Buy) ? bids[index] : asks[index];
  BookOrder& order = orders[event.order_id];

  OrderSlot prev = INVALID_SLOT;
  BookOrder& order = orders[curr];

  orders[order.prev].next = order.next;
  orders[order.next].prev = order.prev;
  level.total_quantity -= order.quantity;
  level.order_count -= 1;
}

void Orderbook::modify(const market::MarketEvent& event) {
  OrderSlot index = price_to_index(event.price);
  
  PriceLevel& level = (event.side == market::Side::Buy) ? bids[index] : asks[index];
  BookOrder& order = orders[event.order_id];
  order.quantity = event.quantity; 
  return;      
}

void Orderbook::execute(const market::MarketEvent& event) {

}

OrderSlot Orderbook::price_to_index(Price price) const {
    if(price < MIN_PRICE || price > MAX_PRICE) return -1;

    return static_cast<OrderSlot>(price - MIN_PRICE);
}

OrderSlot Orderbook::allocate_order() {
  if (free_head_ == INVALID_SLOT) return INVALID_SLOT;
  OrderSlot slot = free_head_;
  free_head_ = orders[slot].next; 

  return slot;
}

}




