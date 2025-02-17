# FanC Compiler
### The compilers frontend stages
- Lexical Analysis - Flex
- Syntax Analysis - Bison
- Semantic Analysis - Visitor Design
- Intermediate Code Generation - LLVM

## Build and run compiler
**Make use of the Makefile for an easy build**
```sh
Make
```
**Write a program of your choosing in FacC syntax**
```c
int fib(int n) {
        if (n == 0)
            return 0;
        else if (n == 1 or n == 2)
            return 1;
        else
            return fib(n - 1) + fib(n - 2);
}

void main() {
        print("This code was compiled by FanC Compiler!");
        printi(fib(5));
}
```
**Generate to llvm code and Run**
```sh
./compiler < fanC.code | lli
```
**output**
```
This code was compiled by FanC Compiler!
5
```
