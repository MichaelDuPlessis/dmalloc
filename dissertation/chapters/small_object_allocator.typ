= Small Object Memory Allocator

== Introduction

A small object memory allocator is a memory allocator that is specifically designed to efficiently allocate and deallocate small objects. There is no strong definition on what is considered a small object but small objects usually are within the range of 1 byte to a couple hundred bytes in size. Many memory allocators are designed to be general purpose and end up not being able to allocate small object efficiently due to how the data is stored along with the required metadata that is stored alongside it.

== Techniques

The following techniques are commonly used by small object allocators:

=== Size Classes

Objects are stored based on their size class and objects within the same size class are stored with one another. This allows for techniques where the allocator can be optimized for the specific size classes instead of being made more general purpose. A bin memory allocator is a common and popular implementation of this.

=== Memory Pooling

Large amounts of memory is preallocated by the allocator and smaller chunks within the pool of memory is then returned to the caller. This minimises the number of calls to the backing memory allocator to allocate memory (which could include a system call).

== Conclusion

A small object memory allocator is a memory allocator that utilises specific techniques to optimise the allocation and deallocation of small objects. They often use one or more techniques to achieve this goal. They are an extension on top of already existing memory allocators.
