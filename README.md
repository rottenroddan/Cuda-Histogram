# Histogram Performance Tuning
___

### About
This project was a personal reintroduction into profiling and 
performance tuning algorithms in C++. The project is a few different implementations of 
a histogram algorithm, with their respective implementations to compare performance 
across different sized arrays with different bin values – exploring cache effects, threading, atomics
etc. Most of the profiling test sizes are powers of 2, and are probably skipping
a few interesting cases, as this isn't for a real use case, so profiling all 
edge cases is probably not worth it.
___

### Plans for the future
* Implement some CUDA kernels to benchmark, analyze, compare and improve.
* Investigate further HOST related speedup oppurtunities.

___
### Analysis
* [0v1-147054a](https://github.com/rottenroddan/Cuda-Histogram/commit/147054a26fcbe33803f629c80622580fef28ce83) Initial
  implementation of a thread pool that can handle type erased lambdas. Looking back at it, it seems that some tests were
  severely impacted by this, noticing a near 2x decrease in performance in some cases. This is probably because
  the lambda is not allowing the compiler to aggressively optimize the code. Below is a table of the results compared from
  previous non-thread pool implementations and the type erasure thread pool implementation. Each algorightm is run 10 times
  and the total time is taken.

| Version               | Test                                                                | Threads | Bin Size | Elements(int) | Time (ms) |
|-----------------------|---------------------------------------------------------------------|---------|----------|---------------|-----------|
| Non-Thread Pool Impl. | Threaded Reduction CPU-Histogram With Unrolling                     | 32      | 4096     | 1000000000    | 826.008   |
| Thread Pool Impl.     | Threaded Reduction CPU-Histogram With Unrolling                     | 32      | 4096     | 1000000000    | 1824.774  |
| Non-Thread Pool Impl. | Threaded Reduction CPU-Histogram With Explicit PreFetch & Unrolling | 32      | 4096     | 1000000000    | 973.519   |
| Thread Pool Impl.     | Threaded Reduction CPU-Histogram With Explicit PreFetch & Unrolling | 32      | 4096     | 1000000000    | 2701.186  |

