The program (by Piotr Beling) for benchmarking the succinct data structures contained in the [SDSL](https://github.com/simongog/sdsl-lite) library.

It mimics (by the way it draws data, measures time, etc.) the cseq_benchmark program included in [BSuccinct](https://github.com/beling/bsuccinct-rs).
Consequently, the results obtained by the two programs can be compared.

The program can be built using [CMake](https://cmake.org/). Its release version can be built as follows:
```
mkdir Release
cd Release
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
The release build type enables native optimizations.
