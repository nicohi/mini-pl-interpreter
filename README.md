# mini-pl-interpreter
An interpreter for a simple Pascal based programming language. Made for the course Compilers (CSM14204). 

## Building
cmake and gcc are required to compile the interpreter.
To build the project run:
`./build.sh`

## Usage
Any valid minipl program can be interpreted with:
`./build/mini-pl [filename].`
For testing purposes, the user can use
`./build/mini-pl -s [filename]`
to run merely the scanner, or 
`./build/mini-pl -p [filename]`
to run the scanner+parser.
Both commands print a readable result.

Example programs are provided in `./test/`.
