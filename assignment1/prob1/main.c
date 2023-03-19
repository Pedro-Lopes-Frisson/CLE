#include <getopt.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "sharedRegion.h"
#include <pthread.h>

void help(char *prog_name);

static void *worker(void *args);
void *process_data_chunk(struct FILE_CHUNK *fc);


int main(int argc, char **argv) {

    int opt, n_workers;
    while ((opt = getopt(argc, argv, "ht:")) != -1) {
        switch (opt) {
            case 't':
                n_workers = (int) atoi(optarg);
                if (n_workers <= 0 || n_workers >= 20) {
                    fprintf(stderr, "%s: non positive number less than 20\n", argv[0] );
                    return EXIT_FAILURE;
                }
                break;
                break;
            case 'h':
                break;
            default: /* '?' */
                //help(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    printf("N WORKERS:a %d\n", n_workers);
    // only thing that we have left is the arguments with no flags let's assume they are all filenames
    int n_files = argc - optind;



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

    for (t = 0; t < n_workers; t++) {
        if (pthread_join(tIdWork[t], (void *) &status_p) !=
            0)                                  /* wait for thread worker to terminate */
        {
            perror("error on waiting for thread producer");
            exit(EXIT_FAILURE);
        }
        printf("thread worker, with id %u, has terminated. \n", t);
    }

    print_processing_results();


    exit(EXIT_SUCCESS);
}

void *process_data_chunk(struct FILE_CHUNK *fc){

   fc->n_words = 10;
    for (int i = 0; i < VOWELS; ++i) {
        (fc->n_words_vowels)[i] = 10 * (i + 1);
    }

}

static void *worker(void *args) {

    unsigned int id = *((unsigned int *) args);              /* worker id */

    struct FILE_CHUNK *file_chunk = (struct  FILE_CHUNK * ) malloc(sizeof (struct FILE_CHUNK)) ;     /* struct to store partial info of current file being processed */

    while (get_data_chunk(&file_chunk) == 0) {        /* while data available */
        //process_data_chunk(&file_chunk);        /* process current data*/
        printf("Chunk: %s\n", file_chunk->buffer);
        //save_partial_results(&file_chunk);         /* save in shared region */
    }
    int status = EXIT_SUCCESS;
    pthread_exit(&status);


}
