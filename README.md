## Cabaliser

A graph state compiler written in C.

(Name is pending review and new suggestions are more than welcome)

Most of what you want is in the `c_lib` directory
The Python side of the code provides a thin wrapper around the C code.


## Dependencies: ##

- Compiler is assumed to be `gcc`, if you are compiling on a Mac you may discover that Apple has helpfully aliased `gcc` to `clang`.
- Various operations assume that your CPU supports `bmi2` and `avx2` instructions along with some collection of atomic operations. M1 family chipsets with rosetta support may not satisfy this.
- `postgresql-libs, libpqxx` or equivalent packages that provides libqp headers for pandora integration
- `rustc` is required to compile the Pauli Tracker library  
- Python should be at least 3.10
  - numpy is required
  - pytest is reccomended


## Installation ##

To build the C library:
- `make`, this should build the shared object library 
- `make test` will build the tests, which are stored in the `tests` directory.
- `make benchmark` will build some seeded random benchmarks, that are stored in the `benchmarks` directory
- `make paulitracker` will build just the pauli tracker
- `make pandora` will build the pandora integration objects, I'm not yet happy with the stability of these so they are currently an optional build

To build the Python wrapper:
- `pip install -e .`

TODO: Invoke make from buildtools 

To compile the Pandora integration you will need the appropriate package that provides the `libpq-fe.h` headers for your system.
On arch this is `postgresql-libs`.

## Known Issues ## 

Python pre-3.10 handled class methods, functions and decorator methods differently, as a result Python >= 3.10 is required.

Python's ctypes casting has edge cases that cast objects to `int32`, which may result in segmentation faults when trying to use ctypes pointers with other library functions.



