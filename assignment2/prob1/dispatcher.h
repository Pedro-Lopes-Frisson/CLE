
#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "structures.h"

#define MAX_KBYTES 8
#define MIN_KBYTES 2

static char **file_names;

static int n_files;

static struct FILES_STATISTICS *file_statistics;

/** \brief position of the file */
static int pos;

/** \brief flag that tells if it is needed to open the file or not */
static bool isOpen;

static FILE *f;

static int file_id;

static bool is_word_separator(char *c, int n);

/**
 *  \brief Initialization of the data transfer region.
 *  Internal monitor operation.
 */

static void initialization(void) {
  isOpen = false;
  pos = 0;
  file_id = 0;
  setlocale(LC_CTYPE, "");
}


void store_file_names(int n, char **filenames) {

  n_files = n;

  file_names = (char **) malloc(n_files * sizeof(char *));
  if (file_names == NULL) {
    fprintf(stderr, "Memory allocation for file_names failed\n");
  }

  for (size_t i = 0; i < n_files; i++) {
    file_names[i] = malloc(strlen(filenames[i]) * sizeof(char) + 1);
    if (file_names[i] == NULL) {
      fprintf(stderr, "Memory allocation for file_names failed\n");
    }
  }

  file_statistics = malloc(sizeof(struct FILES_STATISTICS) * n_files);

  if (file_statistics == NULL) {
    fprintf(stderr, "Memory allocation for file_statistics failed\n");
  }

  for (int i = 0; i < n_files; i++) {

    // copy fname to inner structure
    strcpy(file_names[i], filenames[i]);

    // initialize file statistics conserving the filename
    strcpy(file_statistics[i].file_name, file_names[i]);

    file_statistics[i].n_words = 0;

    for (size_t j = 0; j < VOWELS; j++) {
      file_statistics[i].n_words_vowels[j] = 0;
    }

  }
}


static int extract_char(FILE *f, char *utf_8chr) {
  //printf("extract char");
  char c;
  int res, bytes_to_read = 0;
  res = fread(&c, sizeof(unsigned char), 1, f);
  if (res == 0)
    return EOF;

  if ((c & 0x000000A0) == 0b0000) {
//printf("VALUES %X - 0 \n", c& 0x000000A0);
    bytes_to_read = 0;
  }
  if ((c & 0x000000F0) == 0xC0) {
//printf("\tVALUES %X - 1\n", c& 0x000000F0);
    bytes_to_read = 1;
  }
  if ((c & 0x000000F0) == 0xE0) {
//printf("\tVALUES %X - 2\n", c& 0x000000F0);
    bytes_to_read = 2;
  }
  if ((c & 0x000000FA) == 0XF0) {
//printf("\tVALUES %X - 3\n", c& 0x000000FA);
    bytes_to_read = 3;
  }

  utf_8chr[0] = c;
  //utf_8chr[bytes_to_read + 1] = '\0';

  for (int i = 0; i < bytes_to_read; i++) {
    res = fread(&(utf_8chr[i + 1]), sizeof(char), 1, f);
    if ((utf_8chr[i + 1] & 0x000000C0) != 0b10000000) {
      fprintf(stderr, "UTF-8 file is corrupted %ld.", ftell(f));
      exit(EXIT_FAILURE);
    }
  }

  //printf("\n");

  return feof(f) ? EOF : bytes_to_read + 1;
}

static bool is_word_separator(char *c, int n) {
  int len = n;
  char *c_str = (char *) malloc(sizeof(char) * (n + 1));
  strcpy(c_str, c);
  c_str[len] = '\0';
  if (len <= 0) return true;

  unsigned char c0 = c[0];
  if (c0 == 0x20 || c0 == 0x9 || c0 == 0xA || c0 == 0xD || c0 == 0x2d || c0 == 0x22 || c0 == 0x2e || c0 == 0x2c ||
      c0 == 0x3b || c0 == 0x3a || c0 == 0x3a || c0 == 0x3f || c0 == 0x21 || c0 == 0x5b || c0 == 0x5d || c0 == 0x28 ||
      c0 == 0x29) {

    return true;
  }

  if (len == 3) {
    unsigned char c1 = c[1];
    unsigned char c2 = c[2];
    if (c0 == 0xe2 && c1 == 0x80 && (c2 == 0x9c || c2 == 0x9D)) {
      return true;
    }
    if (c0 == 0xe2 && c1 == 0x80 && (c2 == 0x93 || c2 == 0x96)) {
      return true;
    }
    if (c0 == 0xe2 && c1 == 0x80 && c2 == 0xA6) {
      return true;
    }
    if (c0 == 0xe2 && c1 == 0x80 && c2 == 0x9c) {
      return true;
    }
  }

  return false;
}


/** function that takes in a allocated file_chunk structure and fills the char * buffer with characters in order to separate a file into chunks of data
 * */
int get_data_chunk(struct FILE_CHUNK *file_chunk, int threadId) {
  // acquire lock
  //printf("get data chunkj\n");

  // read N Bytes until you reach Min Bytes and then try to finish the word
  //
  //printf("File id : %d,\tisOpen: %d,\t\n", file_id, isOpen);
  //printf("feof %d,\t ferror %d\n", feof(f), ferror(f));

  if (file_id > n_files - 1) {

    // release lock
    return 1;
  }

  if (!isOpen) {
    // open File and proceed
    f = fopen(file_names[file_id], "rb");
    isOpen = true;
    pos = 0;
  }

  file_chunk->file_id = file_id;
  file_chunk->n_words = 0;
  for (int i = 0; i < VOWELS; i++)
    file_chunk->n_words_vowels[i] = 0;

  int len = MAX_CHAR;

#define f_buffer file_chunk->buffer

  memset(f_buffer, 0, len); // fill buffer with empty

  int res;
  int j = 0;
  int last_end_of_word = 0;
  for (j = 0; j < len - 1; j += res) {
    res = extract_char(f, &(f_buffer[j]));

    if (is_word_separator(&(f_buffer[j]), res) || res == EOF) {
      last_end_of_word = j;
    }

    if (feof(f) != 0) {
      break;
    }


    if (ferror(f) != 0) {
      fprintf(stderr, "A problem with the file %s was detected!\n", file_names[file_id]);

      // file is over
      isOpen = false;
      file_id++;

      // release lock
      return 0;
    }
  }
  f_buffer[last_end_of_word + 1] = '\0';

  if (!feof(f)) // nonzero means eof was reached
  {
    int end_pos = last_end_of_word;
    file_chunk->buffer_size = end_pos;
    fseek(f, -(len - end_pos) + 1, SEEK_CUR); // go back n positions
  } else {
    file_chunk->buffer_size = last_end_of_word;
    // if eof was set then the end position is the end position and the word is complete
    // file is completely read
    isOpen = false;
    file_id++;
  }


#undef f_buffer

  // release lock

  return 0;
}

void save_partial_results(struct FILE_CHUNK *file_chunk) {
  // acquire lock
  // get the file which the thread was processing and update the file_statistics
#define fs file_statistics[file_chunk->file_id]

  fs.n_words += (file_chunk->n_words * 1);
  for (int i = 0; i < VOWELS; i++) {
    fs.n_words_vowels[i] += file_chunk->n_words_vowels[i];
  }

#undef fs


  // release lock
}

void print_processing_results() {

  // acquire lock

  for (int i = 0; i < n_files; i++) {
    fprintf(stdout, "File name: %s\n", file_names[i]);
    fprintf(stdout, "Total Number of words = %d\n", file_statistics[i].n_words);
    fprintf(stdout, "N. of words with an\n");
    fprintf(stdout, "\t%10c", 'A');
    fprintf(stdout, "\t%10c", 'E');
    fprintf(stdout, "\t%10c", 'I');
    fprintf(stdout, "\t%10c", 'O');
    fprintf(stdout, "\t%10c", 'U');
    fprintf(stdout, "\t%10c\n", 'Y');
    for (int j = 0; j < VOWELS; ++j) {
      printf("\t%10d", file_statistics[i].n_words_vowels[j]);
    }
    printf("\n");
    printf("\n");
  }

  // release lock
  free(file_names);
  free(file_statistics);
}


#endif
