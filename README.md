# map-xxhash
A simple resizable hash table in C, powered by the [xxHash](https://github.com/Cyan4973/xxHash) hash function.

## Dependencies

- CMake (>= 3.14)
- pkg-config
- libxxhash development library

On Debian/Ubuntu:
```bash
sudo apt-get install cmake pkg-config libxxhash-dev
```

## Build

```bash
cmake -B build
cmake --build build
```

The library (`libmap.a`) and test binaries are placed in the `build/` directory.

### Build without xxHash

To use a simple built-in hash function instead of xxHash:
```bash
cmake -B build -DUSE_SIMPLE_HASH=ON
cmake --build build
```

## Test

Run functional tests:
```bash
./build/test_functional
```

Run performance benchmarks:
```bash
./build/test_perf
```
