# Introduction
Single instruction, multiple data (SIMD) is a kind of computing method that enables the processing of multiple data within the same operation. By using this method, the programmers can utilize the special hardware design to accelerate the processing. Matrix-matrix multiplication, one of the most important data processing kernels, is one kind of operation which can be accelerated by using SIMD. In this mini-project, the team implements the SIMD programming and cache access optimization in the code to greatly reduce the processing time of large scale matrix-matrix multiplication (from $1000\times1000$ matrix to $10000\times10000$ matrix) and compares the results with the convention methods without any intentional optimization. The solution supports the configurable matrix size, even with size larger than the on-chip cache capacity and both fixed-point and floating-point data. 

# Design





# Usage
Linux is required to run both the traditional and optimized code. 
- To compile the traditional code, run
`g++ [Traditional_XXX.cpp] -o [Output_File_Name.out]` and `./Output_File_Name.out` in the terminal. The execution of the traditional code does not require specific SIMD instruction set.
- To compile the optimized, run `g++ -mavx2 [Optimized_XXX.cpp] -o [Output_File_Name.out]` and `./Output_File_Name.out` in the terminal. AVX2 is required to run the code.

# Results


