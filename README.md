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
* [<some_commit_link>]()
