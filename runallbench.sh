#!/bin/bash

# Define allocator-deallocator pairs
ALLOCATOR_PAIRS=(
  "dmalloc:dfree"
  "malloc:free"
)

# Define benchmark functions
BENCHMARKS=("basic_allocs" "sporadic_allocs" "varying_allocs")

# The sizes to test on (in bytes)
SIZES=(1 2 4 8 16 32 64)

# Create results directory
mkdir -p ./results

# Loop through combinations
for PAIR in "${ALLOCATOR_PAIRS[@]}"; do
  IFS=":" read -r ALLOCATOR DEALLOCATOR <<< "$PAIR"

  for BENCHMARK in "${BENCHMARKS[@]}"; do

    # Run varying_allocs only once per pair
    if [[ "$BENCHMARK" == "varying_allocs" ]]; then
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

    else
      # Loop over sizes for other benchmarks
      for SIZE in "${SIZES[@]}"; do
        echo "🔧 Building and running ALLOCATOR=$ALLOCATOR, BENCHMARK=$BENCHMARK, SIZE=$SIZE"

        clang -O3 \
          -DALLOCATOR="$ALLOCATOR" \
          -DDEALLOCATOR="$DEALLOCATOR" \
          -DBENCHMARK="$BENCHMARK" \
          -DSIZE=$SIZE \
          -o ./bench \
          src/*.c test/*.c benchmark/*.c

        if [[ $? -ne 0 ]]; then
          echo "❌ Compilation failed for $ALLOCATOR with $BENCHMARK size $SIZE"
          continue
        fi

        ./bench
      done
    fi

  done
done
