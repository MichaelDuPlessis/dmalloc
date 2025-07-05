#!/bin/bash

# Define allocator-deallocator pairs
ALLOCATOR_PAIRS=(
  "dmalloc:dfree"
  "malloc:free"
)

# Define benchmark functions
BENCHMARKS=("basic" "sporadic" "varying")

# The sizes to test on (in bytes)
SIZES=(1 2 4 8 16 32 64 128 256 512 1024 2048)

# Accept total amount from CLI or default to 10000
TOTAL_AMOUNT=${1:-10000}
# Always run 10 steps
NUM_STEPS=10
STEP=$((TOTAL_AMOUNT / NUM_STEPS))

# Create results directory
mkdir -p ./results

# Check if hyperfine is installed
if ! command -v hyperfine &> /dev/null; then
  echo "❌ 'hyperfine' is not installed. Install it to benchmark performance."
  exit 1
fi

# Loop through combinations
for PAIR in "${ALLOCATOR_PAIRS[@]}"; do
  IFS=":" read -r ALLOCATOR DEALLOCATOR <<< "$PAIR"

  echo "🔨 Compiling with ALLOCATOR=$ALLOCATOR, DEALLOCATOR=$DEALLOCATOR"
  clang -O3 \
    -DALLOCATOR="$ALLOCATOR" \
    -DDEALLOCATOR="$DEALLOCATOR" \
    -o ./bench \
    src/*.c test/*.c benchmark/*.c

  if [[ $? -ne 0 ]]; then
    echo "❌ Compilation failed for $ALLOCATOR and $DEALLOCATOR"
    continue
  fi

  for BENCHMARK in "${BENCHMARKS[@]}"; do
    for ((i=1; i<=NUM_STEPS; i++)); do
      AMOUNT=$((STEP * i))

      if [[ "$BENCHMARK" == "varying" ]]; then
        CMD="./bench $BENCHMARK $AMOUNT"
        LABEL="${BENCHMARK}_${ALLOCATOR}_amount${AMOUNT}"
        echo "🚀 Benchmarking $LABEL"
        hyperfine --warmup 1 --export-csv "./results/${LABEL}.csv" --runs 10 -N "$CMD"
      else
        for SIZE in "${SIZES[@]}"; do
          CMD="./bench $BENCHMARK $AMOUNT $SIZE"
          LABEL="${BENCHMARK}_${ALLOCATOR}_amount${AMOUNT}_size${SIZE}"
          echo "🚀 Benchmarking $LABEL"
          hyperfine --warmup 1 --export-csv "./results/${LABEL}.csv" --runs 10 -N "$CMD"
        done
      fi
    done
  done
done
