_________________________README.md__________________________
________________DISASSEMBLER IMPLEMENTATION_________________

_DESCRIPTION_:
This program takes in a file consisting of instructions
encoded in hexadecimal and converts it to instructions from 
the RISC-V standard instruction set.

The source code *dis.cpp* is written in C++, implemented
using *iostream*, *fstream*, *string* and *vector* libraries.

_EXECUTING THE PROGRAM_:
To run the program, one can compile the source code *dis.cpp*
using one of the following commands:
-> *g++ dis.cpp* (The executable by default is *a.out*)
-> *g++ dis.cpp -o dis* (The executable is *dis*)

You can run the executable along with the input file *inFile*
as follows:
-> *./a.out inFile*
-> *./dis inFile*

Note that only one file can be given as input at a time.

_NOTES_:
-> The code consists of ANSI Color Codes which may differ or
in some cases may raise exceptions while execution. In most
cases, this should not affect the actual output.
-> The code could be vastly improved by the use of libraries
like *map*, and could have more modularity by commonly used
functions.

_CREDIT_:
This program is the final Lab assignment of the course CS2323
- Computer Architecture (Fall, 2022) offered by Dr. Rajesh 
Kedia.

Author: Rishit D (cs21btech11053)
GitHub ID: purplehand52
