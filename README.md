The program (by Piotr Beling) for benchmarking the succinct data structures contained in the [SDSL2](https://github.com/simongog/sdsl-lite) library.

It mimics (by the way it draws data, measures time, etc.) the [cseq_benchmark](https://crates.io/crates/cseq_benchmark) program included in [BSuccinct](https://github.com/beling/bsuccinct-rs).
Consequently, the results obtained by the two programs can be compared.

The program can be built using [CMake](https://cmake.org/). Its release version can be built as follows:
``` shell
mkdir Release
cd Release
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
Notes:
- release compilation type enables optimizations native to the CPU used for compilation,
- the software and instructions have been tested under GNU/Linux and may require some modifications for other systems.

