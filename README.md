# TreeLib
library for tree structures written in C
- isolated allocator for each tree structures, such as fsalloc .etc
- customized compare logic and copy logic for any type of data

## Deps
- [aLocas](https://github.com/9io9/aLocas)
- [seqlib](https://github.com/9io9/seqlib)

## Getting Started

```bash
> mkdir -p build
> cd build
> cmake .. -DaLocas_DIR=absolute/path/of/aLocas -Dseqlib_DIR=absolute/path/of/seqlib
> make
... # wait for compilation
> cd ..
> cmake --install build --prefix path/of/install
```