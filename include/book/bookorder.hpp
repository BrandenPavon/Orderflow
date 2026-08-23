#ifndef BOOK_BOOKORDER_HPP
#define BOOK_BOOKORDER_HPP

#include "common/types.hpp"

#include "market/types.hpp"

#include "book/pricelevel.hpp"

struct BookOrder {
    OrderID id = 0;
    Price price = 0;
    Quantity quantity = 0;
    market::Side side = market::Side::Buy;

    book::OrderSlot prev = book::INVALID_ORDER_SLOT;
    book::OrderSlot next = book::INVALID_ORDER_SLOT;

    bool active = false;
};

#endif
