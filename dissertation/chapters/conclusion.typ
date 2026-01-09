= Conclusion

This mini-dissertation explained the importance of custom memory allocators and the performance improvements they can bring. It further points out the lack of formal research in
developing memory allocators for the field of Big Data Science and aims to address this issue by developing a memory allocator specialising in small objects for the field of
Big Data Science. The mini dissertation further goes on to explain the building blocks of a memory allocator and creates a benchmark suite for the task of evaluating the
proposed memory allocator.

A small object memory allocator is introduced in this mini-dissertation with the goal of improving the runtime performance of big data science tasks with a focus in algorithms
used in big data science. The memory allocator is based around the `mmap` system call and focuses on small objects by making use of a bin memory allocator design.
The merit of the proposed memory allocator is shown through the use of a number of artificial benchmarks as well as a real world benchmark making use of
the genetic programming algorithm. The allocator is compared to the default system allocator on both MacOS and Linux (Ubuntu 25). The presented allocator is able to perform
on par or better than the default system allocator on both Linux and MacOS in the artificial benchmarks. With regards to the non-artificial benchmark, the presented
allocator performs better than the default memory allocator on MacOS and is similar in performance on Linux. For both MacOS and Linux there is room for improvement. The proposed memory
allocator shows that building custom memory allocators for the field of data science improves runtime performance when compared to the default system memory allocators
which are most often used.

Further this mini-dissertation assists in addressing the issue pertaining to the lack of formal research on the topics of "Small Object Memory Allocators" as well as
"Custom Memory Allocators for the Field of Big Data Science".

Currently the allocator only works on Unix based systems, this is due to its reliance on the `mmap` system call. The Windows operating system does have a `mmap`
equivalent so this support can be trivially implemented using conditional compilation. Once this has been done the benchmark tests will need to be rerun to
determine how the allocator performs on Windows.
The allocator will not work in multithreaded environments as there is no protection against concurrent memory access on allocated pages. A solution to fix this
in a non-blocking manner would be to have each page have a lock on it, or an atomic boolean as locks slow code down. Then when a thread tries to allocate
memory it can check if another thread is currently using that page. If that page is currently in use the current thread can either use a different page or allocate
a new one. This way when, allocating memory, there will be no waiting by any thread. Reading can happen at any time since it is a non-destructive operation.
Further benchmarks as well as comparisons to other state of the art memory allocators can yield further insights into how beneficial this research is.

Overall this mini-dissertation clearly shows the importance of designing and building specialised tools for specific fields as well as the importance of memory allocators and
the usefullness of custom memory allocation techniques over using the defualt memory allocator. It further goes on to show how a custom memory allocator can be applied to
a common algorithm in the field of big data science and what performance improvements it can bring.
