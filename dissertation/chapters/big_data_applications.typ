#import "../lib.typ": code_block

= Big Data Science Applications of Small Object Memory Allocators

This section of the mini dissertation describes what big data science is and the different algorithms in the field of study and how they can benefit from a small object memory
allocator with special attention paid to the genetic programming algorithm in which it is explained in great detail. Further this chapter will explain what pre-allocating is,
when it can be used.

== Big Data Science

Big data science is a field that involves extracting meaningful insights, patterns, and knowledge from very large and complex datasets. The data being processed is typically too large or fast moving for traditional data processing software to handle. It combines elements of computer science, statistics, and domain expertise to make sense of massive volumes of data. Techniques such as machine learning, data mining, distributed computing, and cloud platforms to analyse data from sources like social media, sensors, transactions, and logs. Big data science is applied in fields such as finance, healthcare, marketing, cybersecurity, and urban planning @brady2019challenge @song2016big.
Many data science applications involve repeatedly creating and destroying large numbers of small objects. These could be tokens during text processing, individual entries in a sparse matrix, or nodes in a tree or graph structure. In most cases, these objects are only a few bytes in size and may be allocated millions of times during the lifetime of a program. If memory for these objects is not managed efficiently, it can quickly become a performance bottleneck.
This is especially important for machine learning algorithms such as Random Forests, K-Means clustering, or Expectation-Maximization (EM), all of which allocate many temporary objects during training or inference. For example, a decision tree will repeatedly allocate memory for new nodes while it is being built, and a clustering algorithm may create temporary cluster assignments in each iteration. Having a fast and efficient allocator for these small objects can significantly improve performance.
Text processing also benefits from small object allocators. Tokenisation, for instance, involves splitting text into many small strings or objects which are used briefly and then discarded. A general-purpose allocator will struggle to keep up with this allocation pattern, while a small object allocator can handle it efficiently by reusing memory from a fixed-size pool.

== Pre-allocating

Big data science applications make extensive use of dynamically allocated memory be that when reading data, performing transformations or when various algorithms are run. Memory for all these applications need to be provided in some manner and many times the exact amount of memory is required is unknown. Furthermore many data science applications make extensive use of interpreted programming languages which store all of their data on the heap.

Typically developers make use of a technique called pre-allocating to improve performance where memory is concerned. Pre-allocating is simply a method where if the amount of memory required is known before hand it is better to allocate it all at once rather than on an ad-hoc basis.

#code_block("An example of a Rust program that allocates 1000 integers without pre-allocating.")[
```rust
let mut array = Vec::new();

for i in 0..1000 {
  array.push(i);
}
```
]

#code_block("An example of a Rust program that allocates 1000 integers with pre-allocating.")[
```rust
let mut array = Vec::with_capacity(1000);

for i in 0..1000 {
  array.push(i);
}
```
]

In the two examples above the first code snippet creates an array with 1000 integers but does not pre-allocate while the second example makes use of pre-allocation. This small change can lead to massive performance improvements because memory only needs to be requested once. In the first example the program will need to many times request more memory from the allocator and copy the existing data over which will be noticeably slower than if the memory was only ever allocated once and never needed to be copied.

The issue with pre-allocating is that it only works if the amount of memory required is known before hand. Take the following example:

#code_block(
  "An example of a Rust program that allocates an unknown number of integers."
)[
```rust
let mut array = Vec::new();

// This loops keeps running until keep allocating returns false
let mut i = 0;
while keep_allocating() {
  array.push(i);
  i += 1;
}
```
]

It is unknown when the function ```rust keep_allocating()``` will stop the loop so the amount of memory required cannot be pre-allocated. So there are a couple of options:
- Allocate on demand as the code above does.
- Guess how much will be needed and pre-allocate.
The second option encounters some issues:
- If the guess was too small memory will need to be allocated anyway.
- If the guess was too large memory will be wasted.
So if the amount of needed cannot be guessed with some degree of precision the first option will be used. This is the scenario where a fast and efficient memory allocator will make a difference. This scenario does also apply in real world scenarios, for example when reading a CSV file for example the amount of memory required to store every data point in the file or even only some of the columns is not know before it is read.

== Applications

Many algorithms and applications used in data science and machine learning require allocating dynamic memory during their execution. Usually these allocations are for small objects—such as nodes, tuples, or temporary results that are created and destroyed many times. In situations like this performance can be drastically improved by an allocator that specialises in allocating and deallocating small objects.

=== Random Forest

In the Random Forest @rigatti2017random algorithm when building decision trees new nodes are allocated for each split in the data. Since a random forest can build hundreds or thousands of trees in a single run, the number of small node allocations can easily reach into the millions. Using a small object allocator can reduce memory overhead and accelerate training.

=== K-Means Clustering

K-Means Clustering @burkardt2009k is another algorithm that can benefit. In each iteration every data point is assigned to a cluster. These assignments are stored temporarily and updated repeatedly. The data points often times are very small.

=== Graph Algorithms

Many graph algorithms such as PageRank, Dijkstra’s algorithm, or community detection, many graph nodes and edges are represented as small structs/objects with a few fields. As the graph is traversed, updated, or built, thousands or millions of small allocations may be required. A small object allocator ensures that these operations remain fast and memory efficient.

=== Text Processing

Text processing and NLP pipelines are another area where small object allocation patterns emerge. For example, during tokenisation, stemming, or parsing, millions of short strings or metadata objects may be created and discarded. Efficiently managing memory for these transient objects improves the performance of data ingestion and preprocessing pipelines.

=== Genetic Programming

Genetic programming @koza1994genetic is a technique where trees are generated which represent a program that is used to solve a problem. It is a evolutionary algorithm and can be applied to variety of scenarios. The algorithm consists of many trees where the size and shape of each tree is unknown and arbitrary. This kind of problem is perfect for a small object allocator since each tree's size is unknown the amount of memory required cannot be pre-allocated but the nodes in a tree are still relatively small. Since generating these trees consists of a lot of small memory allocations a small object memory allocator can have substantial performance benefits.

=== Data Science Specific Programming Languages

Most data science tasks are put together using interpreted programming languages that heavily abstract away from the bare metal hardware that they run on. Even though the underlying algorithms are run using a more performant programming language most of the glue to put everything together is done in a high level programming language such as Python @python. Python already does use a custom memory allocator catered for small objects.

== Example

To further expand the significance of where a small object memory allocator would be useful take the Genetic Programming algorithm as mentioned above.

=== Overview

Genetic programming (GP) is an evolutionary algorithm designed with the goal of creating a program to solve a problem, this could be a regression, classification or any other kind of problem. The algorithm works by generating trees where each node may or may not have children nodes. For example if the goal was to find a mathematical function to model some data (a regression problem) the GP could be provided with nodes for addition, subtraction, multiplication, division and then the inputs to the function. A tree could be generated that represents this function or program. For example:
$
  f(x, y) = x * x + y
$

Could be modeled with the following tree:

#figure(
  caption: [Example of how the equation could be represented as a tree.],
  image(
    "../images/gp_tree_example.drawio.png",
    width: 70%,
  ),
)

A GP will generate many of these trees randomly and then following a process of natural selection will create new trees from the old trees and so on. This is done many times over until a stopping criteria is met.

=== Use of a Small Object Memory Allocator

The reason why a small object memory allocator would come in hand is since the trees are generated randomly the size of a tree cannot be known beforehand. Furthermore the nodes of a tree themselves usually have a very small memory footprints this means that these tiny objects are allocated thousands or even millions of time when a genetic program is run. This is ripe optimisation for a small object memory allocator.

== Summary

It is always better to pre-allocate the required memory as that is guaranteed to only ever be one allocation and the memory will be contigous which will improve cache locality
but it is not always possible to identify the amount of memory required in big data science applications and many applications need to allocate memory on the fly and in these
scenarios using a small object memory allocator can greatly benefit the performance of an algorithm or program.

What is important to take forward from this chapter is the explanation of the genetic program algorithm as this algorithm will be used to evaluate the memory proposed small
object memory allocator

// table algorithms where preallocating will not work
