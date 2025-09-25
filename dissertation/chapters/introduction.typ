// Why am I doing this study
// What is the gap
// Mention problem statement
// Discuss my research questions
// Methodology
// Roadmap to the rest of the document

= Introduction

Most programmers regardless of field do not worry or even think about the memory allocator they are using and will almost always use the default memory allocator of the
programming language they are using @grunwald1993improving. While the default memory allocator is often good enough the way that memory is allocated can be improved by
using a memory allocator that is more purpose fit for the problem. Therefore it is common practice for programmers who want to achieve as much performance as possible to
make use of custom memory allocators for their specific task @berger2002reconsidering. Most general purpose allocators tend to have worse performance compared to allocators
that are designed for a specific task. The field of data science and big data is no longer niche and thus the need for an allocator designed for specific use in the field
of data science becomes more and more necessary. There is a history of applications utilising purpose built allocators to increase performance of the intended task
including databases, operating systems and webservers but there has not yet been a published design or implementation of a purpose build allocator for data science related
tasks despite the recent explosion in the field's popularity. This mini-disseratation aims to introduce a memory allocator purpose built for data science tasks and the
large data sets that often need to be allocated in dynamic memory with a focus on efficiently allocating small objects as the default allocator tends to perform worse when
allocating small objects @bonwick1994slab.

== Problem Statement

// no windows
// why we want fast small object memory allocators
An important aspect in the field of data science and big data science is not just data collection and data analysis but also running various algorithms on the collected data
to derive results for various use cases such as prediction tasks. Many of these algorithms take significant time to run and/or train, thus optimising the runtime of these algorithms
can be beneficial, naturally these algorithms can be improved on a case by case basis but all these algorithms will make use of dynamic memory allocation so improving this
can lead to performance increases across the board instead of using the default memory allocator as is often the case by most programmers.

Custom memory allocators are no new thing but there is little to no research in developing memory allocators specifically for the field of big data science. This mini-dissertation
aims to remedy this by developing a small object memory allocator for the specific purpose of running optimisation algorithms faster than the default memory allocator for a
system. Searching for papers containing the terms "small object memory allocator", "memory allocator" and "memory allocator for big data science" end up returning few results
and results that are returned are usually only partly related to the topic. Custom memory allocators however are no new thing and are used in many places such as programming
languages as well as libraries and frameworks yet little research is published on the topic.

== Methodology

This mini-dissertation introduces a `mmap` based memory allocator specialising in small object allocation. The allocator will be developed and discussed, it will then be
compared to both the default memory allocator on Linux (Ubuntu 25) and MacOS. The comparison will make use of a number of artificial benchmarks as well as a real world benchmark
the results will then be compared and it will be determined whether the field of big data science can benefit from small object allocators. The allocators will be compared
based on total memory usage as well as program runtime.

== Overview of Chapters

The mini-dissertation contains the following chapters:
1. Overview of Memory Allocators: This chapter gives an overview of all the theory required to understand memory allocators.
2. Application of Memory Allocators: This chapter continues from the chapter 1 and gives examples of memory allocation strategies as well as the designs of already implemented memory allocators.
3. Big Data Science Applications of Small Object Memory Allocators: This chapter explains the need and usefulness of small object memory allocators for use in the field of Big Data Science.
4. Development of a Small Object Memory Allocator: This chapter discusses the implementation and the design of the small object memory allocator presented in this paper.
5. Discussion: This chapter goes over the results of the benchmark and comparisons of the different memory allocators.
6. Conclusion: This chapter contains the closing remarks.
7. Future Work: This chapter contains future work that can be built off of from this mini-dissertation.

@fig:chapter shows the order in which the chapters are recommended to be read before others. An arrow leading from one bubble to another means it is recommended
to read the chapter from which the arrow is coming from first.

#figure(
  caption: [A dependency graph of the chapters in the mini-dissertation.],
  placement: none,
  image("../images/chapters.drawio.png"),
) <chapter>
