#ifndef SHAREDREGION_H
#define SHAREDREGION_H

#include <locale.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>


/**
 *  Chunk that contains information to be processed
 *
 */
struct WORK_CHUNK {
  int *begin;// id of the FINAL_STRUCT to be updated
  int size;  // size of the arr
  int iteration;
  int dir_flag;// direction flag
};

/**
 *  \brief Initialization of the data transfer region.
 *  Internal monitor operation.
 */

void initialization(void);

/**
 * Store filenames and create FINAL_STRUCTS for each file
 * @param n number of files
 * @param files array of filenames to be stored
 */
void store_info(int n_workers, char *file);

void validate();

void read_file();

void distribute();

/**
 * get a work chunk from the shared region
 *
 * waits until work chunk has some element
 * @param wc work chunk allocated so that the worker thread has something to get
 * @return 0 if it will have more data to work on nonzero if there will be no more work
 */
bool get_work_chunk(struct WORK_CHUNK *wc);

void work_done();
#endif
