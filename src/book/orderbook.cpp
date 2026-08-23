#include "book/orderbook.hpp"

#include <cstddef>

namespace book {

Orderbook::Orderbook() {
  reset();
}

void Orderbook::apply(const market::MarketEvent& event) {
  if (!market::is_valid(event)) {
    return;
  }

  switch (event.type) {
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
  const OrderSlot index = price_to_index(price);
  return index == INVALID_SLOT ? 0 : bids[index].total_quantity;
}

Quantity Orderbook::ask_quantity_at(Price price) const {
  const OrderSlot index = price_to_index(price);
  return index == INVALID_SLOT ? 0 : asks[index].total_quantity;
}

void Orderbook::reset() {
  for (std::size_t i = 0; i < NUM_LEVELS; ++i) {
    bids[i].total_quantity = 0;
    bids[i].head_order = INVALID_SLOT;
    bids[i].tail_order = INVALID_SLOT;
    bids[i].order_count = 0;

    asks[i].total_quantity = 0;
    asks[i].head_order = INVALID_SLOT;
    asks[i].tail_order = INVALID_SLOT;
    asks[i].order_count = 0;
  }

  for (OrderSlot i = 0; i < MAX_ORDERS; ++i) {
    orders[i].id = 0;
    orders[i].price = 0;
    orders[i].quantity = 0;
    orders[i].side = market::Side::Buy;
    orders[i].prev = INVALID_SLOT;
    orders[i].next = (i + 1 < MAX_ORDERS) ? i + 1 : INVALID_SLOT;
    orders[i].active = false;
  }

  for (std::size_t i = 0; i < MAX_ORDERS_ID; ++i) {
    order_index[i] = INVALID_SLOT;
  }

  free_head_ = 0;
  best_bid_ = NO_PRICE;
  best_ask_ = NO_PRICE;
}

void Orderbook::add(const market::MarketEvent& event) {
  const OrderSlot price_index = price_to_index(event.price);
  if (price_index == INVALID_SLOT || event.quantity == 0 ||
      !valid_order_id(event.order_id) ||
      find_order(event.order_id) != INVALID_SLOT) {
    return;
  }

  const OrderSlot slot = allocate_order();
  if (slot == INVALID_SLOT) {
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

  order_index[event.order_id] = slot;
  append_to_level(slot);
  update_best_after_add(order.side, order.price);
}

void Orderbook::cancel(const market::MarketEvent& event) {
  const OrderSlot slot = find_order(event.order_id);
  if (slot == INVALID_SLOT) {
    return;
  }

  const market::Side side = orders[slot].side;
  const Price price = orders[slot].price;
  remove_from_level(slot);
  order_index[event.order_id] = INVALID_SLOT;
  release_order(slot);
  update_best_after_remove(side, price);
}

void Orderbook::modify(const market::MarketEvent& event) {
  const OrderSlot slot = find_order(event.order_id);
  if (slot == INVALID_SLOT) {
    return;
  }

  if (event.quantity == 0) {
    cancel(event);
    return;
  }

  if (price_to_index(event.price) == INVALID_SLOT) {
    return;
  }

  BookOrder& order = orders[slot];
  const Price old_price = order.price;
  const Quantity old_quantity = order.quantity;
  const market::Side old_side = order.side;

  if (old_price == event.price && old_side == event.side) {
    PriceLevel& level = level_for(old_side, price_to_index(old_price));
    if (event.quantity <= old_quantity) {
      // Decreasing size keeps queue priority.
      level.total_quantity -= old_quantity - event.quantity;
      order.quantity = event.quantity;
      return;
    }

    // Increasing size loses queue priority and moves the order to the tail.
    remove_from_level(slot);
    order.quantity = event.quantity;
    append_to_level(slot);
    return;
  }

  // A price or side change loses priority and joins the new level at its tail.
  remove_from_level(slot);
  update_best_after_remove(old_side, old_price);

  order.price = event.price;
  order.quantity = event.quantity;
  order.side = event.side;
  append_to_level(slot);
  update_best_after_add(order.side, order.price);
}

void Orderbook::execute(const market::MarketEvent& event) {
  if (event.quantity == 0) {
    return;
  }

  const OrderSlot slot = find_order(event.order_id);
  if (slot == INVALID_SLOT) {
    return;
  }

  BookOrder& order = orders[slot];
  if (event.quantity < order.quantity) {
    PriceLevel& level = level_for(order.side, price_to_index(order.price));
    order.quantity -= event.quantity;
    level.total_quantity -= event.quantity;
    return;
  }

  const market::Side side = order.side;
  const Price price = order.price;
  const OrderID order_id = order.id;
  remove_from_level(slot);
  order_index[order_id] = INVALID_SLOT;
  release_order(slot);
  update_best_after_remove(side, price);
}

OrderSlot Orderbook::price_to_index(Price price) const {
  if (price < MIN_PRICE || price > MAX_PRICE) {
    return INVALID_SLOT;
  }
  return static_cast<OrderSlot>(price - MIN_PRICE);
}

bool Orderbook::valid_order_id(OrderID order_id) const {
  return order_id > 0 && order_id < MAX_ORDERS_ID;
}

OrderSlot Orderbook::find_order(OrderID order_id) const {
  if (!valid_order_id(order_id)) {
    return INVALID_SLOT;
  }

  const OrderSlot slot = order_index[order_id];
  if (slot == INVALID_SLOT || !orders[slot].active ||
      orders[slot].id != order_id) {
    return INVALID_SLOT;
  }
  return slot;
}

PriceLevel& Orderbook::level_for(market::Side side, OrderSlot price_index) {
  return side == market::Side::Buy ? bids[price_index] : asks[price_index];
}

const PriceLevel& Orderbook::level_for(market::Side side,
                                       OrderSlot price_index) const {
  return side == market::Side::Buy ? bids[price_index] : asks[price_index];
}

void Orderbook::append_to_level(OrderSlot slot) {
  BookOrder& order = orders[slot];
  PriceLevel& level = level_for(order.side, price_to_index(order.price));

  order.prev = level.tail_order;
  order.next = INVALID_SLOT;

  if (level.tail_order == INVALID_SLOT) {
    level.head_order = slot;
  } else {
    orders[level.tail_order].next = slot;
  }
  level.tail_order = slot;
  level.total_quantity += order.quantity;
  ++level.order_count;
}

void Orderbook::remove_from_level(OrderSlot slot) {
  BookOrder& order = orders[slot];
  PriceLevel& level = level_for(order.side, price_to_index(order.price));

  if (order.prev == INVALID_SLOT) {
    level.head_order = order.next;
  } else {
    orders[order.prev].next = order.next;
  }

  if (order.next == INVALID_SLOT) {
    level.tail_order = order.prev;
  } else {
    orders[order.next].prev = order.prev;
  }

  level.total_quantity -= order.quantity;
  --level.order_count;
  if (level.order_count == 0) {
    level.total_quantity = 0;
    level.head_order = INVALID_SLOT;
    level.tail_order = INVALID_SLOT;
  }

  order.prev = INVALID_SLOT;
  order.next = INVALID_SLOT;
}

void Orderbook::update_best_after_add(market::Side side, Price price) {
  if (side == market::Side::Buy) {
    if (best_bid_ == NO_PRICE || price > best_bid_) {
      best_bid_ = price;
    }
  } else if (best_ask_ == NO_PRICE || price < best_ask_) {
    best_ask_ = price;
  }
}

void Orderbook::update_best_after_remove(market::Side side, Price price) {
  const OrderSlot removed_index = price_to_index(price);
  if (side == market::Side::Buy) {
    if (best_bid_ != price || bids[removed_index].order_count != 0) {
      return;
    }

    best_bid_ = NO_PRICE;
    for (OrderSlot i = removed_index; i > 0; --i) {
      const OrderSlot candidate = i - 1;
      if (bids[candidate].order_count != 0) {
        best_bid_ = MIN_PRICE + candidate;
        break;
      }
    }
    return;
  }

  if (best_ask_ != price || asks[removed_index].order_count != 0) {
    return;
  }

  best_ask_ = NO_PRICE;
  for (OrderSlot candidate = removed_index + 1; candidate < NUM_LEVELS;
       ++candidate) {
    if (asks[candidate].order_count != 0) {
      best_ask_ = MIN_PRICE + candidate;
      break;
    }
  }
}

OrderSlot Orderbook::allocate_order() {
  if (free_head_ == INVALID_SLOT) {
    return INVALID_SLOT;
  }

  const OrderSlot slot = free_head_;
  free_head_ = orders[slot].next;
  orders[slot].next = INVALID_SLOT;
  orders[slot].prev = INVALID_SLOT;
  return slot;
}

void Orderbook::release_order(OrderSlot slot) {
  BookOrder& order = orders[slot];
  order.id = 0;
  order.price = 0;
  order.quantity = 0;
  order.side = market::Side::Buy;
  order.prev = INVALID_SLOT;
  order.next = free_head_;
  order.active = false;
  free_head_ = slot;
}

}  // namespace book
