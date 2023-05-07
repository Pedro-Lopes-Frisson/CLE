//
// Created by Pedro1 on 3/23/23.
//

#include <getopt.h>

#include "sharedRegion.h"
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>

void help(char *program_name);

static void *worker(void *args);

static void *distributor(void *args);


int main(int argc, char **argv) {

  int opt, n_workers;
  while ((opt = getopt(argc, argv, "ht:")) != -1) {
    switch (opt) {
      case 't':
        n_workers = (int) atoi(optarg);
        if (n_workers <= 0 ){
          fprintf(stderr, "%s: non positive number less than 20\n", basename(argv[0]));
          return EXIT_FAILURE;
        }
        break;
      case 'h':
        help(basename(argv[0]));
        exit(EXIT_SUCCESS);
        break;
      default: /* '?' */
        help(basename(argv[0]));
        exit(EXIT_FAILURE);
    }
  }

  printf("N WORKERS:a %d\n", n_workers);
  // only thing that we have left is the arguments with no flags let's assume they are all filenames
  int n_files = argc - optind;

  if (n_files <= 0) {
    fprintf(stderr, "It would be nice to provide files to be accounted for.\n");
    help(basename(argv[0]));
    exit(EXIT_SUCCESS);
  }


  // create threads
  //
  //
  //
  pthread_t tIdWork[n_workers]; /* workers internal thread id array */
  pthread_t distributor_t;
  unsigned int work[n_workers]; /* workers application defined thread id array */
  int *status_p;                /* pointer to execution status */

  int t;

  for (int t = 0; t < n_workers; t++) work[t] = t;

  store_info(n_workers, argv[optind]);
  printf("File names stored in the shared region.\n");


  if (pthread_create(&distributor_t, NULL, distributor, NULL) !=
      0) /* create thread worker */
  {
    perror("error on creating thread distributor");
    exit(EXIT_FAILURE);
  }

  struct timespec start_time, end_time;
  double elapsed_time;

  clock_gettime(CLOCK_MONOTONIC, &start_time);

  for (t = 0; t < n_workers; t++) {
    if (pthread_create(&tIdWork[t], NULL, worker, &work[t]) !=
        0) /* create thread worker */
    {
      perror("error on creating thread producer");
      exit(EXIT_FAILURE);
    }
  }


  printf("Workers created and start processing.\n");


  if (pthread_join(distributor_t, NULL) !=
      0) /* wait for thread worker to terminate */
  {
    perror("error on waiting for thread producer");
    exit(EXIT_FAILURE);
  }

  printf("thread worker, with id  1111111111, has terminated. \n");
  fprintf(stdout, "thread worker, with id  has terminated. \n");
  for (t = 0; t < n_workers; t++) {
    if (pthread_join(tIdWork[t], (void *) &status_p) !=
        0) /* wait for thread worker to terminate */
    {
      perror("error on waiting for thread producer");
      exit(EXIT_FAILURE);
    }
    printf("thread worker, with id %u, has terminated. \n", t);
    fprintf(stdout, "thread worker, with id %u, has terminated. \n", t);
  }
  clock_gettime(CLOCK_MONOTONIC, &end_time);

  elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1e9;// Convert seconds to nanoseconds
  elapsed_time += (end_time.tv_nsec - start_time.tv_nsec);

  printf("Elapsed time: %f nanoseconds\n", elapsed_time);
  validate();


  exit(EXIT_SUCCESS);
}


void bitonic_sort(int *arr_1, int n, int flag) {
  // the two first loops are the merging part
  // the last one is the sorting part
  for (int k = 2; k <= n; k = k * 2) {
    // sort the subsequence of k members to bitonic sequences
    for (int j = k / 2; j > 0; j = j / 2) {
      // Sort bitonic sequence of length k according to direction needed
      for (int i = 0; i < n; i++) {
        // sort elements
        int ij = i ^ j;// get the other index to be sorted
        if (ij > i) {
          int dir = ((i & k) == 0);// get direction
          //printf("k %d,j %d,i %d,ij %d,dir %d\n",k,j,i,ij,dir);
          //CAPS
          //Ascending
          if (flag == 0) {
            if ((arr_1[i] > arr_1[ij]) == dir) {
              int temp = arr_1[i];
              arr_1[i] = arr_1[ij];
              arr_1[ij] = temp;
            }
          }
          // Descending
          else {
            if ((arr_1[i] < arr_1[ij]) == dir) {
              int temp = arr_1[i];
              arr_1[i] = arr_1[ij];
              arr_1[ij] = temp;
            }
          }
          //END CAPS
        }
      }
    }
  }
}

void merge(int *arr_1, int n, int flag) {
  int k = n;
  for (int j = k / 2; j > 0; j = j / 2) {
    // Sort bitonic sequence of length k according to direction needed
    for (int i = 0; i < n; i++) {
      // sort elements
      int ij = i ^ j;// get the other index to be sorted
      if (ij > i) {
        // int dir = ((i & k) == 0); // get direction
        //printf("k %d,j %d,i %d,ij %d,dir %d\n",k,j,i,ij,dir);
        //CAPS
        // Ascending
        if (flag == 0) {
          // printf("%i → ", dir);
          if ((arr_1[i] > arr_1[ij])) {
            // printf("→→[%i|%i]",arr_1[i],arr_1[ij]);
            int temp = arr_1[i];
            arr_1[i] = arr_1[ij];
            arr_1[ij] = temp;
          }
        }
        // Descending
        else {
          if ((arr_1[i] < arr_1[ij])) {
            int temp = arr_1[i];
            arr_1[i] = arr_1[ij];
            arr_1[ij] = temp;
          }
        }
        //END CAPS
      }
    }
    k = k / 2;
  }
}

static void *worker(void *args) {

  unsigned int id = *((unsigned int *) args); /* worker id */

  struct WORK_CHUNK *wc = (struct WORK_CHUNK *) malloc(
      sizeof(struct WORK_CHUNK)); /* struct to store partial info of current file being processed */

  while (get_work_chunk(wc) == true) { /* while data available */
    if (wc->iteration == 0) bitonic_sort(wc->begin, wc->size, wc->dir_flag); /*sort data*/
    else
      merge(wc->begin, wc->size, wc->dir_flag); /*merge data*/
    work_done();
  }

  int status = EXIT_SUCCESS;
  free(wc);
  pthread_exit(&status);
}

void help(char *program_name) {
  fprintf(stderr, "Usage: %s -t [ integer between 1 and 10 ] [file1 file2 file3 ...]\n", program_name);
}


static void *distributor(void *args) {
  read_file();
  distribute();
  int status = EXIT_SUCCESS;
  pthread_exit(&status);
}
