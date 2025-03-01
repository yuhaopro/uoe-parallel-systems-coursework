// PPLS Exercise 1 Starter File
//
// See the exercise sheet for details
//
// Note that NITEMS, NTHREADS and SHOWDATA should
// be defined at compile time with -D options to gcc.
// They are the array length to use, number of threads to use
// and whether or not to printout array contents (which is
// useful for debugging, but not a good idea for large arrays).


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h> // in case you want semaphores, but you don't really
                       // need them for this exercise

#define SHOWDATA 1
#define NITEMS 20
#define NTHREADS 8


void* worker(void* arg);
void barrier_init();
void* worker_0(void* arg);
void barrier();

// Print a helpful message followed by the contents of an array
// Controlled by the value of SHOWDATA, which should be defined
// at compile time. Useful for debugging.
void showdata (char *message,  int *data,  int n) {
  int i; 

if (SHOWDATA) {
    printf ("%s", message);
    for (i=0; i<n; i++ ){
     printf (" %d", data[i]);
    }
    printf("\n");
  }
}

// Check that the contents of two integer arrays of the same length are equal
// and return a C-style boolean
int checkresult (int* correctresult,  int *data,  int n) {
  int i; 

  for (i=0; i<n; i++ ){
    if (data[i] != correctresult[i]) return 0;
  }
  return 1;
}

// Compute the prefix sum of an array **in place** sequentially
void sequentialprefixsum (int *data, int n) {
  int i;

  for (i=1; i<n; i++ ) {
    data[i] = data[i] + data[i-1];
  }
}


/**
 * Stage 1. Worker threads will obtain the chunks (start - end) position in the array. compute the prefix sum from index 1 of the chunk. Barrier needs to be added to ensure all threads process finish the chunks. Once all threads reach, it should signal to main thread to start executing stage 2.
 * 
 * Stage 2. thread 0 will do the update for each chunk index end by summing the prev chunk end (synchronous)
 * 
 * Stage 3. all other threads (exclude thread 0) will recompute the chunk from index start - (end-1) using the previous chunk index end.
 */

typedef struct worker_params {
  int worker_id;
  int start;
  int end;
  int size;
  int *data;
 } worker_params;

struct BarrierData {
  pthread_mutex_t barrier_mutex;
  pthread_cond_t barrier_cond;
  int nthread; // Number of threads that have reached this round of the barrier
  int round;   // Barrier round id
} bstate;

pthread_cond_t worker0_must_be_last_thread_cond;

void parallelprefixsum (int *data, int n) {
  // thread creation, size of array wouldn't change over time
  pthread_t threads[NTHREADS];
  worker_params args[NTHREADS];
  int chunk_slice = NITEMS / NTHREADS; // 10 / 4 = 2 
  int chunk_first_index = 0;

  barrier_init(); // initialize reusable barrier
  pthread_cond_init(&worker0_must_be_last_thread_cond, NULL);

  for (int id = 0; id < NTHREADS; id++) {
    // initialize worker params
    args[id].start = chunk_first_index; // 0
    int next_chunk_first_index = chunk_first_index + chunk_slice;

    // last thread takes the surplus
    if (id == NTHREADS - 1) {
      next_chunk_first_index = NITEMS;
    }
    args[id].end = next_chunk_first_index; // 1
    args[id].size = next_chunk_first_index - chunk_first_index;
    args[id].data = data;
    args[id].worker_id = id;
    chunk_first_index = next_chunk_first_index;

    if (id == 0) {
      pthread_create(&threads[id], NULL, worker_0, (void *) &args[id]);
    } else {
      pthread_create(&threads[id], NULL, worker, (void *) &args[id]);
    }

  }

  for (int id = 0; id < NTHREADS; id++) {
    pthread_join(threads[id], NULL);
  }

}

void* worker_0(void* arg) {
  worker_params *worker_info = (worker_params *) arg;

  for (int i = worker_info->start+1; i < worker_info->end; i++) {
    worker_info->data[i] = worker_info->data[i] + worker_info->data[i-1];
  }
  barrier();
  // phase 2, needs to be the last to hold the lock
  pthread_mutex_lock(&bstate.barrier_mutex);
  bstate.nthread += 1;
  if (bstate.nthread != NTHREADS) { // if this isn't the last thread to complete, it should wait 
    pthread_cond_wait(&worker0_must_be_last_thread_cond, &bstate.barrier_mutex);
  }
  // printf("worker %i (pthread id %ld) is performing sequential prefix sum...\n", worker_info->worker_id, pthread_self());
  // showdata("data array [after phase 1]: ", worker_info->data, NITEMS);
  // worker 0 has entered the monitor, performs phase 2 work
  int slice = worker_info->size;
  int chunk_last_index = slice - 1;
  for (int i = 1; i < NTHREADS; i++) // NTHREADS - 1 chunks since first chunk doesn't need to update
  {
    int cur_chunk_value = (worker_info->data)[chunk_last_index]; // eg. chunk 0 value
    int next_chunk_index =chunk_last_index + slice;
    int next_chunk_value =  (worker_info->data)[next_chunk_index]; // eg. chunk 1 value

    if (i == NTHREADS - 1) { // last chunk
      next_chunk_index = NITEMS - 1;
      next_chunk_value = (worker_info->data)[next_chunk_index]; // last item in array
    }

    int new_next_chunk_value = cur_chunk_value + next_chunk_value; // eg. compute chunk 1 value with chunk 0
    worker_info->data[next_chunk_index] = new_next_chunk_value; // updates chunk 1
    chunk_last_index = next_chunk_index;
    // showdata("data array [during phase 2]: ", worker_info->data, NITEMS);
  }

  bstate.round += 1; // to indicate that other threads can exit the while condition after waking up
  pthread_cond_broadcast(&bstate.barrier_cond); // wakes up rest of the threads once work is done
  pthread_mutex_unlock(&bstate.barrier_mutex); // worker 0 thread is done

}

void* worker(void* arg) {
  worker_params *worker_info = (worker_params *) arg;

  for (int i = worker_info->start+1; i < worker_info->end; i++) {
    worker_info->data[i] = worker_info->data[i] + worker_info->data[i-1];
  }

  // wait for all threads to finish here
  barrier();
  // showdata("After Phase 1: ", worker_info->data, NITEMS);

  // phase 2
  pthread_mutex_lock(&bstate.barrier_mutex);
  bstate.nthread += 1;
  if (bstate.nthread == NTHREADS) { // this is the last thread to enter
    pthread_cond_broadcast(&worker0_must_be_last_thread_cond); // wake up worker 0

  }
  int current_round = bstate.round;
  do
  {
    // printf("worker %i (pthread id %ld) is going to wait in phase 2...\n", worker_info->worker_id, pthread_self());
    pthread_cond_wait(&bstate.barrier_cond, &bstate.barrier_mutex);
  } while (bstate.round == current_round);
  pthread_mutex_unlock(&bstate.barrier_mutex);
  int cur_chunk_start_idx = worker_info->start;
  int prev_chunk_last_val = worker_info->data[cur_chunk_start_idx - 1];

  // all other threads need to compute from their first chunk index to the second last index.
  for (int i = cur_chunk_start_idx; i < worker_info->end - 1; i++) // omit the last indexed value as it was calculated in the second phase
  {
    worker_info->data[i] = worker_info->data[i] + prev_chunk_last_val;
  }
  // showdata("data array [during phase 3]: ", worker_info->data, NITEMS);

}

void barrier_init() {
  pthread_mutex_init(&bstate.barrier_mutex, NULL);
  pthread_cond_init(&bstate.barrier_cond, NULL);
  bstate.nthread = 0;
}


void barrier() {
  // only 1 thread can come into the barrier at a time
  // if not they will be blocked here
  pthread_mutex_lock(&bstate.barrier_mutex);
  bstate.nthread += 1;

  if (bstate.nthread == NTHREADS) {
    bstate.round += 1;
    bstate.nthread = 0;
    pthread_cond_broadcast(&bstate.barrier_cond); // wakes up all threads waiting
  } else {
    int current_round = bstate.round;

    do
    {
      pthread_cond_wait(&bstate.barrier_cond, &bstate.barrier_mutex);
    } while (bstate.round == current_round); // prevents spurious wakeups
  }
  pthread_mutex_unlock(&bstate.barrier_mutex);
}




int main (int argc, char* argv[]) {

  int *arr1, *arr2, i;

  // Check that the compile time constants are sensible
  if ((NITEMS>10000000) || (NTHREADS>32)) {
    printf ("So much data or so many threads may not be a good idea! .... exiting\n");
    exit(EXIT_FAILURE);
  }

  // Create two copies of some random data
  arr1 = (int *) malloc(NITEMS*sizeof(int));
  arr2 = (int *) malloc(NITEMS*sizeof(int));
  srand((int)time(NULL));
  for (i=0; i<NITEMS; i++) {
     arr1[i] = arr2[i] = rand()%5;
  }
  showdata ("initial data          : ", arr1, NITEMS);

  // Calculate prefix sum sequentially, to check against later on
  sequentialprefixsum (arr1, NITEMS);
  showdata ("sequential prefix sum : ", arr1, NITEMS);

  // Calculate prefix sum in parallel on the other copy of the original data
  parallelprefixsum (arr2, NITEMS);
  showdata ("parallel prefix sum   : ", arr2, NITEMS);

  // Check that the sequential and parallel results match
  if (checkresult(arr1, arr2, NITEMS))  {
    printf("Well done, the sequential and parallel prefix sum arrays match.\n");
  } else {
    printf("Error: The sequential and parallel prefix sum arrays don't match.\n");
  }

  free(arr1); free(arr2);
  return 0;
}
