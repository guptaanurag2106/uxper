# [Arena and Bump Allocators]

Implementing Arena and Bump Allocators

## Context
- [Region based memory management](https://en.wikipedia.org/wiki/Region-based_memory_management)
- [Allocator Designs](https://os.phil-opp.com/allocator-designs)

## Build & Run

```bash
# Build
make

# Run
./test
```
## Benchmark

### Without warmup
=== Tight alloc/free loop (10000000 iters) ===
malloc/free:  0.061317 sec
pool:         0.042350 sec
arena:        0.035253 sec

=== Batch allocate/free (10000000 objects) ===
malloc batch: 0.382402 sec
pool batch:   0.123578 sec
arena batch:  0.028797 sec

=== Allocate + write (10000000 objects) ===
malloc touch: 0.376826 sec
pool touch:   0.147999 sec
arena touch:  0.169739 sec

### With warmup
=== Tight alloc/free loop (10000000 iters) ===
malloc/free:  0.060462 sec
pool:         0.040365 sec
arena:        0.035094 sec

=== Batch allocate/free (10000000 objects) ===
malloc batch: 0.369635 sec
pool batch:   0.129716 sec
arena batch:  0.028948 sec

=== Allocate + write (10000000 objects) ===
malloc touch: 0.372163 sec
pool touch:   0.147172 sec
arena touch:  0.173727 sec
