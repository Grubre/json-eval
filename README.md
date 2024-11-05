# Json eval
A tool for evaluating expressions on a JSON file.

## Usage
For the build instructions go to the [Build](#Build) section.
The program can be run as follows:
```
json-eval <path_to_json_file> <query>
```
The program will parse the given JSON file and run the query, printing out the evaluated JSON value.

### Queries
The queries follow a simple syntax.
#### Indexing object members
Use `.` to index into object members. For a JSON file: `{ "a": { "b": 1 } }`, the query `a.b` will return 1.
#### Indexing into arrays
Use `[]` to index into arrays, i.e. `a[5]`. You can also use expressions that evaluate to an integer as the index.
For example, the JSON file: `{ "array": [1,[2],3], "b": { "c": 1" } }`, the query `array[b.c]` will return [2].
#### Intrinsic function
- `size()` - takes either an array or an object. For the object returns the number of keys, for the array - number of elements.
- `max()` - takes either an array or a variadic number of doubles/integers. Returns the maximum.
- `min()` - takes either an array or a variadic number of doubles/integers. Returns the minimum.
- `sum()` - takes either an array or a variadic number of doubles/integers. Returns the sum.
- `product()` - takes either an array or a variadic number of doubles/integers. Returns the product.
#### Mathematical operations
Supported binary operations `lhs (+|-|*|/) rhs` as well as the unary minus `-expression`. Grouping is also supported,
`2 * 2 + 2` will evaluate to `6` but `2 * (2 + 2)` to 8.

## Build

### CMake
The project can be built using CMake:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```
The binary is in `build/src/json-eval`.
To run the tests, go to `build` and run `ctest`.

### Just command runner
If you have the [just](https://github.com/casey/just) command runner installed, there are a few recipes available:
```C
Available recipes:
    build BUILD_TYPE="Debug"
    clean
    debug *ARGS
    default
    run *ARGS
    test *ARGS
```
