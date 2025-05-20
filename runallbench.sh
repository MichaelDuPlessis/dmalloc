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

# Accept total amount from CLI or default to 100
TOTAL_AMOUNT=${1:-10000}
# Always run 10 steps
NUM_STEPS=10
STEP=$((TOTAL_AMOUNT / NUM_STEPS))

# Create results directory
mkdir -p ./results

# Loop through combinations
for PAIR in "${ALLOCATOR_PAIRS[@]}"; do
  IFS=":" read -r ALLOCATOR DEALLOCATOR <<< "$PAIR"

  for BENCHMARK in "${BENCHMARKS[@]}"; do

    for ((i=1; i<=NUM_STEPS; i++)); do
      AMOUNT=$((STEP * i))

      if [[ "$BENCHMARK" == "varying_allocs" ]]; then
        echo "🔧 Running ALLOCATOR=$ALLOCATOR, BENCHMARK=$BENCHMARK, AMOUNT=$AMOUNT"

        clang -O3 \
          -DALLOCATOR="$ALLOCATOR" \
          -DDEALLOCATOR="$DEALLOCATOR" \
          -DBENCHMARK="$BENCHMARK" \
          -DAMOUNT=$AMOUNT \
          -o ./bench \
          src/*.c test/*.c benchmark/*.c

        if [[ $? -ne 0 ]]; then
          echo "❌ Compilation failed for $ALLOCATOR with $BENCHMARK (amount=$AMOUNT)"
          continue
        fi

        ./bench

      else
        for SIZE in "${SIZES[@]}"; do
          echo "🔧 Running ALLOCATOR=$ALLOCATOR, BENCHMARK=$BENCHMARK, SIZE=$SIZE, AMOUNT=$AMOUNT"

          clang -O3 \
            -DALLOCATOR="$ALLOCATOR" \
            -DDEALLOCATOR="$DEALLOCATOR" \
            -DBENCHMARK="$BENCHMARK" \
            -DSIZE=$SIZE \
            -DAMOUNT=$AMOUNT \
            -o ./bench \
            src/*.c test/*.c benchmark/*.c

          if [[ $? -ne 0 ]]; then
            echo "❌ Compilation failed for $ALLOCATOR with $BENCHMARK size=$SIZE amount=$AMOUNT"
            continue
          fi

          ./bench
        done
      fi

    done
  done
done
