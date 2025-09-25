= Conclusion

This mini-dissertation explains the importance of custom memory allocators and the performance improvement they can bring. It further points out the lack of formal research in
developing memory allocators for the field of Big Data Science and aims to address this issue by developing a memory allocator specialising in small objects for the field of
Big Data Science. The mini dissertation further goes on to explain the building blocks of a memory allocator and creates a benchmark suite for the task of evaluating the
proposed memory allocator.

A small object memory allocator is introduced in this mini-disseratation with the goal of improving the runtime performance of big data science tasks with a focus in algorithms
used in big data science. The memory allocator is based around the `mmap` system call and focuses on small objects by making use of a bin memory allcoator design.
The merit of the proposed memory allocator is shown through the use of a number of artificial benchmarks as well as a real world benchmark making use of
the genetic programming algorithm. The allocator is compared to the default system allocator on both MacOS and Linux (Ubuntu 25). The presented allocator is able to perform
on par or better than the default system allocator on both Linux and MacOS in the artificial benchmarks. With regards to the non-artificial benchmark the presented
allocator performs better than the default memory allocator on MacOS and is similar in performance on Linux but is worse and there is room for improvement. The proposed memory
allocator proves that building custom memory allocators for the field of data science improves runtime performance when compared to the default system memory allocators
which are most often used.

Further this mini-dissertation assists in addressing the issue pertaining to the lack of formal research on the topics of "Small Object Memory Allocators" as well as
"Custom Memory Allocators for te Field of Big Data Science".
