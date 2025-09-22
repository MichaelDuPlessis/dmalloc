#import "@preview/i-figured:0.2.4"

// setting layout
#set text(11pt)
#set heading(numbering: "1.1")
#set page(
  number-align: center,
  numbering: "i"
)

// make sure you have some heading numbering set
#set heading(numbering: "1.")

// apply the show rules (these can be customized)
#show heading: i-figured.reset-counters
#show figure: i-figured.show-figure


#align(center)[
  #align(horizon)[
    #text(size: 50pt)[Developing a Small Object Memory Allocator For Big Data Science]
    
    #text(size: 30pt)[Author: Michael du Plessis]

    #text(size: 30pt)[Supervisor: Professor Linda Marshall]

    2025-10-31

    Submitted in partial fulfilment of the requirements for the MIT (Big Data Science) in the Department of Computer Science, University of Pretoria
  ]
]

#pagebreak()

#heading([Abstract], numbering: none, outlined: false)

This mini-dissertation introduces a small object memory allocator designed for the field of Big Data Science. The memory allocator aims to optimise runtime performance
of Big Data science tasks that make use of small objects by reducing the execution time of a program. The allocator outperforms the default memory allocators on both Linux (Ubuntu) and MacOS in
both artificial and real world benchmarks. The text further goes into the design of the small object allocator with its inner workings based around the unix `mmap`
system call and delves into the need for more research into custom memory allocators for the field of Big Data Science.

#pagebreak()

#outline()

#i-figured.outline()

#pagebreak()

#set page(
  number-align: center,
  numbering: "1"
)
#counter(page).update(1)

#include "chapters/introduction.typ"

#pagebreak()

#include "chapters/memory_allocators.typ"

#pagebreak()

#include "chapters/big_data_applications.typ"

#pagebreak()

#include "chapters/small_object_allocator.typ"

#pagebreak()

#include "chapters/development.typ"

#pagebreak()

#include "chapters/discussion.typ"

#pagebreak()

#include "chapters/conclusion.typ"

#pagebreak()

#include "chapters/future_work.typ"

#pagebreak()

#show bibliography: set heading(numbering: "1.")

// references
#bibliography("refs.bib", title: "References")

#include "chapters/appendix.typ"

// TODO:
// Ordering: √
// Title
// Abstract
// Acknowledgements
// Table of Contents
// Before introuction roman numerals
// Introduction
//
// Add chapter numbers to overiew
//
// Don't reference diagram with locality rather do "in Figure xxx" √
//
// Add reference to overview of memory allocators also mention that heap, cache, alignment, etc is important
//
// summerise in all chapters that are missing it
//
// overview of memory allocators specify that preallocting is described in later chapter
//
// remove small object allocator definition from data science chapter as well as general purpose memory allocators
//
// put chapter 4 into chapter 2.9
//
// split chapter 2 at 2.9 with Overview of Memory Allocators and Application of Memory Allocators
//
// use uk spelling √
//
// begin chapters on a new page √
//
// do better with figures
//
// fix figure referencing √
//
// conclusion too short take a look at the problem statement again
//
// make sure you state why figures in appendix exist
//
// just put introduction under chapter heading
//
// call conclusions at the end of chapter summary and it is what we are taking forward
