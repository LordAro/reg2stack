reg2stack
=========

An interpreter that can produce stack code for the [J5](https://www-users.cs.york.ac.uk/~chrisb/main-pages/dacs-temp/S08-Addressing%20Modes%20-%20Memory.pdf.1) stack machine from register code for the [DCPU-16](https://raw.githubusercontent.com/gatesphere/demi-16/master/docs/dcpu-specs/dcpu-1-7.txt) and run it.

This project was the implementation part of my final year project at the [University of York](https://www.york.ac.uk) as part of my Computer Science degree course.

Compilation
-----------

Requires a C++ compiler that supports C++14, and a version of the Boost library installed (only headers required). Python3 is also required for running the tests.

From there, navigate to the checked out directory, and run `make`. This will produce a `reg2stack` binary in the current directory.


Running
-------

### Example programs

There are several example test programs in the examples/ directory. As yet the only notable program is bsort, an implementation of bubble sort.

### Test suite

Very basic test suite is implemented, run `make test` to check that the register and the stack outputs match.

### Example usage

Running `./reg2stack -vc examples/bsort.reg` will run the bsort register code on the J5 emulator.

This will result in a lot of output. If you get rid of the `-v` flag and ignore stderr (e.g. append `2> /dev/null` to the command), you will only get the actual output from the stack machine.

### Command line flags

    Usage: ./reg2stack [-v] [-f] [-scr] file

* `-v`:  Verbose output
* `-f`:  Unlimited clock speed
* `-c`:  Convert register code
* `-s`:  Stack (J5) interpreter
* `-r`:  Register (DCPU-16) interpreter

### Known bugs

How the stack scheduler determines blocks is very stupid, as it just splits
based on labels. Therefore, in the case that a loop doesn't have a determined
"end" point specified with a label, the optimiser will think that the rest of
the program is a single block, i.e. including the stuff outside the loop
definition. A workaround is to add an (unused) label at the end of the loop
"block"
