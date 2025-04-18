<h1 align="center">Parallel Programming Examples</h1>
<p align="center">
    <img src="https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white"
         alt="C">
</p>

# Description
This is a compilation of parallel programming examples which I have made and referenced with other sources.

- `*Counter.c` - showcases race condition when multiple threads are incrementing a shared variable
- `*HelloWorld.c` - showcases race condition when passing a variable pointer to the thread argument.
- `*ProducerConsumer.c` - showcases race condition when reading data while another thread is writing to it.
- `jacobi.c` - showcases the use of barriers to provide thread synchronization
- `coursework/` - parallel prefix sum algorithm and the use of barriers for synchronizing between algorithm phases.
- `mpi.c` - uses message passing to divide bag of tasks among a fixed number of threads.
# Running the code
```bash
gcc <filename>.c -o test
./test

# for mpi, install the required libary
sudo apt install build-essential
sudo apt install openmpi-bin libopenmpi-dev

# check binary
which mpic++

# set this to compiler path in .vscode/c_cpp_properties.json
```
