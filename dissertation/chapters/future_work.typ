= Future Work

There are three main aspects that can be expanded on in the future:

== Windows Support

Currently the allocator only works on Unix based systems this is due to its reliance on the `mmap` system call. The Windows operating system does have a `mmap`
equivalent so this support can be trivially implemented using conditional compilation. Once this has been done the benchmark tests will need to be rerun to
determine how the allocator performs on windows.

== Multithreading

The allocator will not work in multithreaded environments as there is no protection against concurrent memory access on allocated pages. A solution to fix this
in a non-blocking manner would be to have each page have a lock on it or better yet an atomic boolean as locks slow code down. Then when a thread tries to allocate
memory it can check if another thread is currently using that page and if that page is currently in use the current thread can either use a different page or allocate
a new one. This way when allocating memory there will be no waiting by any thread. Reading can happen at any time since it is a non-destructive operation.

== More Benchmarks

Further benchmarks as well as comparisons to other state of the art memory allocators can yield further insights into how beneficial this research is.
