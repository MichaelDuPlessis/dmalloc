#!/bin/bash

# Define allocator-deallocator pairs
ALLOCATOR_PAIRS=(
  "dmalloc:dfree"
  "malloc:free"
)

# Define benchmark functions
BENCHMARKS=("basic" "sporadic" "varying")

# Allocation sizes to test (in bytes)
SIZES=(1 2 4 8 16 32 64 128 256 512 1024 2048)

# Total allocations from CLI or default to 10000
TOTAL_AMOUNT=${1:-10000}
NUM_STEPS=10
STEP=$((TOTAL_AMOUNT / NUM_STEPS))

# Output directory
mkdir -p ./results

# Check for valgrind
if ! command -v valgrind &> /dev/null; then
  echo "❌ 'valgrind' is not installed. Please install it to benchmark memory."
  exit 1
fi

# Loop over allocators
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
        echo "📦 Running massif for $LABEL"
        valgrind --tool=massif --massif-out-file="./results/massif_${LABEL}.out" $CMD
      else
        for SIZE in "${SIZES[@]}"; do
          CMD="./bench $BENCHMARK $AMOUNT $SIZE"
          LABEL="${BENCHMARK}_${ALLOCATOR}_amount${AMOUNT}_size${SIZE}"
          echo "📦 Running massif for $LABEL"
          valgrind --tool=massif --massif-out-file="./results/massif_${LABEL}.out" $CMD
        done
      fi
    done
  done
done

