#ifndef STRUCTURES_H
#define STRUCTURES_H
#define VOWELS 6
#define MAX_CHAR 4096

#include <stdbool.h>

struct FILE_CHUNK {

    int file_id;

    char buffer[MAX_CHAR];
    int buffer_size;

    int n_words;
    int n_words_vowels[VOWELS];

    bool isEOF;
};

struct FILES_STATISTICS {
    char file_name[50];
    int n_words;
    int n_words_vowels[VOWELS];
};
#endif
