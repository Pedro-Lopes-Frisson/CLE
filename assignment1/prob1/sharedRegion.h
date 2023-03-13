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

   int  * n_words;
   int  n_words_vowels[VOWELS];
   
   bool isEOF;
};

struct FILES_STATISTICS {
   char * file_name;
   int  n_words;
   int  n_words_vowels[VOWELS];
};

void store_file_names (int n_files, char *fileNames[]);

int get_data_chunk(struct FILE_CHUNK * file_chunk);

void save_partial_results (struct FILE_CHUNK *partialInfo);

void print_processing_results();

#endif









