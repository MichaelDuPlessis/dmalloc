// add date
// submitted in partial fulfilment of the requirements for the MIT (Big Data Science) in the Department of Computer Science, University of Pretoria

// Introduction 1
// Why am I doing this study
// What is the gap
// Mention problem statement
// Discuss my research questions
// Methodology
// Roadmap to the rest of the document

// Title 2
// Small Memory ....

// Memory allocators 3
// What they are
// How they work
// Why they are important

// Big data science applications 4
// Why small memory allocators are necessary

// Development of a small memory allocator 5

// Application of small memory allocator 6

// Expiriments 7
// - Artificial benchmarks (time and memory used)
// - Actual application of the memory allocator (time and memory used)

// Discussion 8

// Conclusion 9

// Future Work 10

// #let chapter(chapter_name, introduction, summary, filename) = {
//   let heading_level = 1
  
//   heading(level: heading_level)[#chapter_name]

//   heading(level: heading_level + 1)[Introduction]
//   par()[#introduction]

//   include filename

//   heading(level: heading_level + 1)[Summary]
//   par()[#summary]
// }


#import "@preview/i-figured:0.2.4"

// setting layout
#set text(11pt)
#set heading(numbering: "1.1")
#set page(
  number-align: center,
  numbering: "1"
)

// make sure you have some heading numbering set
#set heading(numbering: "1.")

// apply the show rules (these can be customized)
#show heading: i-figured.reset-counters
#show figure: i-figured.show-figure

#outline()

#i-figured.outline()

#include "chapters/introduction.typ"

#pagebreak()

#align(center)[
  #align(horizon)[
    #text(size: 50pt)[Developing a Small Object Memory Allocator For Big Data Science]
    
    #text(size: 30pt)[Author: Michael du Plessis]

    #text(size: 30pt)[Supervisor: Professor Linda Marshall]
  ]
]

#pagebreak()

#include "chapters/memory_allocators.typ"

#include "chapters/big_data_applications.typ"

#include "chapters/small_object_allocator.typ"

#include "chapters/development.typ"

#include "chapters/discussion.typ"

#include "chapters/conclusion.typ"

#include "chapters/future_work.typ"

// references
#bibliography("refs.bib", title: "References")

#include "chapters/appendix.typ"
