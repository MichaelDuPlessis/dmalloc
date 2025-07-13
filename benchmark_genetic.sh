#!/bin/bash

# Genetic Programming Benchmark Script
# Compares dmalloc vs standard malloc performance

echo "Building genetic programming benchmarks..."

# Build with dmalloc
echo "Building with dmalloc..."
clang -O3 -o ./bench_dmalloc src/*.c test/*.c benchmark/*.c -lm

# Build with standard malloc
echo "Building with standard malloc..."
clang -O3 -DALLOCATOR=malloc -DDEALLOCATOR=free -DNAME=malloc \
      -o ./bench_malloc src/*.c test/*.c benchmark/*.c -lm

echo ""
echo "Running Genetic Programming Benchmarks"
echo "======================================"

# Test parameters
GENERATIONS=30
POPULATION=50
SEED=42

echo ""
echo "Test 1: Small population (30 generations, 50 individuals)"
echo "--------------------------------------------------------"

echo "dmalloc:"
time ./bench_dmalloc genetic $GENERATIONS $POPULATION $SEED dmalloc

echo ""
echo "malloc:"
time ./bench_malloc genetic $GENERATIONS $POPULATION $SEED malloc

echo ""
echo "Test 2: Larger population (20 generations, 100 individuals)"
echo "----------------------------------------------------------"

GENERATIONS=20
POPULATION=100

echo "dmalloc:"
time ./bench_dmalloc genetic $GENERATIONS $POPULATION $SEED dmalloc

echo ""
echo "malloc:"
time ./bench_malloc genetic $GENERATIONS $POPULATION $SEED malloc

echo ""
echo "Test 3: Many generations (50 generations, 30 individuals)"
echo "--------------------------------------------------------"

GENERATIONS=50
POPULATION=30

echo "dmalloc:"
time ./bench_dmalloc genetic $GENERATIONS $POPULATION $SEED dmalloc

echo ""
echo "malloc:"
time ./bench_malloc genetic $GENERATIONS $POPULATION $SEED malloc

echo ""
echo "Benchmark complete!"
echo "The genetic programming algorithm creates many small tree nodes,"
echo "making it ideal for testing small object allocators."
