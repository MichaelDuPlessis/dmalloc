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
which are designed for a specific task. The field of data science and big data is no longer niche and thus the need for an allocator designed for explicit use in the field
of data science becomes more and more necessary. There is a history of applications utilizing purpose built allocators to increase performance of the intended task
including databases, operating systems and webservers but there has not yet been a published design or implementation of a purpose build allocator for data science related
tasks despite the recent explosion in the fields popularity. This proposed research paper aims to introduce a memory allocator purpose built for data science tasks and the
large data sets that often needed to be allocated in dynamic memory with a focus on efficiently allocating small objects as the default allocator tends to perform worse when
allocating small objects @bonwick1994slab.

== Problem Statement

// no windows
// why we want fast small object memory allocators
An important aspect in the field of data science and big data science is not just data collection and analysis but also running various algorithms on the collected data
to derive results for various use cases such as prediction. Many of these algorithms take significant time to run and/or train thus optimising the runtime of these algorithms
can be beneficial, naturally these algorithms can be improved on a case by case basis but all these algorithms will make use of dynamic memory allocation so improving this
can lead to performance increases across the board.

Custom memory allocators are no new thing but there is little to no research in developing memory allocators specifically for the field of big data science. This mini-dissertation
aims to remedy this by developing a small object memory allocator for the specific purpose of running optimisation algorithms faster than the default memory allocator for a
system.

== Methodology

This mini-dissertation introduces a `mmap` based memory allocator speicalising in small object allocation. The allocator will be developed and disucssed, it will then be
compared to both the default memory allocator on Linux (Ubuntu) as well as MacOS. The comparison will make use of a number of artificial benchmarks as well as a real world benchmark
the results will then be compared and it will be determined whether the field of big data science can benefit from small object allocators. The allocators will be compared
based on total memory usage as well as program runtime.

== Overview of Chapters

// What each chapter speaks about
// with diagram of how they link
