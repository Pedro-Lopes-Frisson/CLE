#ifndef SHAREDREGION_H
#define SHAREDREGION_H

#include <stdio.h>
#include <stdbool.h>

#define VOWELS 6

/**
   *
   */
struct FILE_CHUNK {

   int file_id;
   
   char * buffer;
   int buffer_size;

   int  n_words;
   int  n_words_vowels[VOWELS];

   bool isEOF;
};

struct FILES_STATISTICS {
   char file_name[50];
   int  n_words;
   int  n_words_vowels[VOWELS];
};

/**
 * This function shall perform a store operation inside the shared region.
 * @param n_files Explicit tell the function to save *n_files*
 * @param fileNames strings to store in the shared memory
 */
void store_file_names (int n_files, char *fileNames[]);

/**
 * Returns a data chunk, this operation is performed under mutual exclusion and the processing of the subsequent files is done in order.
 * @param file_chunk structure to store data related to the processing of the data and data to be processed
 * @return 1 in case of error or in case there are no more files to be processed, 0 otherwise
 */
int get_data_chunk(struct FILE_CHUNK * file_chunk, int threadId);

/**
 * Stores information related to a file identified in the FILE_CHUNK structure by fileId this operation is performed under mutual exclusion
 * @param partialInfo info related to the processing of a file_chunk
 */
void save_partial_results (struct FILE_CHUNK *partialInfo);

/**
 * Final action performed by the main thread in order to present the compiled statistics
 */
void print_processing_results();

#endif

