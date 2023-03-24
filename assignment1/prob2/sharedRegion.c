#include "sharedRegion.h"
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/**
 *  \brief Initialization of the data transfer region.
 *  Internal monitor operation.
 */

void initialization(void) {
  pthread_cond_init(&wait_for_work, NULL);
  pthread_cond_init(&wait_work_done, NULL);
  pthread_cond_init(&workers_ready, NULL);
  it = 0;
  flip = 0;
  isOver = false;
  setlocale(LC_CTYPE, "");
}


void store_info(int n, char *file) {


  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  n_workers = n;

  file_name = (char *) malloc(strlen(file) * sizeof(char));

  if (file_name == NULL) {
    int status = EXIT_FAILURE;
    fprintf(stderr, "Memory allocation for file_names failed\n");
    pthread_exit(&status);
  }

  strcpy(file_name, file);

  wc_chunks = (struct WORK_CHUNK *) malloc(sizeof(struct WORK_CHUNK) * n_workers);
  iterations_required = (int) log2(n_workers) + 1;

  if (wc_chunks == NULL) {
    int status = EXIT_FAILURE;
    fprintf(stderr, "Memory allocation for file_statistics failed\n");

    pthread_exit(&status);
  }


  pthread_once(&init, initialization);

  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
}
void read_file() {
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  f = fopen(file_name, "rb");
  size_t res;
  if (f == NULL) {
    fprintf(stderr, "File could not be opened %s.\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
  res = fread(&number_of_values, sizeof(int), 1, f);
  if (res == 0) {
    fprintf(stderr, "Error reading Size of array\n");
    exit(EXIT_FAILURE);
  }
  //fprintf(stdout, "Number %d\n", number_of_values / (1024));

  values = (int *) malloc(sizeof(int) * number_of_values);

  for (int i = 0; i < number_of_values; i++) {
    res = fread(&(values[i]), sizeof(int), 1, f);
    if (res == 0) {
      fprintf(stderr, "Error reading value of array\n");
      exit(EXIT_FAILURE);
    }

    //fprintf(stdout, "[%d]\t %d\n",i, values[i]);
  }

  wc_chunks = (struct WORK_CHUNK *) malloc(sizeof(struct WORK_CHUNK) * n_workers);
  if (wc_chunks == NULL) {
    fprintf(stderr, "Error allocating memory for wc_chunks\n");
    int status = EXIT_FAILURE;
    exit(status);
  }

  n_workers_needed = n_workers;
  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
}

void distribute() {
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  int offset;

  while (n_workers_ready_to_work != n_workers_needed) {
    if ((pthread_cond_wait(&workers_ready, &accessCR)) != 0) {
      perror("Signal failed! workers_ready"); /* save error in errno */
      int status = EXIT_FAILURE;
      pthread_exit(&status);
    }
  }

  for (; it < iterations_required; it++) {
    n_workers_needed = ((n_workers) / (int) pow(2, it));
    fprintf(stdout, "needed. %d. ready: %d\n", n_workers_needed, n_workers_ready_to_work);
    // wait for workers to start wanting to work

    // increment wc_chunks_count
    for (int j = 0; j < n_workers_needed; j++) {
      wc_chunks_count++;

      // send signal for worker who are waiting for work
      // implicit release of lock
      if ((pthread_cond_signal(&wait_for_work)) != 0) {
        perror("Signal failed! wait_for_work"); /* save error in errno */
        int status = EXIT_FAILURE;
        pthread_exit(&status);
      }
    }

    // wait for workers to finish what they are doing
    // n_workers_needed = the workers he sent on a mission
    while (n_workers_work_is_done != n_workers_needed) {
      if ((pthread_cond_wait(&wait_work_done, &accessCR)) != 0) {
        perror("Signal failed! wait_for_work"); /* save error in errno */
        int status = EXIT_FAILURE;
        pthread_exit(&status);
      }
    }
    n_workers_work_is_done = 0;
    wc_chunks_count = 0;
    n_workers_ready_to_work = 0;
    fprintf(stdout, "it %d is done\n", it);
  }

  isOver = true;
  wc_chunks_count = -12314124;

  for (int i = 0; i < n_workers; i++) {
    if ((pthread_cond_signal(&wait_for_work)) != 0) {
      perror("Signal failed! wait_for_work"); /* save error in errno */
      int status = EXIT_FAILURE;
      pthread_exit(&status);
    }
  }
  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
}

bool get_work_chunk(struct WORK_CHUNK *wc) {

  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  n_workers_ready_to_work++;

  if ((pthread_cond_signal(&workers_ready)) != 0) {
    perror("Signal failed! wait_for_work"); /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }

  fprintf(stdout, "%s because chunks_count %d\n\nTHREAD: %lu\n\n", wc_chunks_count == 0 ? "W" : "G", wc_chunks_count, pthread_self());

  while (wc_chunks_count <= 0) {

    if ((pthread_cond_wait(&wait_for_work, &accessCR)) != 0) {
      perror("Signal failed! wait_for_work"); /* save error in errno */
      int status = EXIT_FAILURE;
      pthread_exit(&status);
    }
    fprintf(stdout, "wc_chunks_count %d\n", wc_chunks_count);
    if (isOver) {
      fprintf(stdout, "Entrei is over\n");
      // release lock
      if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
        perror("error on exiting monitor(CF)");     /* save error in errno */
        int status = EXIT_FAILURE;
        pthread_exit(&status);
      }
      return false;
    }
  }

  fprintf(stdout, "%s because chunks_count %d\n\n\n\n", wc_chunks_count == 0 ? "W" : "G", wc_chunks_count);

  // get the pointer
  int j = n_workers_needed - wc_chunks_count--;
  int offset = (number_of_values / n_workers_needed) * j;

  wc->begin = &(values[offset]);
  wc->dir_flag = (j % 2) == flip;
  wc->iteration = it;
  wc->size = number_of_values / n_workers_needed;
  if (j % 2 == 0) flip = !flip;


  // release lock
  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  return true;
}

void work_done() {
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  n_workers_work_is_done++;
  printf("N is done : %d by %lu\n", n_workers_work_is_done, pthread_self());
  // signal the distributor that we are done with the task
  if ((pthread_cond_signal(&wait_work_done)) != 0) {
    perror("Signal failed! wait_for_work"); /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  // release lock
  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
}

void validate() {
  if ((pthread_mutex_lock(&accessCR)) != 0) { /* enter monitor */
    perror("error on entering monitor(CF)");  /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
  for (int i = 0; i < number_of_values - 1; i++) {
    if (values[i] > values[i + 1]) {
      fprintf(stderr, "Values are not ordered %d, %d\n", values[i], values[i + 1]);
      break;
    } else {
      fprintf(stdout, "Values are ordered\n");
    }
  }

  // release lock
  if ((pthread_mutex_unlock(&accessCR)) != 0) { /* exit monitor */
    perror("error on exiting monitor(CF)");     /* save error in errno */
    int status = EXIT_FAILURE;
    pthread_exit(&status);
  }
}
