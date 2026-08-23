#include <iostream>
#include <string_view>

#include "book/orderbook.hpp"
#include "market/event.hpp"
#include "market/types.hpp"

namespace {

market::MarketEvent event(market::EventType type, OrderID order_id,
                          Price price = 0, Quantity quantity = 0,
                          market::Side side = market::Side::Buy) {
  market::MarketEvent result;
  result.type = type;
  result.order_id = order_id;
  result.price = price;
  result.quantity = quantity;
  result.side = side;
  return result;
}

bool expect(bool condition, std::string_view message) {
  if (!condition) {
    std::cerr << "  assertion failed: " << message << '\n';
  }
  return condition;
}

bool test_initial_state_and_add() {
  book::Orderbook book;
  bool ok = true;
  ok &= expect(book.best_bid() == 0, "empty book has no best bid");
  ok &= expect(book.best_ask() == 0, "empty book has no best ask");

  book.apply(event(market::EventType::Add, 101, 5000, 10,
                   market::Side::Buy));
  book.apply(event(market::EventType::Add, 999, 5100, 7,
                   market::Side::Buy));
  book.apply(event(market::EventType::Add, 201, 6000, 12,
                   market::Side::Sell));
  book.apply(event(market::EventType::Add, 202, 5900, 5,
                   market::Side::Sell));

  ok &= expect(book.best_bid() == 5100, "highest bid is selected");
  ok &= expect(book.best_ask() == 5900, "lowest ask is selected");
  ok &= expect(book.bid_quantity_at(5000) == 10,
               "bid quantity is aggregated");
  ok &= expect(book.ask_quantity_at(6000) == 12,
               "ask quantity is aggregated");
  return ok;
}

bool test_cancel_and_best_price_updates() {
  book::Orderbook book;
  bool ok = true;
  book.apply(event(market::EventType::Add, 42, 5000, 10,
                   market::Side::Buy));
  book.apply(event(market::EventType::Add, 7, 5000, 20,
                   market::Side::Buy));
  book.apply(event(market::EventType::Add, 900, 5000, 30,
                   market::Side::Buy));
  book.apply(event(market::EventType::Add, 3, 4900, 8,
                   market::Side::Buy));

  book.apply(event(market::EventType::Cancel, 7));
  ok &= expect(book.bid_quantity_at(5000) == 40,
               "cancelling a middle FIFO order updates quantity");
  book.apply(event(market::EventType::Cancel, 42));
  book.apply(event(market::EventType::Cancel, 900));
  ok &= expect(book.bid_quantity_at(5000) == 0,
               "head and tail cancellation leaves an empty level");
  ok &= expect(book.best_bid() == 4900,
               "best bid falls to the next populated level");

  book.apply(event(market::EventType::Cancel, 3));
  ok &= expect(book.best_bid() == 0, "last bid removal clears best bid");
  book.apply(event(market::EventType::Cancel, 12345));
  return ok;
}

bool test_modify() {
  book::Orderbook book;
  bool ok = true;
  book.apply(event(market::EventType::Add, 10, 5000, 10,
                   market::Side::Buy));
  book.apply(event(market::EventType::Add, 11, 5000, 20,
                   market::Side::Buy));

  book.apply(event(market::EventType::Modify, 10, 5000, 15,
                   market::Side::Buy));
  ok &= expect(book.bid_quantity_at(5000) == 35,
               "quantity increase updates the level");
  book.apply(event(market::EventType::Modify, 11, 5000, 5,
                   market::Side::Buy));
  ok &= expect(book.bid_quantity_at(5000) == 20,
               "quantity decrease updates the level");

  book.apply(event(market::EventType::Modify, 10, 5100, 7,
                   market::Side::Buy));
  ok &= expect(book.bid_quantity_at(5000) == 5,
               "price change removes old-level quantity");
  ok &= expect(book.bid_quantity_at(5100) == 7,
               "price change adds new-level quantity");
  ok &= expect(book.best_bid() == 5100, "price change updates best bid");

  book.apply(event(market::EventType::Modify, 10, 5200, 9,
                   market::Side::Sell));
  ok &= expect(book.bid_quantity_at(5100) == 0,
               "side change removes the old bid");
  ok &= expect(book.ask_quantity_at(5200) == 9,
               "side change adds the new ask");
  ok &= expect(book.best_bid() == 5000, "side change repairs best bid");
  ok &= expect(book.best_ask() == 5200, "side change updates best ask");

  book.apply(event(market::EventType::Modify, 10, 0, 0,
                   market::Side::Sell));
  ok &= expect(book.best_ask() == 0, "zero-size modify cancels the order");
  return ok;
}

bool test_execute() {
  book::Orderbook book;
  bool ok = true;
  book.apply(event(market::EventType::Add, 55, 6100, 20,
                   market::Side::Sell));
  book.apply(event(market::EventType::Add, 56, 6200, 4,
                   market::Side::Sell));

  book.apply(event(market::EventType::Execute, 55, 0, 6));
  ok &= expect(book.ask_quantity_at(6100) == 14,
               "partial execution reduces remaining quantity");
  ok &= expect(book.best_ask() == 6100,
               "partial execution preserves the price level");

  book.apply(event(market::EventType::Execute, 55, 0, 14));
  ok &= expect(book.ask_quantity_at(6100) == 0,
               "full execution removes the order");
  ok &= expect(book.best_ask() == 6200,
               "full execution advances the best ask");

  book.apply(event(market::EventType::Execute, 56, 0, 100));
  ok &= expect(book.best_ask() == 0,
               "execution larger than remaining size fully removes order");
  return ok;
}

bool test_invalid_inputs_and_duplicates() {
  book::Orderbook book;
  bool ok = true;
  book.apply(event(market::EventType::Add, 1, 500, 10,
                   market::Side::Buy));
  book.apply(event(market::EventType::Add, 0, 5000, 10,
                   market::Side::Buy));
  book.apply(event(market::EventType::Add, 65536, 5000, 10,
                   market::Side::Buy));
  book.apply(event(market::EventType::Add, 2, 5000, 0,
                   market::Side::Buy));
  ok &= expect(book.best_bid() == 0, "invalid adds are ignored safely");
  ok &= expect(book.bid_quantity_at(500) == 0,
               "out-of-range query returns zero");

  book.apply(event(market::EventType::Add, 5, 5000, 10,
                   market::Side::Buy));
  book.apply(event(market::EventType::Add, 5, 5100, 99,
                   market::Side::Buy));
  ok &= expect(book.bid_quantity_at(5000) == 10,
               "duplicate active order ID is ignored");
  ok &= expect(book.bid_quantity_at(5100) == 0,
               "duplicate ID does not create another order");

  book.apply(event(market::EventType::Modify, 5, 500, 20,
                   market::Side::Buy));
  ok &= expect(book.bid_quantity_at(5000) == 10,
               "invalid modify leaves the original order unchanged");

  book.apply(event(market::EventType::Add, 65535, 10000, 1,
                   market::Side::Sell));
  ok &= expect(book.best_ask() == 10000,
               "maximum valid order ID and price are accepted");
  return ok;
}

bool test_event_utilities() {
  bool ok = true;
  ok &= expect(market::side_to_string(market::Side::Buy) == "BUY",
               "buy side has a stable string representation");
  ok &= expect(market::side_to_string(market::Side::Sell) == "SELL",
               "sell side has a stable string representation");
  ok &= expect(
      market::side_to_string(static_cast<market::Side>(99)) == "UNKNOWN",
      "invalid side is reported as unknown");

  ok &= expect(market::is_valid(
                   event(market::EventType::Add, 1, 5000, 1,
                         market::Side::Buy)),
               "valid add event passes validation");
  ok &= expect(!market::is_valid(
                   event(market::EventType::Add, 1, 5000, 1,
                         static_cast<market::Side>(99))),
               "add with invalid side fails validation");
  ok &= expect(market::is_valid(event(market::EventType::Cancel, 1)),
               "cancel only requires an order ID");
  ok &= expect(market::is_valid(
                   event(market::EventType::Modify, 1, 0, 0)),
               "zero-size modify is a valid cancel request");
  ok &= expect(!market::is_valid(
                   event(market::EventType::Execute, 1, 0, 0)),
               "zero-size execution fails validation");
  return ok;
}

bool test_reset_and_slot_reuse() {
  book::Orderbook book;
  bool ok = true;
  for (int i = 0; i < 70000; ++i) {
    book.apply(event(market::EventType::Add, 1, 5000, 1,
                     market::Side::Buy));
    book.apply(event(market::EventType::Cancel, 1));
  }
  ok &= expect(book.best_bid() == 0,
               "cancelled slots return to the fixed-capacity free list");

  book.apply(event(market::EventType::Add, 8, 5000, 3,
                   market::Side::Buy));
  book.apply(event(market::EventType::Add, 9, 6000, 4,
                   market::Side::Sell));
  book.reset();
  ok &= expect(book.best_bid() == 0 && book.best_ask() == 0,
               "reset clears best prices");
  ok &= expect(book.bid_quantity_at(5000) == 0 &&
                   book.ask_quantity_at(6000) == 0,
               "reset clears quantities");
  return ok;
}

bool test_full_capacity() {
  book::Orderbook book;
  bool ok = true;
  for (OrderID id = 1; id <= 65535; ++id) {
    book.apply(event(market::EventType::Add, id, 5000, 1,
                     market::Side::Buy));
  }
  ok &= expect(book.bid_quantity_at(5000) == 65535,
               "every fixed-pool slot can be allocated");

  book.apply(event(market::EventType::Cancel, 32768));
  book.apply(event(market::EventType::Add, 32768, 5000, 2,
                   market::Side::Buy));
  ok &= expect(book.bid_quantity_at(5000) == 65536,
               "a released slot can be reused after the pool was full");
  return ok;
}

}  // namespace

int main() {
  struct TestCase {
    std::string_view name;
    bool (*run)();
  };

  const TestCase tests[] = {
      {"initial state and add", test_initial_state_and_add},
      {"cancel and best-price updates", test_cancel_and_best_price_updates},
      {"modify", test_modify},
      {"execute", test_execute},
      {"invalid inputs and duplicates", test_invalid_inputs_and_duplicates},
      {"event utilities", test_event_utilities},
      {"reset and slot reuse", test_reset_and_slot_reuse},
      {"full capacity", test_full_capacity},
  };

  int failed = 0;
  for (const TestCase& test : tests) {
    const bool passed = test.run();
    std::cout << (passed ? "PASS" : "FAIL") << "  " << test.name << '\n';
    failed += !passed;
  }

  std::cout << (failed == 0 ? "All tests passed.\n" : "Test failures detected.\n");
  return failed == 0 ? 0 : 1;
}
