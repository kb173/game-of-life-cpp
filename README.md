# Game of Life

Conway's Game of Life implemented efficiently in C++.

The input file must contain the number of columns and rows to expect. Live cells are marked as `x`, dead cells are `.`. For example:

```
3,2
...
.x.
```

## Build & Run

Build with `make`.

Run with `./gol --load infile.gol --save outfile.gol --generations 100`. Optionally, also print a time measurement to stdout with `--measure`.
