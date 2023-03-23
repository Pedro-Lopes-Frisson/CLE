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

/** \brief save the filenames to be read */
static char *file_name;

//8
static int n_workers;

/**
 * how many workers are still working
 */
static int n_workers_request_to_wait_for;
static int n_workers_needed;
static int free_workers;
/**
 * how many workers are workin
 */
static int n_workers_work_is_done;

static int n_workers_ready_to_work;

/** \brief array that holds every number to be sorted in place */
static int *values;

/**
 * iteration num
 */
static int it;

/** \brief condition used by the distributor to ensure that every worker has concluded his sorting iteration */
static pthread_cond_t wait_work_done;

/** \brief workers condition to wait for work assignment */
static pthread_cond_t wait_for_work;

/** \brief distributor uses this condition to wait for the workers to be ready for processing */
static pthread_cond_t workers_ready;

/** \brief flag which warrants that the data transfer region is initialized exactly once */
static pthread_once_t init = PTHREAD_ONCE_INIT;

/** \brief flag that tells if it is needed to open the file or not */
static bool isOpen;

static bool isOver;

static FILE *f;

static int file_id;

static int number_of_values;

static struct WORK_CHUNK * wc_chunks;
static int wc_chunks_count;
static int flip;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;

static int iterations_required;
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
