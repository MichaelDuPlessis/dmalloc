_default:
    @just --list

# builds the program
build:
    gcc -g -o main main.c src/*.c test/*.c

# builds and runs the program
run: build
    ./main

# deletes all build artifacts
clean:
    rm main
