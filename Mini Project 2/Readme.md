# Introduction
The requirement of the mini project is to design a program which can utilize multiple threads to accelerate the time required to compress a file. 

# Current Process
- [x] Read a file by the file name.
- [x] Put the file into the memory.
- [x] Split the file into 4KB blocks.
- [x] Send the file to different threads.
- [ ] Compress the file block using zlib.
- [x] Receive the file from different threads in order.
- [x] Write the file blocks into a new compressed file. 