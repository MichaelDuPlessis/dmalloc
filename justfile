_default:
    @just --list

# builds the program
build:
    gcc main.c -o main

# builds and runs the program
run: build
    ./main
