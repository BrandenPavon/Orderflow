# Orderflow

`Orderflow` currently implements a fixed-capacity, single-symbol order-book core
for applying add, cancel, modify, and execution events. Each price level uses an
intrusive FIFO queue, and order IDs are mapped to stable pool slots for constant-
time lookup.

## Current semantics

- Valid prices are integer values from `1000` through `10000`.
- Valid order IDs are `1` through `65535`.
- An empty side reports best price `0`.
- Duplicate active order IDs and invalid events are ignored.
- A same-price quantity decrease keeps FIFO priority.
- A quantity increase, price change, or side change moves the order to the tail.
- A zero-quantity modify cancels the order.
- An execution reduces remaining quantity and removes a fully filled order.

This component applies already-sequenced events. It does not yet cross incoming
orders, generate trades, or implement exchange order types.

## Build and test

```sh
make test
make sanitize
```
