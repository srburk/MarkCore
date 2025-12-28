#!/bin/bash

# Build the CLI first (ensure it's up to date)
cd build-cli
make -j4 > /dev/null
cd ..

MARKCORE=./build-cli/markcore-cli

echo "========================================"
echo "Baseline Measurements"
echo "========================================"

run_test() {
    FILE=$1
    NAME=$2
    echo "----------------------------------------"
    echo "Testing $NAME ($FILE)"

    # Measure time
    echo "Time:"
    /usr/bin/time -f "%e seconds" $MARKCORE $FILE > /dev/null

    # Measure memory with valgrind (massif if available, else standard memcheck)
    # Using simple Valgrind memcheck to see allocations and leaks summary
    echo "Memory & Leaks:"
    valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes $MARKCORE $FILE > /dev/null 2> valgrind_out.txt

    grep "total heap usage" valgrind_out.txt
    grep "definitely lost" valgrind_out.txt
    grep "indirectly lost" valgrind_out.txt

    # Check for specific errors in stderr (like Line too long)
    $MARKCORE $FILE > /dev/null 2> stderr_out.txt
    if [ -s stderr_out.txt ]; then
        echo "Stderr output:"
        cat stderr_out.txt
    fi
}

run_test "test.md" "Small Test"
run_test "benchmarks/benchmark_large.md" "Large File (2MB)"
run_test "benchmarks/benchmark_long_line.md" "Long Line"
# run_test "benchmarks/benchmark_deep.md" "Deep Nesting" # Careful with this one
