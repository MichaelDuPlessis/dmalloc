
= Overview of Memory Allocators

A memory allocator is an algorithm that decides how to manage memory this includes when and how memory is requested from the operating system as well as returned.
Most memory allocators are known as general purpose memory allocators and are designed to be used for a wide variety of use cases. This chapter will give an overview of all
the important aspects of a memory allocator and what must be considered when building a memory allocator such as
what is heap memory, what is cache and why it is important, an introdcution to memory alignment and the performance impacts of misaligned memory and how
a memory allocator requests memory from the operating system. The chapter further discusses the main components and theory behind building memory allocators such as the inner workings of a memory allocator, what composability is and a definition of small
objects. The chapter then ends off by explaining how memory allocators are evaluated and how their effectiveness is determined when compared to other memory allocators.

== Heap

Programs running on modern computers on modern operating systems have two places where they can allocate memory: the stack or the heap. Memory allocated on the heap is commonly referred to as dynamically allocated memory and is allocated at run time of the program and the exact amount of memory required does not need to be known at compile time. This is in contrast to stack memory where the amount of memory required is always known at compile time. Memory on the heap is accessed through a pointer, which is always a fixed size (64 bits on modern 64 bit computers), the pointer it is accessed through is stored on the stack @ferres2010memory. Stack memory is handled by the compiler by using built-in instructions while heap memory has to be managed by the user known as manual memory management or automatically managed by the program by a system known as garbage collecter but this will incur a runtime penalty. Dynamic or heap memory is what is allocated through a memory allocator and its deallocation and allocation strategies are managed by an allocator. From the programmer's perspective they are just calling functions to get pointers to dynamically allocated memory and calling functions to deallocate that memory pointed to by a pointer @ArpaciDusseau23-Book @bryant2011computer @tanenbaum2015modern.

#figure(
  caption: [How memory is allocated on the stack and heap.],
  placement: none,
  image(
    "../images/stackheap.drawio.png",
    width: 45%,
    // height: 75%,
  ),
) <stack_and_heap>

In @fig:stack_and_heap the grey blocks are unallocated memory, the green blocks are memory allocated on the stack and the blue blocks are memory allocated on the heap. Notice how stack memory is always next to one another while heap memory can have gaps between allocated blocks. Stack memory also has the property that the allocated memory is ordered so that memory allocated later is closer to the bottom of the stack. So if `int a` is allocated before `int b` then `int a` will be above `int b` on the stack. This chronologically ordered kind of allocation does not apply to heap memory.

This poses a challenge since heap memory can become fragmented @ArpaciDusseau23-Book and this must be managed. The reason for these differences is because stack memory allocation can be calculated at compile time while heap memory is allocated at runtime in a seemingly random manner since an available space must be found to place the data.

== Cache

Assuming a common modern day computer system, cache memory is a special kind of memory that sits between a computer's main memory and the CPU (central processing unit). Cache memory allows for extremely fast retrieval of data and therefore allows programs to execute faster. Cache memory is orders of magnitude faster than main memory but is usually very small relative to main memory @smith1982cache. CPU cache also tends to come in multiple levels most commonly designated as L1, L2, etc. where L1 is the smallest cache but the fastest with speeds decreasing and size increasing as the numbers go up @hennessy2011computer. Hennessy and Patterson in their book also describe the principle of locality which states that data recently used is likely to be used again as well as the data around it (around it meaning neighboring memory addresses) @hennessy2011computer. This principle is applied by not just putting the accessed data in cache but data around it, this is known as a cache line. This implies that certain data structures have better cache locality since they are more tightly grouped together. A common example of this is a linked list vs an array.

#figure(
  caption: [An example of how arrays are allocated in memory.],
  placement: none,
  image("../images/array.drawio.png"),
) <array>

#figure(
  caption: [An example of how linked lists are allocated in memory.],
  placement: none,
  image("../images/linkedlist.drawio.png"),
) <linkedlist>

@fig:array and @fig:linkedlist compares how linked lists and arrays store their data in main memory. In the figures grey blocks are unallocated memory while green blocks are allocated memory.

As you can see, elements in an array are stored next to each other in main memory this means that they have a good cache locality and when one element of the array is accessed it, as well as other elements, in the array are likely to be cached thus allowing for faster access and a faster execution of the program. Looking at the linked list on the other hand the elements are not next to one another and therefore have a bad cache locality so if the data at address 10 is accessed it is unlikely that any other data of the array will be cached alongside which will slow down the execution of the program as whenever another element of the linked lists is fetched the CPU will first experience a cache miss and have to retrieve the data from main memory. On top of that linked lists usually require more memory as every data element needs to store the data (or a pointer to the data) as well as a pointer to the next element which on 64 bit operating systems is usually 8 bytes so even if the elements were next to each other as with an array due to the increased size of each element fewer elements would be able to fit in the cache at once.

This all should be kept in mind when developing a memory allocator as efficiently storing the data can lead to significant gains in performance as there will be fewer cache misses.

== Memory Alignment

Alignment refers to the restriction that certain data types or memory addresses must adhere to specific boundaries, such as 4-byte or 8-byte alignments, depending on the system architecture. Memory allocators must allocate memory with a specific alignment. This is done to improve performance but also because in some cases it is a requirement by the processor's architecture.

Modern processors are designed to access memory in aligned chunks, typically corresponding to the system's word size e.g. 8 bytes on a 64 bit system. Misaligned memory access can lead to increased CPU cycles, as the processor may need to issue multiple memory requests to retrieve a single piece of data @drepper2007every. For example if an 8 byte piece of data is stored from memory address 13 to 20 in the system's memory and the system has an 8 byte word size, the CPU will then need to do two fetches, one from 8 to 15 and from 15 to 23. It will then need to combine the retrieved memory leading to a more expensive operation than just getting the data from an aligned memory address as that would only require one fetch with no data manipulation.

#figure(
  caption: [An example of what misaligned memory looks like.],
  placement: none,
  image("../images/alignment_bad.drawio.png"),
) <alignment_good>

#figure(
  caption: [An example of what aligned memory looks like.],
  placement: none,
  image("../images/alignment_good.drawio.png"),
) <alignment_bad>

In @fig:alignment_good and @fig:alignment_bad the green blocks represent unused memory, the red blocks extra memory that is being fetched and the purple blocks the actual memory that we want to access. As we can see if the memory is misaligned it requires fetching extra memory whereas in the second figure the memory that we want to fetch is aligned correctly and is therefore easy to get without any extra operations. The figure is not completely accurate since computers will usually fetch memory equal to their word size so on most 64 bit systems two 8 byte chunks would be retrieved on the bad alignment and one 8 byte chunk on the good alignment version.

On some processor architectures data must be aligned to its natural boundary. A data type's natural boundary is usually the smallest power of 2 that can fit the data. For example a 4 byte piece of data has a natural boundary of 4 bytes whereas a 6 byte piece of data has a natural boundary of 8 bytes @hennessy2011computer.

== System Calls

A system call is how a running program is able to communicate with the underlying operating system of the computer. System calls allow a running program to perform operating system specific tasks as many of these can only be executed in a protected mode that only the operating system has access to. System calls are operating system specific but most operating systems have system calls for the following operations:
starting a process, alocating memory, spawning a thread and other specific things that only the operating system can do.
The largest concern when it comes to system calls for a memory allocator is the performance overhead. System calls require a context switch from user mode to kernel mode which is resource intensive. Thus a good memory allocator will try to minimise the number of system calls made.

== Concepts Behind Memory Allocators

=== Inner Workings of a Memory Allocator

Fundamentally a memory allocator manages a block of memory. This block of memory is also referred to as a pool of memory or a chunk of memory. This block of memory that is managed by the allocator is just a region of raw bytes. It does not matter where these raw bytes are situated, for example while memory allocators generally work with heap memory there is no reason that the memory managed by the allocator cannot sit on the stack instead. A memory allocator is further able to allocate chunks of memory of any size from this region of memory as long as the requested block of memory is smaller than the total amount of free memory in the region. Allocation means to return a pointer to the callee where this pointer can be assumed to point to valid memory that the callee can interact with and that is at a minimum the size that the callee requested. A memory allocator also needs to be able to deallocate memory by having the callee provide the address of previously allocated memory that was allocated by that allocator, the allocator can then mark this memory as deallocated and provide it again at a later time possibly with a different size of requested memory. A memory allocator also must have a strategy for how it allocates the memory @alexandrescu2001modern.

#figure(
  caption: [An Example of a Callee Requesting Memory.],
  placement: none,
  image("../images/allocrequest.drawio.png"),
) <memory_request>

As seen in @fig:memory_request above even though the callee requested 10 bytes of memory they received 12 bytes which is valid since from the callee's perspective they can always assume that they received at least 10 bytes.

=== Small Objects

Small objects do not have a strong definition and is usually left to the implementor what they define as a small object for example the Python programming language has
its own allocator for small objects and defines small objects as an object whose size is less than 512 bytes @pythonmemory while the paper by Klein et al @klein2014memory
defines small objects of having a size of 16-64 bytes. A small object allocator is an allocator that focuses on the efficient allocation and deallocation of objects with
a small memory footprint typically only a number of bytes in size. Small objects are used in various places for example Schußler and Gruber create a small
object allocator to allocate and deallocate millions of mesh primitives @schuessler2016traversable. Memory allocators usually have a method to deal with small
objects but once an object is no longer considered small a different allocation strategy will be used as seen with the Slab Allocator @bonwick1994slab.
While all memory allocators can allocate small objects a small object allocator is designed to efficiently allocate and manage small data allocations as general
purpose allocators tend to perform poorly when given the task of allocating small objects. A small object memory allocator likewise can more often than not allocate non-small
objects. A small object memory allocator does not need to be seperate allocator but can be an allocation strategy for an allocator. This ties into the composability of memory
allocators. There are various techniques used when allocating small objects such as:
- Size Classes: Objects are stored based on their size class and objects within the same size class are stored with one another. This allows for techniques where the allocator can be optimised for the specific size classes instead of being made more general purpose. A bin memory allocator is a common and popular implementation of this.
- Memory Pooling: Large amounts of memory is preallocated by the allocator and smaller chunks within the pool of memory is then returned to the caller. This minimises the number of calls to the backing memory allocator to allocate memory (which could include a system call).

=== Composability

Composability is the property of memory allocators that allow them to be chained together and built off one another. For example if I have a memory allocator that can allocate
arbitrary sizes of memory then I can naturally use this memory allocator to build another memory allocator that requests its memory from this allocator before handing it to
the callee.

#figure(
  caption: [A Diagram Explaining the Concept of Composability in Terms of Memory Allocators.],
  placement: none,
  image("../images/composableallocator.drawio.png"),
) <composable>

In the @fig:composable allocator *A* uses allocator *B* as a backing allocator. Allocator *B* in turn uses allocators *C*, *D* and *E* as backing allocators and chooses between them given some heuristic. Allocator *C* and *D* use *F* as their backing allocator while allocator *E* uses *G* which in turn uses *F*. This shows how allocators are built off one another and while a design does not necessarily require this many separate allocators it indicates the power of composability that memory allocators should have.

== Building Blocks of a Memory Allocator

Every memory allocator requires a backing memory allocator as well as an allocation strategy and memory allocators should be designed to be composable which is done by ensuring every allocator conforms to the same interface.

=== The Backing Allocator

To create a memory allocator requires a memory allocator, this is often known as the backing allocator. The reason that a memory allocator requires a memory allocator is due to the fact that the memory an allocator manages needs to come from somewhere. If one were to read about memory allocators it is common that the underlying (also known as backing) memory allocator is left out. The reason this is done is because it is simply an implementation detail and whatever appropriate memory allocator can be used as the backing memory allocator. For example Bonwick's @bonwick1994slab Slab Allocator could have used malloc or any other allocator to provide it with its initial memory. A common question then is where does the backing allocator get its memory? The lowest level allocator that provides all the memory is provided by the operating system and is usually mmap or sbrk on Linux Operating System @ArpaciDusseau23-Book assuming there is an operating system to begin with. 

=== Memory Allocation Strategy

The memory allocation strategy is the heart of the allocator and it encompasses all the logic of the memory allocator. It is responsible for deciding when to request additional memory from the backing allocator and if the memory should really be de-allocated when a de-allocation request occurs. The memory allocation strategy is also responsible for dealing with both internal and external fragmentation and if the requested object to be allocated should rather be passed onto a different allocator.

=== Fragmentation

Internal fragmentation occurs when more memory is allocated than what is necessary. For example if 1 byte of memory is requested but the allocator slots it into a slot of size 2 bytes than 1 byte of memory is wasted. External fragmentation occurs when there is wasted memory between allocated blocks @balabhadra2015data.

#figure(
  caption: [An Example of Internal Fragmentation.],
  placement: none,
  image("../images/internal_fragmentation.drawio.png"),
)

#figure(
  caption: [An Example of External Fragmentation.],
  placement: none,
  image("../images/external_fragmentation.drawio.png"),
)

== Evaluating Memory Allocators

The paper by Pastaryev et al. provides a taxonomy on memory allocators @rauchwergergenerating. They define three metrics for memory allocators:
+ Speed of allocation.
+ Amount of memory used.
+ Quality of allocation.

When trying to optimise for these three metrics it is often the case that improving one metric will be to the detriment of another. As an example storing more metadata may allow for faster allocation and deallocation but this then leads to more memory being used than what is required.

// === Speed of Allocation

The speed of allocation is just how long it takes for the allocator to return a block of memory to the callee. This time taken can be influenced by a number of factors. For example when a callee requests memory the allocator may need to first request more memory from its backing allocator which will take time and is subject to the very same considerations as the calling allocator. Another important consideration is that even if there is enough memory for the requested amount finding a suitable block can take some time. If the allocator where to for example manage its memory as a linear array then it may need to iterate through the array to find a block of memory large enough to for the requested amount. In the described scenario it can largely be improved by storing the next free block of memory internally but this will increase the overall memory footprint of the allocator.

// === Amount of Memory Used

Allocators consume memory in three main ways: the memory it allocates, through internal and external fragmentation, by storing metadata.
Fragmentation as well as storing metadata causes the memory allocated to be more than the requested amount of memory to be allocated.
A good allocator tries to minimise this by reducing the amount of fragmentation and only storing as much metadata as necessary.
Metadata is never visible to the end user and that some metadata will always be required. Take the example of the C standard libraries malloc and free functions @c_standard:

#figure(
  [
```C
int* x = (int*)malloc(sizeof(int) * 10);

// do something really cool with 'x'

free(x);
```
  ],
  caption: [An example of a C program that allocates 10 integers and then deallocates them.]
)<c_allocation>

@fig:allocation C code that allocates 10 integers on the heap using the inbuilt malloc but if you take a look at the corresponding free function to deallocate the memory you will notice the amount of memory to deallocate is not specified which means that the memory allocator needs to store some metadata about each allocation so that it can later be deallocated.

// === Quality of Allocation

Quality of an allocation largely has to do with a computers cache memory. If all the allocated data is next to one another then the computers cache will most likely have the data for the other elements in it even if only one was accessed which will lead to more cache hits and an overall speed improvement. Grunwald et. al. @grunwald1993improving emphasises the importance of having memory allocators with a good allocation locality as it can lead to large increases in performance for the program accessing the memory.

This is the same reason why arrays are usually preferred over linked lists since arrays have a much higher cache locality than linked lists which can have their data stored anywhere in memory and it is not guaranteed that the elements in a linked list will be local to one another.

== Summary

This chapter discusses the important theoretical concepts of memory allocator and serves as a good introduction. The most important aspects to take forward from this chapter
is the definition of small objects, the building blocks of a memory allocator and the different aspects in which a memory allocator can be evaluated. 
