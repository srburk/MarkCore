
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

## Performance Testing

This repository includes tools to benchmark memory usage and performance.

### Prerequisites: Valgrind
To run the full test suite, you need `valgrind`.
* **Linux**: [Installation Guide](https://valgrind.org/docs/manual/quick-start.html) (usually `sudo apt install valgrind`)
* **macOS**: [Installation Guide](https://github.com/LouisBrunner/valgrind-macos) (install via Homebrew: `brew tap LouisBrunner/valgrind && brew install --HEAD LouisBrunner/valgrind/valgrind`)

### Running Benchmarks
1. Build the CLI tool (see above).
2. Generate benchmark files:
   ```bash
   python3 tools/generate_benchmarks.py
   ```
3. Run the measurement script:
   ```bash
   ./tools/measure.sh
   ```
   This will run `markcore-cli` against various benchmarks and report time and memory statistics.

## Debugging Notes

Useful for watching for memory leaks
`sudo MallocStackLogging=1 leaks --atExit -- ./markcore-cli ../test.md` 

Enable address sanitizer with Cmake flag `-DADDRESS_SANITIZER=ON`
Enable debugging with flag `-DCMAKE_BUILD_TYPE=Debug`

## Plugin System

* Pre/Post tree traversal for spelling check, maybe markdown suggestions?

## TODO:

[X] Fix image gen
[X] Add finish code for parser to free synax tree
[] Smart tree diff algorithm for renderer?
[X] Add state management for parser

[] Fill out public interface (output options, cli flags, etc.)

[X] Auto numbered list
[X] Fix code blocks having HTML styling
[X] Refactor to store string bounds in original buffer instead of copying every time
