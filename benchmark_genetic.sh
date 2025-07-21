#!/bin/bash

# Benchmark script for genetic programming using dmalloc, malloc, and hybrid mode
# All executables compiled as "bench", benchmarked immediately, then replaced

# Define allocator-deallocator pairs
# Format: allocator:deallocator:define_only_small_flag
ALLOCATOR_PAIRS=(
  "dmalloc:dfree:no"
  "malloc:free:no"
  "dmalloc:dfree:yes"
)

# Ensure hyperfine is installed
if ! command -v hyperfine &> /dev/null; then
  echo "❌ 'hyperfine' is not installed. Please install it to run benchmarks."
  exit 1
fi

# Prepare results directory
mkdir -p ./results/genetic

# Test cases: (name, generations, population)
TEST_CASES=(
  "small:50:50"
  "large:50:500"
  "long:50:1000"
)

SEED=42

echo "🛠️  Starting genetic programming benchmarks..."

for PAIR in "${ALLOCATOR_PAIRS[@]}"; do
  IFS=":" read -r ALLOCATOR DEALLOCATOR DEFINE_ONLY_SMALL <<< "$PAIR"

  EXTRA_FLAGS=""
  LABEL="$ALLOCATOR"
  if [[ "$DEFINE_ONLY_SMALL" == "yes" ]]; then
    EXTRA_FLAGS="-DONLY_SMALL"
    LABEL="${ALLOCATOR}_onlysmall"
  fi

  echo ""
  echo "🔨 Compiling with ALLOCATOR=$ALLOCATOR, DEALLOCATOR=$DEALLOCATOR, ONLY_SMALL=$DEFINE_ONLY_SMALL..."
  clang -O3 \
    -DALLOCATOR="$ALLOCATOR" \
    -DDEALLOCATOR="$DEALLOCATOR" \
    $EXTRA_FLAGS \
    -o bench \
    src/*.c benchmark/*.c -lm

  if [[ $? -ne 0 ]]; then
    echo "❌ Compilation failed for $ALLOCATOR"
    exit 1
  fi

  for TEST in "${TEST_CASES[@]}"; do
    IFS=":" read -r TEST_NAME GENERATIONS POPULATION <<< "$TEST"
    echo ""
    echo "🧪 Running test '$TEST_NAME' ($GENERATIONS generations, $POPULATION individuals) with $LABEL"
    echo "------------------------------------------------------------"

    CMD="./bench genetic $GENERATIONS $POPULATION $SEED"

    # Run benchmark with hyperfine, export results to unique CSV per allocator and test
    hyperfine --warmup 1 --export-csv "./results/genetic/genetic_${TEST_NAME}_${LABEL}.csv" --runs 10 -N "$CMD"
  done
done

echo ""
echo "✅ Benchmarking complete!"
echo "Results saved in ./results/genetic/"
