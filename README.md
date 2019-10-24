pmt
===
Pattern Matching Tool

Algorithms
----------
- `bf`: Brute-force
- `ac`: Aho-Corasick (1975)
- `bm`: Boyer-Moore (1977)
- `uk`: Ukkonen (1985)
- `so`: Shift-Or (Baeza-Yates–Gonnet, 1992)
- `wm`: Wu-Manber (1992)

Build
-----
To build make sure you have `gcc` and `make` installed:
```shell script
make
```

Running
-------
`Usage: pmt [-a ALGO] [-c] [-e DIST] [-h] (PATTERN | -p PATTERN_FILE) FILE [FILE...]`

Execute the program passing as arguments the pattern and the files to search.
Run  `./bin/pmt --help` to see the whole list of arguments.
```shell script
./bin/pmt Romeo shakespeare.txt
```
