<h1 align="center">Parallel Programming Examples</h1>
<p align="center">
    <img src="https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white"
         alt="C">
</p>

# Description
This is a compilation of parallel programming examples which I have made and referenced with other sources.
Some of the concepts will have diagrams to explain the algorithm.

- `counter/` - showcases race condition when multiple threads are incrementing a shared variable
- `hello-world/` - showcases race condition when passing a variable pointer to the thread argument.
- `producer-consumer/` - showcases race condition when reading data while another thread is writing to it.
- `jacobi/` - showcases the use of barriers to provide thread synchronization
- `coursework/` - parallel prefix sum algorithm and the use of barriers for synchronizing between algorithm phases.
- `mpi/` - uses message passing to divide bag of tasks among a fixed number of threads, and adapting MPI code in finding prime numbers.

## MPI Task Allocation Diagram
<p align="center">
    <img src="images/mpi-task-allocation.png"
         alt="Task Allocation with MPI">
</p>

# Running the code
```bash

# compiling c file
gcc <filename>.c -o test
# run c program
./test

# for mpi, install the required libary
sudo apt install build-essential
sudo apt install openmpi-bin libopenmpi-dev

# check binary
which mpic++

# set this to compiler path in .vscode/c_cpp_properties.json

# compile mpi code
mpic++ mpi.c -o mpi.exe
# run mpi program with 4 processes
mpirun -np 4 ./mpi.exe
```
