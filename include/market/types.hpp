#ifndef MARKET_TYPES_HPP
#define MARKET_TYPES_HPP

#include <cstdint>

namespace market {


enum class Side : std::uint8_t {
    Buy  = 0, // Bid
    Sell = 1  // Ask
};


}
#endif
