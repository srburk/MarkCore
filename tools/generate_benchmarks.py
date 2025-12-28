import os

def generate_large(filename, size_mb=10):
    print(f"Generating {filename} (~{size_mb} MB)...")
    with open(filename, "w") as f:
        f.write("# Large Benchmark File\n\n")
        f.write("This file is generated to test performance.\n\n")

        target_size = size_mb * 1024 * 1024
        while f.tell() < target_size:
            f.write("## Section Header\n\n")
            f.write("Some regular text paragraph with *italics* and **bold** and [links](http://example.com).\n\n")
            f.write("* List item 1\n* List item 2\n* List item 3\n\n")
            f.write("1. Ordered item 1\n2. Ordered item 2\n\n")
            f.write("```\ncode block\n```\n\n")

def generate_deep(filename, depth=2000):
    print(f"Generating {filename} (depth={depth})...")
    with open(filename, "w") as f:
        f.write("# Deep Nesting Benchmark\n\n")
        for i in range(depth):
            f.write(" " * (i * 2) + "* Level " + str(i) + "\n")

def generate_long_line(filename, length=5000):
    print(f"Generating {filename} (line length={length})...")
    with open(filename, "w") as f:
        f.write("# Long Line Benchmark\n\n")
        f.write("Start " + "a" * length + " End\n")

if __name__ == "__main__":
    os.makedirs("benchmarks", exist_ok=True)
    generate_large("benchmarks/benchmark_large.md", size_mb=2) # 2MB is enough to show diff, 10MB might take too long in valgrind
    generate_deep("benchmarks/benchmark_deep.md", depth=500) # Start conservatively
    generate_long_line("benchmarks/benchmark_long_line.md", length=2048)
    print("Done.")
