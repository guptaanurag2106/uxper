# Timing Attacks

## Context
- [Strcmp](https://github.com/jsutch/strcmp_timing_attacks_demo)
- [Does it even work](https://www.sjoerdlangkemper.nl/2024/05/29/string-comparison-timing-attacks/)
- [TSC](https://en.wikipedia.org/wiki/Time_Stamp_Counter)
- [sched_setaffinity](https://man7.org/linux/man-pages/man2/sched_setaffinity.2.html)

This code will probably only work on linux
Works with both -O0 -O2, but fails with -O3 sometimes

## Build & Run

``` bash
make
./main
```

