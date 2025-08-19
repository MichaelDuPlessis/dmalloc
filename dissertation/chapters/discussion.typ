#import "../lib.typ": code_block

= Discussion

The following section discusses the results of the benchmarks on the memory allocator. The benchmarks were run both on a Linux system and a MacOS system to identify
how the allocator performed on different operating systems. The specifications of the machiens tested are listed below. The Clang compiler was used for both operating systems.

#figure(
  caption: [Specifications of the machines on which the benchmarks were run.],
  table(
    columns: 3,
    [*Operating System*], [Ubuntu 24.0.4.3 (Linux)], [MacOS 15.6 Sequoia],
    [*Architecture*], [x86], [Arm],
    [*CPU*], [Ryzen 5600x 6 cores \@3.7 GHz max clock, 12 threads], [M3 4 performance cores \@4.05 Ghz max clock, 4 efficeincy cores \@2.75 GHz max clock, 8 threads],
    [*Total CPU Cache*], [35 MB], [20 MB],
    [*RAM*], [32 GB \@3600 MHz], [16 GB \@6400 MHz],
    [*Page Size*], [4 KB], [16 KB],
    [*Malloc Implmentation*], [glibc malloc], [magazine malloc],
  )
)

For the purposes of these results the default memory allocator used by the operationg system will be known as `malloc` while the newly developed implementation
will be known as `dmalloc`.

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

== Memory Allocator Benchmark Results

=== Linux

==== Basic

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

Looking at the benchmarks its clear that dmalloc outperforms malloc on smaller sizes. dmalloc has a noticable lead 16 bytes where it is most of the time outperforming malloc
with some spikes where malloc will outperform it but this is not the norm. Starting at 32 bytes the gap between the two allocators narrows but dmalloc is still superior in
terms of speed. As allocation sizes approach 256 bytes the gab narrows until finally malloc is superior. When it comes to memory usage dmalloc is slightly better over malloc
until again 256 byte sized allocations in which malloc starts to outperform dmalloc. It must be stated that dmalloc's small memory allocator is only active up until 128 bytes
afterwards a different allocation strategy is used.

==== Sporadic

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

The performance time wise between malloc and dmalloc is nearly identical (barring for the size class 2048 bytes) which shows when compared to the basic benchmark that
malloc performs better for linear allocations for sizes 128 bytes and smaller. However dmalloc does perform worse when it comes to total amount of memory used across every
version of the benchmark. This benchmark seems to indicate that dmalloc is not well suited to tasks where allocations are all over the place.

==== Varying

#figure(
  image("../results/linux/memory/varying_size_0_memory.png"),
  caption: "Varying memory usage"
)

#figure(
  image("../results/linux/time/varying_size_varying.png"),
  caption: "Varying time usage"
)

The varying benchmark is biased towards dmalloc as dmalloc focuses on small objects while varying allocates objects of many different sizes which can mean that other weaker
allocation strategies can be used. It is however useful to see how dmalloc would perform if just arbitraly used instead of focusing on its strongs points.

==== Genetic Program

#figure(
  image("../results/linux/genetic/genetic_benchmark_mean_time.png"),
  caption: "Genetic program mean time"
)

This benchmark shows how a real world algorithm would perform. On linux dmalloc performs worse than malloc on versions of the benchmark (small, large, long). The gap also
grows as the number of iterations increase. This algorithm creates lots of trees and perhaps accessing the memory is slower using dmalloc when compared to malloc despite
dmallocs cache locality advantages.

=== MacOS

==== Basic

#for size in ("1", "2", "4", "8", "16", "32", "64", "128", "256", "512", "1024", "2048") [
  #figure(
    image("../results/macos/time/basic_size_"+size+".0.png"),
    caption: "Basic benchark for size: "+ size
  )
]

The same that was said about the Linux benchmark could be mentioned here except that the graphs do appear to be more sporadic this is maybe due to how MacOS schedules
processing time when compared to Linux or could be due to the difference in architecture (x86 vs ARM). A strong possiblity for the odd spikes could be due to MacOS moving
the application between performance and efficiency cores. Both dmalloc and malloc are sporadic so it the cause does not appear
to be with the implementation of either allocator. Overall dmalloc is faster and by larger margins at times then when compared to the linux benchmark. The same thing occurs after
128 bytes where dmalloc starts slowing down when compared to malloc due to the subpar allocation strategy which is used.

==== Sporadic

#for size in ("1", "2", "4", "8", "16", "32", "64", "128", "256", "512", "1024", "2048") [
  #figure(
    image("../results/macos/time/sporadic_size_"+size+".0.png"),
    caption: "Sporadic benchark for size: "+ size
  )
]

Both dmalloc and malloc are mirror images of one another and there is no significant difference between the two however unlike on linux this remains the case as the sizes
increase showing that there is no real difference between the two.

==== Varying

#figure(
  image("../results/macos/time/varying_size_varying.png"),
  caption: "Varying time usage"
)

The varying benchmark has the same results as it did on Linux and what was stated there can be applied exactly here.

==== Genetic Program

#figure(
  image("../results/macos/genetic/genetic_benchmark_mean_time.png"),
  caption: "Genetic program mean time"
)

This is where the benchmark results get interesting as now the reverse of what happened on Linux is happening here. Now dmalloc is the superior memory allocator with a growing
lead as the number of iterations increase. This is most likely due to the increased page size which is four times what it is on linux. Other factors may include a weaker memory
allocator when compared to Linux but this unlikely as the artificial benchmarks are similar.
