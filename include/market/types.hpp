#ifndef MARKET_TYPES_HPP
#define MARKET_TYPES_HPP

#include <cstdint>

using Timestamp = std::uint64_t; // Unix timestamp in nano seconds UTC
using OrderID = std::uint64_t; // start at 1, 
using Price = std::int64_t; // $10993.32 -> 1099332
using Quantity = std::uint64_t; 
using SymbolID = std::uint64_t; 

#endif
