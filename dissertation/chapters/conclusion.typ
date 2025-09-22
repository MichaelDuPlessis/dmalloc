= Conclusion

A small object memory allocator is introduced in this paper with the goal of improving the runtime performance of big data science tasks with a focus in algorithms used in
big data science. The presented allocator is able to perform on par or better than the default system allocator on both Linux and MacOS in artificial benchmarks. With regards
to a non-artificial benchmark the presented allocator performs better than the default memory allocator on MacOS and is similar in performance on Linux but is worse and there
is room for improvement. The allocator is based around the unix `mmap` system call and shows that there is merit in developing specialised memory allocators in the field of
data science. A faster memory allocator allows all big data science programs to run faster and further research into this topic in the field can yield further improvements.
