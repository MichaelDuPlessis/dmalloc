= Application of Memory Allocators

This chapter will go over common memory allocation algorithms and kinds of memory allocators as well as discuss some designs of existing memory allocator implementations.

== Memory Allocation Strategies

=== Region Based Memory Management <region_allocator>

Region based memory management works by putting every allocated object into a region. The only way to deallocate an object is to free the entire region @gay1998memory @grossman2002region @tofte1997region @tofte2004retrospective. Region based memory allocation works by first allocating a sufficiently large amount of memory known as an area and having a pointer point to the next free spot in the block of memory (in the beginning this will be the the beginning of the first allocated memory block). When the program requests memory it is then giving to the pointer to the next free spot in memory and the pointer is moved forward to directly after the allocation. If the region becomes full or there is not enough space left in the region to fulfill the requested allocation a new block of memory is allocated and is used for further allocations @gay1998memory and are linked together using a linked list. It is not possible to deallocate single objects in memory and instead the entire block needs to be deallocated.

#figure(
  caption: [How the memory looks for a region based memory allocator.],
  placement: none,
  image("../images/regionallocator.drawio.png"),
) <regionallocator>

Diagram @fig:regionallocator the red blocks are allocated memory while the green is unallocated memory. Notice how there is a pointer to the next free spot in memory where the next allocation will take place.

The benefits of region based management are that the a large block of memory is preallocated thus reducing the number of calls to the operation system which is known to be a common bottleneck. It is very simple to implement and does not have any complex allocation strategy and so assuming that a new block does not need to be allocated the time for allocation is $O(1)$. Since all objects are store next to one another in memory there cannot be any internal fragmentation and the only possible external fragmentation that can occur is when there is not enough memory left in a region and a new one must be allocated.

There are also numerous downsides to region based memory management. If the region used is very large but only a few small objects are allocated then there can be a lot of wasted memory. Since it is not possible to deallocate single objects in memory it means even if majority of the objects are no longer referenced as long as there is one object in use all the memory must sit around. Most commonly region based management is explicit in the program and does not look like a "normal" allocation. This also means that the standard form of region based memory management is user managed and this then requires users to know about the lifetime of all the objects that are allocated to the region @gay1998memory.

#figure(
  [
```C
// Creating the region
int regionSizeInBytes = 128;
Region region = createRegion(regionSizeInBytes);

// Assinging values to the region
for (char c = 0; c < 128; c++) {
  char* ptr = (char*)allocate(&region, sizeof(char));
  *ptr = c;
}

// deallocation the region
freeRegion(region);

```
  ],
  caption: [An example of how region based memory management would look in C.]
)

The above code snippet shows an example of how a region could be created in C @c_standard and then allocated to. Notice how this does not use `malloc` and `free` and it requires the programmer to know the lifetime of the memory and deallocate themselves. Region allocators are also known as Arena or Linear allocators.

=== Stack-Like Allocator <stack_allocator>

The reason it is called a stack like allocator is because it allocates memory like a LIFO (last in first out) stack. The strategy employed is very similar to the already discussed region based memory management with the difference that every allocation also includes some metadata allowing for the data on top of the stack to be deallocated. The metadata stored with every allocation can be numerous things as long as it allows for allocator to know how far back it can go in its region of memory to free that memory. The metadata stored is stored in front of the allocated memory and is therefore known as the header. An example of what can be stored in the header is the padding required to align the allocated memory. Allocating memory is the same as allocating memory for the region based memory allocator except the size of the header must also be taken into account and thus the pointer to the allocated memory that is returned should be the address immediately after the header. @ginger

#figure(
  caption: [How the memory looks for a stack-like memory allocator after an allocation.],
  placement: none,
  image("../images/stackallocator.drawio.png"),
) <stackallocator>

As seen in diagram @fig:stackallocator the memory allocated now also contains a header. It can also be seen where the next address for free memory is before and after the allocation of new memory.

This immediately has the benefit over region based memory management that it allows for deallocation without needing to deallocate the whole block of memory but is still limited because only data on top of the stack can be deallocated. The other benefit is that allocation is still $O(1)$ since it uses the same technique to allocate memory as the region based memory allocator. Since all the memory is close to one another there is a high cache locality which can massively improve performance. 

Only being able to deallocate the memory on top of the stack leads to the problem that to efficiently use this allocator the programmer will need to be conscious of when they allocate and deallocate objects and should endeavour to arrange their lifetimes in a manner that allows objects that will be deallocated first to be on the top of the stack. Arranging objects in such a manner is not always possible and thus this allocator is not optimal in such scenarios as in the best case the method call to deallocate the memory does nothing and in the worst case an error occurs as deallocating memory that is not at the top of the stack is not supported. The other drawback of this strategy is that since some metadata is required for every allocation there will be a higher memory overhead, in other words every allocation of $n$ bytes will always be larger than $n$ bytes.

This kind of memory management can very useful for programming languages that utilise virtual machines as they have a virtual stack which is very often allocated on the heap @ArpaciDusseau23-Book @tanenbaum2015modern and this strategy lends its self well to how a stack works in a program running on a typical operating system.

=== Pool Memory Allocator <pool_allocator>

The basic idea between pool allocators is that a chunk of memory (also known as a buffer) is split into smaller chunks. The allocator then keeps track of which chunks are available and when a block of memory is requested a free chunk is provided. This means that some metadata needs to be stored to make it clear which blocks of memory are free and which are not. A common approach to this is by making use of a free list which is just a linked list @cormen2022introduction of the available chunks of memory. The head of the linked list points to the next available chunk of memory. Due to the data associated with each allocation it is possible for the pool allocator to deallocate individual objects. When an object is deallocated it is just made the new head of the free list @gay1998memory @boostpool @whyusepool.

#figure(
  caption: [How a pool allocator looks before any allocations.],
  placement: none,
  image("../images/poolallocator_empty.drawio.png"),
)

As seen above the pool allocator appears just like a normal linked list.

#figure(
  caption: [How a pool allocator could look after some number of allocations and deallocations.],
  placement: none,
  image("../images/poolallocator.drawio.png"),
)

Due to the ability for individual objects to be deallocated the free memory can no longer be next to one another as seen with the allocators in @stack_allocator or the @region_allocator. This means that the while the pointers will always start pointing to neighboring memory blocks as time goes on this will not always happen.

This is a more advanced strategy and immediately has some benefits. The clearest being that it is not possible to deallocate any individual object in memory and the memory is deallocated in constant ($O(1)$) time. The memory can also be allocated in constant time as the head of the linked list just needs to be popped off and updated. The other major benefit is that if the original buffer of memory becomes full it is relatively easy to allocate a new buffer and having it link to the existing buffer assuming allocating a new buffer is possible. A new buffer just needs to be updated and the linked list just needs to point to it as depicted in diagram @fig:poolallocator_extended.

#figure(
  caption: [How more buffers can easily be added to a pool allocator.],
  placement: none,
  image("../images/poolallocator_extended.drawio.png"),
) <poolallocator_extended>

In diagram @fig:poolallocator_extended the red blocks are allocated memory, the green blocks unallocated memory and the orange blocks are a region of memory that is not available to the allocator.

Naturally, there are downsides to this kind of allocations strategy. Every allocation requires some metadata which means that there will always be more memory used than requested. As well with that each memory block being a fixed sizes means that while there will never be any internal fragmentation two other problems can occur:
1. A memory request smaller than the block size can be requested which will lead to internal fragmentation.
2. The amount of memory requested may be larger than the block size in which case the memory allocation will fail or will be deferred to the backing allocator.
The final issue with this memory allocation strategy is that since it makes use of a linked list it naturally has poor cache locality which will be exasperated if a new buffer needs to be allocated @hennessy2011computer.

An interesting technique that can be employed to determine what chunks of memory is free is to make use of a bit mask. For example if a 32 bit integer is use than each bit corresponds to a chunk of memory and if the value of the corresponding bit is 1 than the chunk is available otherwise it is not or vice versa. This method has the benefit that a header does not need to be stored with every block of allocated memory but has the downside that the number of chunks in your buffer of memory is limited by the size of the integer you use. Allocating a new buffer of memory would require a list of these bitmasks to be kept.

=== Free List Memory Allocator <freelist_allocator>

Free list allocators are a form of memory allocation that allows for allocating randomly sized portions of memory as well as out of order frees. While this makes them very usable it does make them slower than the previous allocators that were discussed. There are two common approaches to when implementing a free list memory allocator. A common way to implement a free list allocator is by using a linked list @ginger.

A linked list is used to store the blocks of memory that are free along with the size of the free block. The implementation will search through the linked list to find a block that can fit the requested allocation and then remove that block from the linked list and places what is known as an allocation header. Below is an example of what a linked list implementation may look like at some point in time after a number of allocations and deallocatioins.

#figure(
  caption: [An example of what a free list may look like at a random point in time.],
  placement: none,
  image("../images/freelist_allocator_linkedlist.drawio.png"),
)

Allocation works by finding a free block in memory based on some strategy such as first fit, best fit and so on. Once a suitable location has been found a header is placed with the information on how large the allocated block is and the new links for the linked list is made. Diagrams @fig:freelist_before and @fig:freelist_after depict what it looks like when allocating memory to a free list. The purple block is the memory that is too be allocated and in this example first fit is being used so the head of the linked list is chosen to allocate the memory too.

#figure(
  caption: [How a free list looks before allocation.],
  placement: none,
  image("../images/freelist_allocator_linkedlist_before_allocation.drawio.png"),
) <freelist_before> 

#figure(
  caption: [How a free list looks after allocation.],
  placement: none,
  image("../images/freelist_allocator_linkedlist_after_allocation.drawio.png"),
) <freelist_after>

Deallocating is a bit more complex as the linked list needs to be repaired after deallocation. To do this the linked list is iterated over until the unallocated memory just before the memory wishing to be free is found. Once it is found its next pointer can be updated to point to the newly freed memory and the newly free memory next pointer can be updated to point the the old next pointer of the previous unallocated memory block. The process is not dissimilar to inserting into sorted linked list. This means that deallcation has time complexity $O(n)$. During this whole process if the free memory caused unallocated blocks to be contagious they are then merged into one larger block. And example of the process is depicted below.

#figure(
  caption: [How a free list looks before deallocation.],
  placement: none,
  image("../images/freelist_allocator_linkedlist_before_deallocation.drawio.png"),
)

#figure(
  caption: [How a free list looks after deallocation.],
  placement: none,
  image("../images/freelist_allocator_linkedlist_after_deallocation.drawio.png"),
)

Overall the free list implementation of an allocator has the benefits that there is no internal fragmentation and that arbitrary sized memory allocations are allowed while still allowing for arbitrary deallocations. It also does not suffer from any internal fragmentation as only the amount of space needed for an allocation is used.

The downsides are that the deallocation can be slow if the linked list is large and it heavily suffers from external fragmentation but this can be mitigated based on the placement strategy used.

=== Buddy Memory Allocator

The buddy memory allocator is looks to improve on the memory fragmentation issues that allocators using free lists are susceptible to. It works by recursively splitting memory a larger block of memory to fit the requested amount of memory. A block of memory in a power of two and a free list is initialised similar to what is done with a pool allocator. The free list contains one element which points to the whole block of memory. When a piece of memory is requested the free list is searched for a block of memory that will fit the requested amount than if the memory to be allocated is at least half the size of the block it is being allocated in the block is than split into two equal halves. This is done recursively until the block cannot be split in half anymore. All blocks of memory created this way are added to the free list (besides the block that the new allocation is going to be used for) @von1975simple @ginger @knowlton1965fast.

#figure(
  caption: [How a free list looks after deallocation.],
  placement: none,
  image("../images/buddy_allocator.drawio.png"),
) <buddy_allocator>

As can be seen from diagram @fig:buddy_allocator the memory is recursively split until it can be not longer. This assists with minimising fragmentation by trying to use as little memory as possible. The allocator may make blocks too small that a new piece of memory cannot be allocated when this happens blocks of memory are coalesced with one another until a block of memory large enough to fit the requested allocation is created.

Deallocation works just by marking the block of memory as free and adding it to the free list. This means that freeing memory is fast as it is $O(1)$. This allocator also minimises internal fragmentation by trying to decrease the size of blocks and also decrease external fragmentation as memory blocks next to one another can be made bigger if necessary.

The downsides of this allocator is that while deallaction is fast allocation my not necessarily be since there may be any number of recursive calls involved as well as an unknown amount of memory blocks may need tobe coalesced. This means it is difficult to determine exactly how long an allocation will take as a unknown number of events may or may not occur.

=== Bin Memory Allocator

A bin memory allocator works by organising memory into bins of fixed sizes. A bin is simply a region of memory and an allocator will have bins of varying sizes @berger2000hoard. A bins size is the maximum size an allocation must be to be placed in that bin, for example if an allocator has bins 4 bytes, 8 bytes and 32 bytes then if a 5 byte allocation request is made it can be placed in the 8 or 32 byte bin. When a memory allocation is requested the smallest bin that can fit the requested amount of memory is used for the allocation so in the example the 8 byte bin would be used @li2018efficient. An allocator can obviously not have an infinite number of bins so if an allocation request comes in that cannot fit into any bin the backing memory allocator will be used instead. Each bin itself is divided into blocks of its respective bin size so when an allocation request comes in a free block from that bin is returned.

#figure(
  caption: [A Diagram Explaining the Concept of Bins.],
  placement: none,
  image("../images/bins.drawio.png"),
)

#figure(
  caption: [An Example of a Bin Allocator with Three Bins.],
  placement: none,
  image("../images/binallocator.drawio.png"),
) <binallocator>

Diagram @fig:binallocator depicts a typical bin allocator with 3 bins. The green blocks is unallocated memory, the red blocks allocated memory and the orange blocks wasted memory (fragmentation).

The bin memory allocator tries to reduce fragmentation in an efficient way. While the buddy allocator may still perform better in terms of decreased fragmentation it has the downside of needing to use recursion which increase allocation and deallocation times.

The downsides of a bin allocator are that while allocation is fast since finding a bin is fast there is a high likelihood of internal fragmentation. Imagine if we again had bins of 4 bytes, 8 bytes and 32 bytes than if many allocations of 16 bytes came in they would be allocated into the 32 byte bin but this means for every 16 byte allocation 16 bytes are wasted. Due to this bin allocators number of bins and bin sizes need to be though about carefully to reduce the severity of internal fragmentation.

An interesting change that can be made to bin allocators is that instead of dividing bins into fixed sized blocks is to instead have a separate allocator handle the allocations. Any memory allocation strategy can be used within a bin and thus what this means is that if a implementor wanted to use different algorithms for different sized allocations since certain algorithms may be more beneficial they are then able to. A bin could in fact use a bin allocator in itself as its allocation strategy.

=== Other Kinds of Allocators

Another problem that may arise when creating a memory allocator is its applicability in multithreaded environments which in this modern day has become the norm rather than the exception @gandham2024performance. Berger et al. @berger2000hoard created the Hoard memory allocator to address the issues with multithreaded allocators such as allocator-induced false sharing of heap objects and blowup. The main issue of all the problems with memory allocators in multithreaded environments is that all the threads share the same region of heap memory and thus managing what thread can allocator what and where as well as sharing memory blocks becomes a challenge.

A persistent memory allocator is an allocator that will allocate memory to non-volatile storage allowing the memory allocations to be reused again @dang2024pmalloc but this may incur a speed cost as non-volatile memory is typically slower than volatile memory.

A slab allocator is a popular strategy for allocating small objects and involves having "bins" for various object sizes thus speeding up allocation and deallocation as it is easier to find free space with for an object of a certain size @bonwick1994slab @iwabuchi2022metall. It also tends to avoid internal fragmentation as memory regions (bins) are already split into fixed sized blocks but can lead to an increase in external fragmentation. For example if each bin is 64B in size and a bin is allocated for objects of 1B in size then if the bin only has one allocation 63B is wasted until further allocations occur.

Other memory allocators are designed to prevent bug such as use after free and improve security. An example of this is the cling memory allocator @akritidis2010cling.

== Existing Memory Allocators

The text below discuss commonly used memory allocators.

=== jemalloc

jemalloc is a memory allocator designed originally for the FreeBSD operation system @freebsd to make better use of multiprocessor architectures. The core concept behind jemallocs implementation are arenas which is a memory pool for specific threads. Since each thread gets its own arena it improves multiprocessor utilisation by reducing contention between processors. Each arena is further divided into bins, runs and chunks.
- bins are used for small object allocations.
- runs are used for medium object allocations.
- chunks are used for large object allocations.
jemalloc further makes use of thread-local caching to improve performance. Small objects make use of bitmaps and specialised operations to efficiently know when blocks in a bin are free @evans2006scalable.

=== TCMalloc

TCMalloc (Thread-Caching Malloc) is a memory allocator developed by Google. It is designed to optimise memory allocation and deallocation, particularly in multi-threaded applications. TCMalloc aims to achieve the following goals:

- Fast, uncontended allocation and deallocation for most objects.
- Flexible use of memory.
- Low per-object memory overhead.
- Efficient sampling mechanisms for performance monitoring.

TCMalloc is divided into three main parts.

#figure(
  caption: [The Three Main Parts of TCMalloc.],
  placement: none,
  image("../images/tcmalloc.drawio.png"),
)

==== Frontend

The frontend handles memory allocations of small objects using thread-local caches. Each thread has a private cache that can only be accessed by that thread, eliminating the need for locks and improving allocation and deallocation performance.

- Small objects are allocated using size classes, similar to a bin-based memory allocator.
- If the frontend cannot satisfy an allocation request, it fetches memory from the middle-end to refill the cache.
- When an object is deallocated, it is placed back into the frontend cache if there is space. Otherwise, it is returned to the backend (PageHeap) for reuse.

==== Middle-End

The middle-end is built from a CentralFreeList and a TransferCache.

- The CentralFreeList holds free objects that can be used to replenish thread-local caches.
- The TransferCache provides a fast mechanism for moving freed objects between the frontend and backend without excessive overhead.
- If an allocation request is too large for the frontend, or if the middle-end runs out of memory, the backend (PageHeap) handles the request.
- The middle-end provides memory to the frontend and returns unused memory to the backend as needed.

==== Backend (PageHeap)

The backend is responsible for managing large blocks of unallocated memory and interacting with the operating system to request or release memory.

- The backend uses PageHeap, which manages large memory chunks.
- When the middle-end requires more memory, it requests pages from PageHeap.
- If memory is no longer needed, the backend can return it to the operating system using `madvise()` or `munmap()`.

This structure allows TCMalloc to achieve high performance by reducing contention, efficiently managing small and large objects, and minimising fragmentation.

=== mimalloc

Mimalloc is a memory allocator created by Microsoft @leijen2019mimalloc. This memory allocator was designed for backend implementations of programming languages but it can be used for other use cases. Mimalloc makes use of the following ideas:
- Extreme free list sharding.
- Separate thread-free lists.
- Local free list per page for thread local frees.
- There are no locks used only atomic operations.
An important note is that when referring to "pages" with regards to mimalloc it is not referencing memory pages as an OS would but closer to a superblock @berger2000hoard.

==== Free List Sharding

Free lists have the inherent issue of having bad locality due to the fact that objects can be scattered across many places in the heap. Mimalloc attempts to negate this by making use of free list sharding. The heap is subdivided into smaller sections called a page and each page has its own free list per size class. This greatly improves cache locality allowing for less time to be spent fetching data from main memory.

#figure(
  caption: [The Extreme Free List Sharding in Mimalloc.],
  placement: none,
  image("../images/mimalloc_free_list_sharding.drawio.png"),
)

==== Local Free List

Mimalloc makes use of a local free list when deallocating memory. When memory is freed it is not added back to the free list where it was taken off from instead it is placed on a separate free list. When the free list use for allocations runs out of memory to allocate which will now happen after a set number of allocations memory is then freed from the local free list. This means that the allocator will wait to free memory until a later time and allows for the possibility that new memory does not need to be allocated if there is enough memory on the local free list.

==== Thread Free List

Mimalloc pages are also in a thread local heap which means that no locks are needed when allocating memory since each thread has its own region of memory. A thread can still however free memory on another thread to make this still efficient mimalloc employees a thread free list per page.


=== glibc malloc

The glibc malloc was created from the pthreads malloc. This implementation is a heap-style allocator, meaning it uses variable-sized memory regions rather than fixed regions, bitmaps, or arrays. Glibc's malloc makes use of the following concepts:
- Arena: Contains references to at least one heap and a linked list of free chunks within the heaps. A thread will only allocate memory from its assigned arena.
- Heap: A region of memory belonging to exactly one arena, subdivided into chunks.
- Chunk: A block of memory that can be allocated or freed. Freed chunks can be coalesced with neighbors to form larger chunks. Each chunk belongs to exactly one heap, and thus to one arena.
The allocator operates in a chunk-oriented fashion. A heap is divided into smaller chunks, each of which stores its size and status (allocated or free). When freed, chunks enter a free list. Glibc maintains multiple arenas, allowing multiple threads to allocate memory in parallel. While mutexes are still used to guard arena access, threads can try different arenas to reduce contention. Glibc malloc maintains several types of free lists:
- Fast Bins: Store small, fixed-size chunks in size-specific singly-linked lists. Chunks are not coalesced on free to maximise speed and may be migrated to other bins later.
- Unsorted Bin: Freed chunks are first placed here, deferring sorting until allocation time. This centralises sorting logic and allows quick reuse of recent frees.
- Small Bins: Manage fixed-size chunks, coalescing adjacent free chunks to form larger regions. Small bins use doubly-linked lists for efficient removal and insertion.
- Large Bins: Handle variable-size chunks. Allocation requires finding the best-fit chunk, potentially splitting larger chunks to satisfy requests.
Additionally, glibc uses:
- Top Chunk: The remaining free space in a heap. If no suitable chunk is found in the bins, the allocator attempts to allocate from the top chunk, expanding it with sbrk() or mmap() if necessary.
- mmap: For very large allocations, glibc bypasses arenas and bins, mapping memory directly from the OS using mmap().
- Thread-Local Cache (tcache): Modern versions of glibc use per-thread caches to speed up allocation and deallocation, reducing contention even further.

== Summary

This chapter explained common memory allocation strategies and analysed real world exampls of memory allocators. Attention should be paid to with regards to the bin memory
allocation strategy as it is used in the design of the proposed memory allocator.
