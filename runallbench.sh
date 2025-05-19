#!/bin/bash

# Define allocator-deallocator pairs
ALLOCATOR_PAIRS=(
  "dmalloc:dfree"
  "malloc:free"
)

# Define benchmark functions
BENCHMARKS=("basic_allocs" "sporadic_allocs" "varying_allocs")

# Create results directory
mkdir -p ./results

# Loop through combinations
for PAIR in "${ALLOCATOR_PAIRS[@]}"; do
  IFS=":" read -r ALLOCATOR DEALLOCATOR <<< "$PAIR"

  for BENCHMARK in "${BENCHMARKS[@]}"; do
    echo "🔧 Building and running ALLOCATOR=$ALLOCATOR, BENCHMARK=$BENCHMARK"

    clang -O3 \
      -DALLOCATOR="$ALLOCATOR" \
      -DDEALLOCATOR="$DEALLOCATOR" \
      -DBENCHMARK="$BENCHMARK" \
      -o ./bench \
      src/*.c test/*.c benchmark/*.c

    if [[ $? -ne 0 ]]; then
      echo "❌ Compilation failed for $ALLOCATOR with $BENCHMARK"
      continue
    fi

    ./bench
  done
done
