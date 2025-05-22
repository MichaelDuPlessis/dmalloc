_default:
    @just --list

# builds the program
build:
    clang -g -o main main.c src/*.c test/*.c benchmark/*.c

# builds the program with optimizations
buildfast:
    clang -O3 -o main main.c src/*.c test/*.c benchmark/*.c

# builds and runs the program
run: build
    ./main

# builds and runs the program with optimizations
runfast: buildfast
    ./main

# compiles the benchmark
buildbench:
    clang -O3 -o ./bench src/*.c test/*.c benchmark/*.c

# benchmarks the program
bench: buildbench
    ./bench

# counts the lines of code in the program not including the notebooks directory
count:
    cloc $(git ls-files) --by-file --exclude-dir=notebooks

# deletes all build artifacts
clean:
    rm main bench
