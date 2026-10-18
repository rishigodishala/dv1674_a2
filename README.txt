DV1674 Assignment 2 - Group 7

Final implementations: blur/ and pearson/
Build both applications: make clean && make

Example commands, from the repository root:
./blur/blur_par 15 blur/data/im1.ppm blur/data_o/im1.ppm 4
./pearson/pearson_par pearson/data/128.data pearson/data_o/128.data 4

The last argument specifies the number of threads at runtime.
Replace 4 with the desired positive thread count.
A thread count of 1 runs the optimized sequential algorithm.
Blur performance measurements use radius 15.

Both application directories contain standalone Makefiles.
Each builds its sequential and parallel executables.
The final build requires g++, gcc, make and pthreads.

Correctness checks:
bash blur/verify.sh
bash pearson/verify.sh

optimization_steps/ preserves baseline, opt1, opt2 and opt3 source code in experimental order. Input data are included only in the final application directories. To rebuild an earlier stage, use make CXX=g++ in its application directory.

Performance measurements were collected on an Ubuntu ARM64 VM with four virtual CPUs, not on the Intel Xeon grader.
