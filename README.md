
## MarkCore

Why? - Feature complete + extensibility for plugins for grammar/spellcheck planned.

## Building

Building as static library:
```
mkdir build
cd build
cmake ..
make
```

Building with cli tool:
```
mkdir build
cd build
cmake .. -DMARKCORE_BUILD_CLI=ON
make
```

## Debugging Notes

Useful for watching for memory leaks
`sudo MallocStackLogging=1 leaks --atExit -- ./markcore-cli ../test.md` 

Enable address sanitizer with Cmake flag `-DADDRESS_SANITIZER=ON`
Enable debugging with flag `-DCMAKE_BUILD_TYPE=Debug`

## TODO:

[X] Fix image gen
[X] Add finish code for parser to free synax tree
[] Smart tree diff algorithm for renderer?
[X] Add state management for parser

[X] Auto numbered list
[X] Fix code blocks having HTML styling
[] Refactor to store string bounds in original buffer instead of copying every time