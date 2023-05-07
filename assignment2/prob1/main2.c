//
// Created by Pedro1 on 5/6/23.
//



#include <mpi.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "dispatcher.h"
#include "structures.h"

#define TERMINATION_TAG 101
#define WORK_TO_DO 100


void update_status(int *flag, int *stats);

void reset_flags(int *inWord, int *has_vowel);

int get_available_worker(bool *worker_status, int size);

void process_data_chunk(struct FILE_CHUNK *fc);

int main(int argc, char **argv) {

  int rank, size;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  struct timespec start_time, end_time;
  double elapsed_time;

  clock_gettime(CLOCK_MONOTONIC, &start_time);
  if (rank == 0) {
    // dispatcher
    int opt;
    bool worker_status[size]; // status of each worker
    for (int i = 0; i < size; i++)
      worker_status[i] = true;
    MPI_Request recv_requests[size];
    struct FILE_CHUNK file_chunk_rcv[size];

    int sent_count, // count sent messages
    received_count = sent_count = 0; // count received messages


    while ((opt = getopt(argc, argv, "h")) != -1) {
      switch (opt) {
        case 'h':
          MPI_Finalize();
          exit(EXIT_SUCCESS);
        default: /* '?' */
          MPI_Finalize();
          exit(EXIT_FAILURE);
      }
    }
    initialization();

    //printf("N WORKERS:a %d\n", size);
    // only thing that we have left is the arguments with no flags let's assume they are all filenames
    int n_files = argc - optind;

    if (n_files <= 0) {
      fprintf(stderr, "It would be nice to provide files to be accounted for.\n");
      MPI_Finalize();
      exit(EXIT_SUCCESS);
    }

    int available_worker;
    int flag;
    MPI_Status status;
    store_file_names(n_files, &(argv[optind]));
    struct FILE_CHUNK file_chunk;
    while (get_data_chunk(&file_chunk, 0) == 0) {        /* while data available */

      // check for free workers
      //printf("Maybe everyone is still working\n");
      while ((available_worker = get_available_worker(worker_status, size)) == -1) {
        // Probe for completed work
        for (int k = 1; k < size; k++) {
          if (!worker_status[k]) {
            MPI_Test(&recv_requests[k], &flag, &status);
            //printf("Testing %d rank resulted in %d\n", k, flag);
            if (flag) {
              worker_status[k] = true;
              //MPI_Recv(&words[k], 1, MPI_INT, k, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
              save_partial_results((struct FILE_CHUNK *) &file_chunk_rcv[k]);
              //printf("Received completed work from %d result was: %d\n", k, words[k]);
              received_count++;
            }
          }
        }
        /*
        MPI_Isend(work, size, MPI_CHAR, status.MPI_SOURCE, 0, MPI_COMM_WORLD, &send_requests[status.MPI_SOURCE]);
        MPI_Irecv(&words[status.MPI_SOURCE], 1, MPI_INT, sent_count, 0, MPI_COMM_WORLD,
                  &recv_requests[status.MPI_SOURCE]);
        worker_status[status.MPI_SOURCE] = false;
        sent_count += 1;
         */
      }

      /*
      MPI_Isend((char *) &file_chunk, sizeof(struct FILE_CHUNK), MPI_BYTE, available_worker, 0, MPI_COMM_WORLD,
                &send_requests[available_worker]);
      */

      //printf("Sending work to %d buffer has \n", available_worker);
      MPI_Send((char *) &(file_chunk), sizeof(struct FILE_CHUNK), MPI_BYTE, available_worker, 0, MPI_COMM_WORLD);
      MPI_Irecv((char *) &file_chunk_rcv[available_worker], sizeof(struct FILE_CHUNK), MPI_BYTE, available_worker, 0,
                MPI_COMM_WORLD,
                &recv_requests[available_worker]);
      worker_status[available_worker] = false;
      sent_count += 1;

      available_worker = get_available_worker(worker_status, size);

      //TODO receive any result and save it
      // repeat
    }

    // all work was sent
    // receive any left message

    while (sent_count != received_count) {
      // Probe for completed work
      for (int k = 1; k < size; k++) {
        if (!worker_status[k]) {
          MPI_Test(&recv_requests[k], &flag, &status);
          if (flag) {
            worker_status[k] = true;
            //MPI_Recv(&words[k], 1, MPI_INT, k, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            save_partial_results((struct FILE_CHUNK *) &file_chunk_rcv[k]);
            //printf("Received completed work from %d result was: %d\n", k, words[k]);
            received_count++;
          }
        }
      }
      /*
      MPI_Isend(work, size, MPI_CHAR, status.MPI_SOURCE, 0, MPI_COMM_WORLD, &send_requests[status.MPI_SOURCE]);
      MPI_Irecv(&words[status.MPI_SOURCE], 1, MPI_INT, sent_count, 0, MPI_COMM_WORLD,
                &recv_requests[status.MPI_SOURCE]);
      worker_status[status.MPI_SOURCE] = false;
      sent_count += 1;
       */
    }


    for (int k = 1; k < size; k++) {
      worker_status[k] = true;
      MPI_Send((char *) &file_chunk, sizeof(struct FILE_CHUNK), MPI_BYTE, k, TERMINATION_TAG, MPI_COMM_WORLD);
      //printf("Sending done to every worker process %d\n", k);
      received_count++;
    }

    print_processing_results();
    printf("Done\n");
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1e9; // Convert seconds to nanoseconds
    elapsed_time += (end_time.tv_nsec - start_time.tv_nsec);

    printf("Elapsed time: %f nanoseconds\n", elapsed_time);

    MPI_Finalize();
    return EXIT_SUCCESS;

  } else {
    // workers
    struct FILE_CHUNK fc;
    MPI_Status status;
    srandom(rank);

    /*
    MPI_Irecv(&buffer, 1, MPI_CHAR, 0, TERMINATION_TAG, MPI_COMM_WORLD,
              &recv_request_termination);
              */

    //MPI_Test(&recv_request_termination,&flag, &status);
    while (1) {

      MPI_Recv((char *) &fc, sizeof(struct FILE_CHUNK), MPI_BYTE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      if (status.MPI_TAG == TERMINATION_TAG)
        break;
      //printf("FBUFFER_WORKER\n");
      //printf("Recebi isto :|%s|\n",fc.buffer);

      process_data_chunk((struct FILE_CHUNK *) &fc);
      //fc.n_words = 10;


      MPI_Send((char *) &fc, sizeof(struct FILE_CHUNK), MPI_BYTE, 0, 0, MPI_COMM_WORLD);

    }

  }
  //printf("I am killing myself %d\n", rank);
  MPI_Finalize();
  return EXIT_SUCCESS;
}

int get_available_worker(bool *worker_status, int size) {
  for (int i = 1; i < size; i++) {
    if (worker_status[i])
      return i;
  }
  return -1;
}

void process_data_chunk(struct FILE_CHUNK *fc) {
  int inWord = 0, bytes_to_read = 0;        // flag to say if you are inside a word or not
  char char_utf8[5];
  int has_vowel[VOWELS];

  for (int j = 0; j < VOWELS; j++) {
    has_vowel[j] = 0;
  }


  for (int i = 0; i < fc->buffer_size; i++) {
    if (fc->buffer[i] == 0) {
      break;
    }

    bytes_to_read = 0;
    if ((fc->buffer[i] & 0x000000A0) == 0x0) {
      // printf("VALUES %X - 0 \n", fc->buffer[i]& 0x000000A0);
      bytes_to_read = 0;
    }
    if ((fc->buffer[i] & 0x000000F0) == 0xC0) {
      // printf("\tVALUES %X - 1\n", fc->buffer[i]& 0x000000F0);
      bytes_to_read = 1;
    }
    if ((fc->buffer[i] & 0x000000F0) == 0xE0) {
      // printf("\tVALUES %X - 2\n", fc->buffer[i]& 0x000000F0);
      bytes_to_read = 2;
    }
    if ((fc->buffer[i] & 0x000000FA) == 0XF0) {
      // printf("\tVALUES %X - 3\n", buffer[i]& 0x000000FA);
      bytes_to_read = 3;
    }

    char_utf8[bytes_to_read + 1] = '\0';

    for (int l = 0; l <= bytes_to_read; l++) {
      char_utf8[l] = fc->buffer[i + l];
    }

    // printf("Char8:%s | Len:%li | Bytes:%i\n", char_utf8, strlen(char_utf8), bytes_to_read);

    if ((((unsigned char) char_utf8[0] >= 0x41 && (unsigned char) char_utf8[0] <= 0x5a)
         || ((unsigned char) char_utf8[0] >= 0x61 && (unsigned char) char_utf8[0] <= 0x7a)
         || ((unsigned char) char_utf8[0] >= 0x30 && (unsigned char) char_utf8[0] <= 0x39)
         || ((unsigned char) char_utf8[0] == 0xC3))) {
      update_status(&inWord, &(fc->n_words));
    } else if ((unsigned char) char_utf8[0] == 0x27 && inWord == 1) {
      inWord = 1;
    } else if (strlen(char_utf8) == 3) {
      if (inWord == 1 && (unsigned char) char_utf8[0] == 0xE2 && (unsigned char) char_utf8[1] == 0x80 && (
          (unsigned char) char_utf8[2] == 0x98 || (unsigned char) char_utf8[2] == 0x99)) {
        inWord = 1;
      } else {
        reset_flags(&inWord, &has_vowel[0]);
      }
    } else {
      reset_flags(&inWord, &has_vowel[0]);
    }

    if (inWord == 1) {
      if (strlen(char_utf8) == 2 && (unsigned char) char_utf8[0] == 0xC3) {
        if (((unsigned char) char_utf8[1] >= 0xA0 && (unsigned char) char_utf8[1] <= 0xA3) ||
            ((unsigned char) char_utf8[1] >= 0x80 && (unsigned char) char_utf8[1] <= 0x83)) {
          update_status(&has_vowel[0], &fc->n_words_vowels[0]);
        }
        if (((unsigned char) char_utf8[1] >= 0xA8 && (unsigned char) char_utf8[1] <= 0xAA) ||
            ((unsigned char) char_utf8[1] >= 0x88 && (unsigned char) char_utf8[1] <= 0x8A)) {
          update_status(&has_vowel[1], &fc->n_words_vowels[1]);
        }
        if (((unsigned char) char_utf8[1] >= 0xAC && (unsigned char) char_utf8[1] <= 0xAD) ||
            ((unsigned char) char_utf8[1] >= 0x8C && (unsigned char) char_utf8[1] <= 0x8D)) {
          update_status(&has_vowel[2], &fc->n_words_vowels[2]);
        }
        if (((unsigned char) char_utf8[1] >= 0xB2 && (unsigned char) char_utf8[1] <= 0xB5) ||
            ((unsigned char) char_utf8[1] >= 0x92 && (unsigned char) char_utf8[1] <= 0x95)) {
          update_status(&has_vowel[3], &fc->n_words_vowels[3]);
        }
        if (((unsigned char) char_utf8[1] >= 0xB9 && (unsigned char) char_utf8[1] <= 0xBA) ||
            ((unsigned char) char_utf8[1] >= 0x99 && (unsigned char) char_utf8[1] <= 0x9A)) {
          update_status(&has_vowel[4], &fc->n_words_vowels[4]);
        }
      } else {
        if ((unsigned char) char_utf8[0] == 0x41 || (unsigned char) char_utf8[0] == 0x61) {
          update_status(&has_vowel[0], &fc->n_words_vowels[0]);
        } else if ((unsigned char) char_utf8[0] == 0x45 || (unsigned char) char_utf8[0] == 0x65) {
          update_status(&has_vowel[1], &fc->n_words_vowels[1]);
        } else if ((unsigned char) char_utf8[0] == 0x49 || (unsigned char) char_utf8[0] == 0x69) {
          update_status(&has_vowel[2], &fc->n_words_vowels[2]);
        } else if ((unsigned char) char_utf8[0] == 0x4F || (unsigned char) char_utf8[0] == 0x6F) {
          update_status(&has_vowel[3], &fc->n_words_vowels[3]);
        } else if ((unsigned char) char_utf8[0] == 0x55 || (unsigned char) char_utf8[0] == 0x75) {
          update_status(&has_vowel[4], &fc->n_words_vowels[4]);
        } else if ((unsigned char) char_utf8[0] == 0x59 || (unsigned char) char_utf8[0] == 0x79) {
          update_status(&has_vowel[5], &fc->n_words_vowels[5]);
        }
      }

    }
    memset(char_utf8, 0, sizeof(char_utf8));
    // char_utf8[bytes_to_read+1] = '\0';
    i = i + bytes_to_read;
    // res = extract_char(buffer, char_utf8);
  }
}

void reset_flags(int *inWord, int *has_vowel) {
  *inWord = 0;
  for (int j = 0; j < VOWELS; j++) {
    has_vowel[j] = 0;
  }
  return;
}

void update_status(int *flag, int *stats) {
  if (*flag == 0) {
    *flag = 1;
    *stats = *stats + 1;
  }
  return;
}

