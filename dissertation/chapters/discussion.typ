#import "../lib.typ": code_block

= Discussion

== Benchmarks

A various number of benchmarks were run to compare the performance of the the allocators. The allocators that were used were as follows:
- dmalloc vs malloc
- dmalloc + malloc vs malloc

All benchmarks are seeded so that they can be replicated. The benchmarks are performed as follows:

=== Linear Allocation, Reallocation and Deallocation

1. This benchmark first allocates an $N$ number of $S$ sized objects
2. It then deallocates all the objects
3. It then reallocates all the objects
4. Finally it deallocates all the objects
The values of $N$ are as follows:
#align(
  center,
  table(
    columns: 10,
    [1000], [2000], [3000], [4000], [5000], [6000], [7000], [8000], [9000], [10000]
  )
)
The values for $S$ in bytes are:
#align(
  center,
  table(
    columns: 12,
    [1], [2], [4], [8], [16], [32], [64], [128], [256], [512], [1024], [2048]
  )
)

=== Sporadic Allocation and Deallocation

This benchmark will randomly perform $N$ allocations and deallocations at random of size $S$ bytes.
The values of $N$ are as follows:
#align(
  center,
  table(
    columns: 10,
    [1000], [2000], [3000], [4000], [5000], [6000], [7000], [8000], [9000], [10000]
  )
)
The values for $S$ in bytes are:
#align(
  center,
  table(
    columns: 12,
    [1], [2], [4], [8], [16], [32], [64], [128], [256], [512], [1024], [2048]
  )
)

=== Varying Allocation and Deallocation

This benchmark will randomly perform $N$ allocations and deallocations at random where the size of the allocation is chosen from range $R$ bytes.
The values of $N$ are as follows:
#align(
  center,
  table(
    columns: 10,
    [1000], [2000], [3000], [4000], [5000], [6000], [7000], [8000], [9000], [10000]
  )
)
The range for $R$ is $[1-4096]$ bytes.

=== Genetic Program

The internal workings of genetic programs have already been explained. The exact hyperparemetrs of this genetic program will be discussed below.

The genetic program was run 3 times each time with an increasing number of individuals in the population while keeping the number of generations the same. The goal the genetic algorithms was to approximate the following equation:

#code_block(
  "The target equation for the genetic program."
)[
  $
    f(x) = x^2 + 2x + 1
  $
]

The genetic program had access to the following terminal and non-terminal nodes:

#figure(
  caption: [Nodes the genetic program had access too.],
  table(
    columns: 2,
    [Terminal], [Non-terminal],
    [Number (Random number)], [Addition],
    [Variable (The inputted variable)], [Subtraction],
    [], [Multiplication],
    [], [Division],
    [], [Sin],
    [], [Cos],
  )
)

The difficulty of the problem is not what is being evaluated here thus even the result of the program does not matter as what is being evaluated is the effect of a different memory allocator on length of time taken to come to an answer even if it is incorrect.

The in code definition of a node is as follows:
#code_block(
  "How a node is defined in code."
)[```c
typedef enum {
    NODE_NUMBER,
    NODE_VARIABLE,
    NODE_ADD,
    NODE_SUB,
    NODE_MUL,
    NODE_DIV,
    NODE_SIN,
    NODE_COS
} NodeType;

// Expression tree node
typedef struct Node {
    NodeType type; // 1 byte
    double value; // 8 bytes
    struct Node *left; // 8 bytes
    struct Node *right; // 8 bytes
} Node;
```]

All in all the node structure is 25 bytes but due to how C's padding in structs work this will be padded with another 7 bytes for a total size of 32 bytes. This size does fit in the small object allocator and will be handled by the bin allocator. 

= Memory Allocator Benchmark Results

== Linux

=== Basic

#for size in ("1", "2", "4", "8", "16", "32", "64", "128", "256", "512", "1024", "2048") [
  #figure(
    image("../results/linux/memory/basic_size_"+size+"_memory.png"),
    caption: "Basic benchmark memory usage for size: " + size
  )

  #figure(
    image("../results/linux/time/basic_size_"+size+".0.png"),
    caption: "Basic benchark for size: "+ size
  )
]

=== Sporadic

#for size in ("1", "2", "4", "8", "16", "32", "64", "128", "256", "512", "1024", "2048") [
  #figure(
  image("../results/linux/memory/sporadic_size_"+size+"_memory.png"),
    caption: "Sporadic benchmark memory usage for size: " + size
  )

  #figure(
    image("../results/linux/time/sporadic_size_"+size+".0.png"),
    caption: "Sporadic benchark for size: "+ size
  )
]

=== Varying

#figure(
  image("../results/linux/memory/varying_size_0_memory.png"),
  caption: "Varying memory usage"
)

#figure(
  image("../results/linux/time/varying_size_varying.png"),
  caption: "Varying time usage"
)

=== Genetic Program

#figure(
  image("../results/linux/genetic/genetic_benchmark_mean_time.png"),
  caption: "Genetic program mean time"
)

== MacOS

=== Basic

#for size in ("1", "2", "4", "8", "16", "32", "64", "128", "256", "512", "1024", "2048") [
  #figure(
    image("../results/macos/time/basic_size_"+size+".0.png"),
    caption: "Basic benchark for size: "+ size
  )
]

=== Sporadic

#for size in ("1", "2", "4", "8", "16", "32", "64", "128", "256", "512", "1024", "2048") [
  #figure(
    image("../results/macos/time/sporadic_size_"+size+".0.png"),
    caption: "Sporadic benchark for size: "+ size
  )
]

=== Varying

#figure(
  image("../results/macos/time/varying_size_varying.png"),
  caption: "Varying time usage"
)

=== Genetic Program

#figure(
  image("../results/macos/genetic/genetic_benchmark_mean_time.png"),
  caption: "Genetic program mean time"
)
