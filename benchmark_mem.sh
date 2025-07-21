#!/bin/bash

# Memory usage benchmark for artificial benchmarks (basic, sporadic, varying)
# using dmalloc, malloc, and hybrid mode with ONLY_SMALL macro.
# Measures max resident set size with GNU /usr/bin/time.

ALLOCATOR_PAIRS=(
  "dmalloc:dfree:no"
  "malloc:free:no"
  "dmalloc:dfree:yes"
)

BENCHMARKS=("basic" "sporadic" "varying")

SIZES=(1 2 4 8 16 32 64 128 256 512 1024 2048)

TOTAL_AMOUNT=${1:-10000}
NUM_STEPS=10
STEP=$((TOTAL_AMOUNT / NUM_STEPS))

# Check GNU time presence
if ! command -v /usr/bin/time &> /dev/null; then
  echo "❌ GNU time (/usr/bin/time) is not installed."
  exit 1
fi

mkdir -p ./results/artificial_memory

echo "🧪 Starting artificial benchmark memory tests..."

for PAIR in "${ALLOCATOR_PAIRS[@]}"; do
  IFS=":" read -r ALLOCATOR DEALLOCATOR DEFINE_ONLY_SMALL <<< "$PAIR"

  EXTRA_FLAGS=""
  LABEL="$ALLOCATOR"
  if [[ "$DEFINE_ONLY_SMALL" == "yes" ]]; then
    EXTRA_FLAGS="-DONLY_SMALL"
    LABEL="${ALLOCATOR}_onlysmall"
  fi

  echo ""
  echo "🔨 Compiling $LABEL ..."
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

  for BENCHMARK in "${BENCHMARKS[@]}"; do
    for ((i=1; i<=NUM_STEPS; i++)); do
      AMOUNT=$((STEP * i))

      if [[ "$BENCHMARK" == "varying" ]]; then
        CMD="./bench $BENCHMARK $AMOUNT"
        LABEL_RUN="${BENCHMARK}_${LABEL}_amount${AMOUNT}"

        echo ""
        echo "📊 Memory test: $LABEL_RUN"
        MEMORY_LOG="./results/artificial_memory/memory_${LABEL_RUN}.log"
        /usr/bin/time -v $CMD 2> "$MEMORY_LOG"

        MAX_RSS=$(grep "Maximum resident set size" "$MEMORY_LOG" | awk -F: '{print $2}' | tr -d ' ')
        echo "Max Resident Set Size (KB): $MAX_RSS"

        # Append to CSV summary
        CSV_FILE="./results/artificial_memory/memory_summary.csv"
        if [ ! -f "$CSV_FILE" ]; then
          echo "benchmark,label,amount,0,max_rss_kb" > "$CSV_FILE"
        fi
        echo "${BENCHMARK},${LABEL},${AMOUNT},${MAX_RSS}" >> "$CSV_FILE"

      else
        for SIZE in "${SIZES[@]}"; do
          CMD="./bench $BENCHMARK $AMOUNT $SIZE"
          LABEL_RUN="${BENCHMARK}_${LABEL}_amount${AMOUNT}_size${SIZE}"

          echo ""
          echo "📊 Memory test: $LABEL_RUN"
          MEMORY_LOG="./results/artificial_memory/memory_${LABEL_RUN}.log"
          /usr/bin/time -v $CMD 2> "$MEMORY_LOG"

          MAX_RSS=$(grep "Maximum resident set size" "$MEMORY_LOG" | awk -F: '{print $2}' | tr -d ' ')
          echo "Max Resident Set Size (KB): $MAX_RSS"

          # Append to CSV summary
          CSV_FILE="./results/artificial_memory/memory_summary.csv"
          if [ ! -f "$CSV_FILE" ]; then
            echo "benchmark,label,amount,size,max_rss_kb" > "$CSV_FILE"
          fi
          echo "${BENCHMARK},${LABEL},${AMOUNT},${SIZE},${MAX_RSS}" >> "$CSV_FILE"
        done
      fi
    done
  done
done

echo ""
echo "✅ Artificial benchmarks memory testing complete!"
echo "Results saved in ./results/artificial_memory/"
