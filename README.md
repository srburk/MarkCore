
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

## TODO:

[X] Fix image gen
[X] Add finish code for parser to free synax tree
[] Smart tree diff algorithm for renderer?
[X] Add state management for parser

[] Auto numbered list
[] Fix code blocks having HTML styling