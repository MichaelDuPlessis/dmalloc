#!/bin/bash

# Define allocator-deallocator pairs
# Format: allocator:deallocator:define_only_small_flag
ALLOCATOR_PAIRS=(
  "dmalloc:dfree:no"    # Pure dmalloc
  "malloc:free:no"      # Pure malloc
  # "dmalloc:dfree:yes"   # Hybrid mode (ONLY_SMALL defined)
)

# Define benchmark types
BENCHMARKS=("basic" "sporadic" "varying")

# Allocation sizes to test (in bytes)
SIZES=(1 2 4 8 16 32 64 128 256 512 1024 2048)

# Accept total allocation count from CLI or default to 10000
TOTAL_AMOUNT=${1:-10000}
NUM_STEPS=10
STEP=$((TOTAL_AMOUNT / NUM_STEPS))

# Create output directory
mkdir -p ./results

# Ensure hyperfine is installed
if ! command -v hyperfine &> /dev/null; then
  echo "❌ 'hyperfine' is not installed. Install it to benchmark performance."
  exit 1
fi

# Loop through allocator configurations
for PAIR in "${ALLOCATOR_PAIRS[@]}"; do
  IFS=":" read -r ALLOCATOR DEALLOCATOR DEFINE_ONLY_SMALL <<< "$PAIR"

  # Add -DONLY_SMALL flag for hybrid mode
  EXTRA_FLAGS=""
  MODE_SUFFIX=""
  if [[ "$DEFINE_ONLY_SMALL" == "yes" ]]; then
    EXTRA_FLAGS="-DONLY_SMALL"
    MODE_SUFFIX="_onlysmall"
  fi

  echo "🔨 Compiling with ALLOCATOR=$ALLOCATOR, DEALLOCATOR=$DEALLOCATOR $EXTRA_FLAGS"

  clang -O3 \
    -DALLOCATOR="$ALLOCATOR" \
    -DDEALLOCATOR="$DEALLOCATOR" \
    $EXTRA_FLAGS \
    -o ./bench \
    src/*.c benchmark/*.c

  if [[ $? -ne 0 ]]; then
    echo "❌ Compilation failed for $ALLOCATOR and $DEALLOCATOR"
    continue
  fi

  # Loop through benchmarks
  for BENCHMARK in "${BENCHMARKS[@]}"; do
    for ((i = 1; i <= NUM_STEPS; i++)); do
      AMOUNT=$((STEP * i))

      if [[ "$BENCHMARK" == "varying" ]]; then
        CMD="./bench $BENCHMARK $AMOUNT"
        LABEL="${BENCHMARK}_${ALLOCATOR}${MODE_SUFFIX}_amount${AMOUNT}"
        echo "🚀 Benchmarking $LABEL"
        hyperfine --warmup 1 --export-csv "./results/${LABEL}.csv" --runs 10 -N "$CMD"
      else
        for SIZE in "${SIZES[@]}"; do
          CMD="./bench $BENCHMARK $AMOUNT $SIZE"
          LABEL="${BENCHMARK}_${ALLOCATOR}${MODE_SUFFIX}_amount${AMOUNT}_size${SIZE}"
          echo "🚀 Benchmarking $LABEL"
          hyperfine --warmup 1 --export-csv "./results/${LABEL}.csv" --runs 10 -N "$CMD"
        done
      fi
    done
  done
done
