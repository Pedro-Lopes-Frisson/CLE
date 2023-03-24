#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <time.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "sharedRegion.h"
#include <pthread.h>

void help(char *program_name);

static void *worker(void *args);

void process_data_chunk(struct FILE_CHUNK *fc);

void update_status(int *flag, int *stats);

void reset_flags(int *inWord, int *has_vowel);


int main(int argc, char **argv) {

    int opt, n_workers;
    while ((opt = getopt(argc, argv, "ht:")) != -1) {
        switch (opt) {
            case 't':
                n_workers = (int) atoi(optarg);
                if (n_workers <= 0 || n_workers >= 20) {
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
    pthread_t tIdWork[n_workers];                            /* workers internal thread id array */
    unsigned int work[n_workers];                      /* workers application defined thread id array */
    int *status_p;                                  /* pointer to execution status */

    int t;

    for (int t = 0; t < n_workers; t++) work[t] = t;

    store_file_names(n_files, &(argv[optind]));

    printf("File names stored in the shared region.\n");

    for (t = 0; t < n_workers; t++) {
        if (pthread_create(&tIdWork[t], NULL, worker, &work[t]) !=
            0)                              /* create thread worker */
        {
            perror("error on creating thread producer");
            exit(EXIT_FAILURE);
        }
    }

    printf("Workers created and start processing.\n");

    struct timespec start_time, end_time;
    double elapsed_time;

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (t = 0; t < n_workers; t++) {
        if (pthread_join(tIdWork[t], (void *) &status_p) !=
            0)                                  /* wait for thread worker to terminate */
        {
            perror("error on waiting for thread producer");
            exit(EXIT_FAILURE);
        }
        printf("thread worker, with id %u, has terminated. \n", t);
    }
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1e9; // Convert seconds to nanoseconds
    elapsed_time += (end_time.tv_nsec - start_time.tv_nsec);

    printf("Elapsed time: %f nanoseconds\n", elapsed_time);
    print_processing_results();


    exit(EXIT_SUCCESS);
}

void help(char *program_name) {

    fprintf(stderr, "Usage: %s -t [ integer between 1 and 10 ] [file1 file2 file3 ...]\n", program_name);
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

static void *worker(void *args) {

    unsigned int id = *((unsigned int *) args);              /* worker id */

    struct FILE_CHUNK *file_chunk = (struct FILE_CHUNK *) malloc(
            sizeof(struct FILE_CHUNK));     /* struct to store partial info of current file being processed */

    while (get_data_chunk(file_chunk, id) == 0) {        /* while data available */
        process_data_chunk(file_chunk);        /* process current data*/
        save_partial_results(file_chunk);         /* save in shared region */
    }
    int status = EXIT_SUCCESS;
    free(file_chunk);
    pthread_exit(&status);


}
