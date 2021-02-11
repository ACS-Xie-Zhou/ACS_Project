# Introduction
Single instruction, multiple data (SIMD) is a kind of computing method that enables the processing of multiple data within the same operation. By using this method, the programmers can utilize the special hardware design to accelerate the processing. Matrix-matrix multiplication, one of the most important data processing kernels, is one kind of operation which can be accelerated by using SIMD. In this mini-project, the team implements the SIMD programming and cache access optimization in the code to greatly reduce the processing time of large scale matrix-matrix multiplication (from 1000×1000 matrix to 10000×10000 matrix) and compares the results with the convention methods without any intentional optimization. The solution supports the configurable matrix size, even with size larger than the on-chip cache capacity and both fixed-point and floating-point data. 

# Design





# Usage
The code may be compiled on Windows operating system using Mingw's g++ program. However, during the execution phase, the code will not run correctly. Therefore, Linux is required to run both the traditional and optimized code. 
- To compile the traditional code, run
`g++ [Traditional_XXX.cpp] -o [Output_File_Name.out]` and `./Output_File_Name.out` in the terminal. The execution of the traditional code does not require specific SIMD instruction set.
- To compile the optimized, run `g++ -mavx2 [Optimized_XXX.cpp] -o [Output_File_Name.out]` and `./Output_File_Name.out` in the terminal. AVX2 is required to run the code.

The code will then automatically run the multiplication on different sizes of matrices, from 1000×1000 to 10000×10000. For the optimized code, depending on the system performance, the multiplication of 10000×10000 matrix should take less than 500 seconds. For the traditional code, the multiplication of 10000×10000 matrix can take several hours.

# Results
For the 2-byte fixed-point matrix multiplication, the results are shown below:
| Matrix Size | Traditional (sec) | Optimized (sec)
| ----------- | ----------- | ----------|
| 1000×1000 | 6.882 | 0.271 |
| 2000×2000 | 85.85 | 2.248 |
| 3000×3000 | 333.018 | 7.692 |
| 4000×4000 | 789.918 | 18.297 |
| 5000×5000 | 1627.98 | 35.773 |
| 6000×6000 | 2856.81 | 61.57 |
| 7000×7000 | 4621.21 | 99.035 |
| 8000×8000 | 7539.79 | 147.494 |
| 9000×9000 | 10565.2 | 207.581 |
| 10000×10000 | 15214.2 | 284.932 |

For the 4-byte floating-point matrix multiplication, the results are shown below:
| Matrix Size | Traditional (sec) | Optimized (sec)
| ----------- | ----------- | ----------|
| 1000×1000 | 12.996 | 0.288 |
| 2000×2000 | 143.416 | 2.309 |
| 3000×3000 | 525.691 | 7.606 |
| 4000×4000 | 1291 | 18.337 |
| 5000×5000 | 2501.49 | 35.377 |
| 6000×6000 | 4548.29 | 61.181 |
| 7000×7000 | 7672.94 | 98.059 |
| 8000×8000 | 12690.1 | 145.061 |
| 9000×9000 | 15473.2 | 208.056 |
| 10000×10000 | 21847.8 | 284.272 |





