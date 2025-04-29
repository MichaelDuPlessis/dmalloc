_default:
    @just --list

# builds the program
build:
    gcc -g -o main main.c src/*.c test/*.c

# builds the program with optimizations
buildfast:
    gcc -O3 -o main main.c src/*.c test/*.c

# builds and runs the program
run: build
    ./main

# builds and runs the program with optimizations
runfast: buildfast
    ./main

# deletes all build artifacts
clean:
    rm main
