# `Copied from private git server at Chalmers; I'm still the author :)`

# Javalette Compiler

By Samuel Kajava, group 20.

## Project structure

- [`/src`](./src) - all the source code
- [`/lib`](./lib) - runtimes for assignments B and C
- [`/doc`](./doc) - explanations and specifications

## Instructions

```bash
make                    # build the compiler
./jlc path/to/<file>.jl # compile a file
```

#### Set up `.clangd`:

```bash
# cd to project root
echo "CompileFlags:" > .clangd
echo "  Add: ['-I$(pwd)/src']" >> .clangd
```
