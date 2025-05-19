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
    clang -O3 -o ./benchmark/bench src/*.c test/*.c benchmark/*.c

# benchmarks the program
bench: buildbench
    ./benchmark/bench

# deletes all build artifacts
clean:
    rm main
