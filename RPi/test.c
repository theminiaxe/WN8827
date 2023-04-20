//relies on the use of condition variables as explained at https://www.geeksforgeeks.org/condition-wait-signal-multi-threading/
//allowing for writer and/or reader to wait in the event that buffer is full or empty
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

//pthread_mutex_t read_mutex;
pthread_mutex_t write_mutex;
pthread_cond_t buffer_full_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer_empty_cond = PTHREAD_COND_INITIALIZER;
//integer representing the maximise size of the buffer
int maximumBufferSize = 10;
//char array representing the buffer to be written to and read from
char buffer[10];
//integer to more simply maintain info on contents of buffer
int currentBufferSize = 0;


void * writer(void *arg) {
  //printf("Debug - Entered writer");
  //define a char to take input from user
  char input;
  //operate indefintely
  while (1) {
    //now that an input has been read, lock buffer to prevent conflict between reader and writer
    pthread_mutex_lock(&write_mutex);
    //printf("Debug - Mutex' locked");
    //single shared resource, so I've removed the read_mutex used in lab example, as it also complicates use of the conditional waiting
    //pthread_mutex_lock(&read_mutex);
    input = getchar();
    //printf("Debug - Got char, %c\n", input);
    //while buffer is full wait for a signal indicating it is no longer full
    while (currentBufferSize >= maximumBufferSize) {
      //printf("Debug - maximumBufferSize exceeded - wait for signal from reader indicating space has been made available");
      pthread_cond_wait(&buffer_full_cond, &write_mutex);
      //printf("Debug - signal received, no longer waiting, condition in while loop will still be checked");
    }

    //add contents of input to buffer and increment size of buffer counter, array starts at, as does currentBufferSize, hence
    //the additional contents of the array can be added at currentBufferSize'th index
    buffer[currentBufferSize] = input;
    //increment currentBufferSize to keep track of addition to buffer
    currentBufferSize++;
    //printf("Debug - contents added to buffer, unlocking mutex");
    //printf("Debug - %s", buffer);
    
    //send signal - buffer no longer empty, i.e. if reader is waiting, when it runs again, it should be able to continue execution
    pthread_cond_signal(&buffer_empty_cond);
    //buffer modified as necessary by writer - unlock the mutex, read_mutex not in use - outlined in prior comment
    //pthread_mutex_unlock(&read_mutex);
    pthread_mutex_unlock(&write_mutex);
  }
  return 0;
}

void * reader(void *arg) {
  //printf("Debug - Entered Reader");
  //operate indefinitely
  char temp;
  while(1) {
    //lock the write_mutex, as we will immediately wish to read from buffer, and therefore require mutual exclusion
    pthread_mutex_lock(&write_mutex);
    //pthread_mutex_lock(&read_mutex);
    //printf("Debug - mutex locked");

    //while buffer is empty wait for it to have at least one char added
    while (currentBufferSize == 0) {
      //printf("Debug - buffer empty, waiting until contents are added");
      pthread_cond_wait(&buffer_empty_cond, &write_mutex);
      //printf("Debug - signal received indicating buffer no longer empty, while loop will be reevaluated");
    }

    //operating fifo, would probably be easier to just pop chars off the back of the array, but that seems counter to how I would expect an I/O buffer to operate
    //take the 0'th index char, and store in temp to be output later
    temp = buffer[0];
    //printf("Debug - %c, %c,", buffer[0], temp);
    //for loop, shift each char in array down an index - 
    for (int i = 1; i <= currentBufferSize; i++) {
      //printf("Debug - Bumping from buffer\n");
      buffer[i-1] = buffer[i];
    }
    //\0 what was previously the last value in the array before decrementing the buffer size counter
    buffer[currentBufferSize] = "\0";
    currentBufferSize--;
    /*Debug - Print the contents of the buffer
    for (int i = 0; i < (currentBufferSize-1); i++) {
        printf("%c, ", buffer[i]);
    }
    */
    printf("[%c:%d]\n", temp, currentBufferSize);
    //buffer has been decremented, indicating that buffer will no longer be full, ergo no longer need to wait
    pthread_cond_signal(&buffer_full_cond);
    //pthread_mutex_unlock(&read_mutex);
    //unlock the mutex
    pthread_mutex_unlock(&write_mutex);
  }
  return 0;
}

int main(void) {

  pthread_t thread_id_write,thread_id_read;
  if (pthread_create(&thread_id_write,NULL,&writer,NULL)){
    printf("error creating thread.");
    abort();
  }
  sleep(1);
  if (pthread_create(&thread_id_read,NULL,&reader,NULL)){
    printf("error creating thread.");
    abort();
  }
  else printf("\nThreads successflly created.");

  if (pthread_join(thread_id_write, NULL)){
    printf("error joining thread.");
    abort();
  }
  if (pthread_join(thread_id_read,NULL)){
    printf("error joining thread.");
    abort();
  }
}
