#include "sharedRegion.h"
#include <pthread.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_KBYTES 8
#define MIN_KBYTES 4

static char ** file_names;

static int n_files;

static struct FILES_STATISTICS * file_statistics;

/** \brief flag which warrants that the data transfer region is initialized exactly once */
static pthread_once_t init = PTHREAD_ONCE_INIT;

/** \brief workers synchronization point when previously processed partial info was stored in region */
static pthread_cond_t store;

/** \brief workers synchronization point when next chunk was obtained and processed */
static pthread_cond_t process;

/** \brief flag signaling the previously processed partial info was stored */
static bool stored;

/** \brief flag signaling the next chunk was obtained/processed */
static bool processed;

/** \brief position of the file */
static int pos;

/** \brief flag that tells if it is needed to open the file or not */
static bool isOpen;

static FILE * f;

static int file_id ;


/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;


/**
 *  \brief Initialization of the data transfer region.
 *  Internal monitor operation.
 */

static void initialization (void) {
    isOpen = false;
    pos = 0;
    file_id = 0;
    pthread_cond_init (&process, NULL);                                 /* initialize workers synchronization point */
    pthread_cond_init (&store, NULL);          
    setlocale(LC_CTYPE, "");    
}


void store_file_names (int n, char **filenames){
      
    if ((pthread_mutex_lock (&accessCR)) != 0) {                             /* enter monitor */                       
       perror ("error on entering monitor(CF)");                            /* save error in errno */
       int status = EXIT_FAILURE;
       pthread_exit(&status);
    }
    n_files = n;

    printf("Files : %d\n", n_files);
    file_names = malloc(n_files);
    file_statistics = malloc(sizeof(struct FILES_STATISTICS) * n_files);
    
    for (int i = 0; i < n_files; i++){

        file_names[i] = malloc(strlen(filenames[i]));
        strcpy( file_names[i], filenames[i]);

        #define fs file_statistics[i]

        fs.file_name = malloc(strlen(filenames[i]));
        strcpy( fs.file_name, filenames[i]);


        fs.n_words = 0;
        memset(&fs.n_words_vowels, 0, VOWELS);

        #undef fs

    }


    
    
    pthread_once (&init, initialization);    

    if ((pthread_mutex_unlock (&accessCR)) != 0) {                   /* exit monitor */                                                
       perror ("error on exiting monitor(CF)");                     /* save error in errno */
       int status = EXIT_FAILURE;
       pthread_exit(&status);
    }

}


int extract_char(FILE * f, char *  utf_8chr){
    //printf("extract char");
    char c;
    int res, bytes_to_read = 0;
    res = fread(&c, sizeof(unsigned char), 1, f);
    if(res == 0)
        return  EOF;

    if ((c & 0x000000A0) == 0b0000)
    {
//printf("VALUES %X - 0 \n", c& 0x000000A0);
        bytes_to_read = 0;
    }
    if ((c & 0x000000F0) == 0xC0)
    {
//printf("\tVALUES %X - 1\n", c& 0x000000F0);
        bytes_to_read = 1;
    }
    if ((c & 0x000000F0) == 0xE0)
    {
//printf("\tVALUES %X - 2\n", c& 0x000000F0);
        bytes_to_read = 2;
    }
    if ((c & 0x000000FA) == 0XF0)
    {
//printf("\tVALUES %X - 3\n", c& 0x000000FA);
        bytes_to_read = 3;
    }

    utf_8chr[0] = c;
    //utf_8chr[bytes_to_read + 1] = '\0';

    for (int i = 0; i < bytes_to_read; i++) {
        res = fread(&(utf_8chr[i+1]), sizeof(char), 1, f);
        if((utf_8chr[i+1] & 0x000000C0) != 0b10000000){
            fprintf(stderr, "UTF-8 file is corrupted %ld.",ftell(f));
            exit(EXIT_FAILURE);
        }
    }

    //printf("String: |%s|\n", utf_8chr);
    //printf("\n");

    return feof(f) ? EOF : bytes_to_read + 1;
}

bool is_word_separator(char * c){
    int len = strlen(c);
    
    if (len == 0) return true;

    unsigned char c0 = c[0];
    if (c0 == 0x20 || c0 == 0x9 || c0 == 0xA || c0 == 0xD || c0 == 0x2d || c0 == 0x22 || c0 == 0x2e || c0 == 0x2c ||
        c0 == 0x3b || c0 == 0x3a || c0 == 0x3a || c0 == 0x3f || c0 == 0x21 || c0 == 0x5b || c0 == 0x5d || c0 == 0x28 ||
        c0 == 0x29) {
        return true;
    }

    if (len == 3){
        unsigned char c1 = c[1];
        unsigned char c2 = c[2];
        if (c0 == 0xe2 && c1 == 0x80 && ( c2 == 0x9c || c2 == 0x9D) ) {
            return true;
        }
        if (c0 == 0xe2 && c1 == 0x80 && (c2 == 0x93 || c2 == 0x96) ) {
            return true;
        }
        if (c0 == 0xe2 && c1 == 0x80 && c2 == 0xA6 ) {
            return true;
        }
        if (c0 == 0xe2 && c1 == 0x80 && c2 == 0x9c ) {
            return true;
        }
    }

    return false;
}

int get_data_chunk(struct FILE_CHUNK * file_chunk){
    // acquire lock
    //printf("get data chunkj\n");

    if ((pthread_mutex_lock (&accessCR)) != 0) {                             /* enter monitor */                       
       perror ("error on entering monitor(CF)");                            /* save error in errno */
       int status = EXIT_FAILURE;
       pthread_exit(&status);
    }

    printf("TID: %ld\n\n", pthread_self());
    // read N Bytes until you reach Min Bytes and then try to finish the word
    //
    printf("File id : %d,\tisOpen: %d,\t", file_id, isOpen);
    //printf("feof %d,\t ferror %d\n", feof(f), ferror(f));
    if (file_id > n_files - 1) {
        printf("Processing Complete\n");

        // release lock
        if ((pthread_mutex_unlock (&accessCR)) != 0) {                   /* exit monitor */                                                
           perror ("error on exiting monitor(CF)");                     /* save error in errno */
           int status = EXIT_FAILURE;
           pthread_exit(&status);
        }
        return 1;
    }

    if (!isOpen){
        // open File and proceed
        f = fopen(file_names[file_id], "rb");
        isOpen = true;
        pos=0;
    }
    
    file_chunk->file_id = file_id;
    file_chunk->n_words = 0;
    for (int i = 0; i < VOWELS; i++)
        file_chunk->n_words_vowels[i] = 0;

    int len = 1024 * MIN_KBYTES;
    file_chunk->buffer = malloc(sizeof(char) * len);
    
    #define f_buffer file_chunk->buffer

    memset(f_buffer, 0, len); // fill buffer with spaces

    int res;

    for (int i = 0; i< len; i+=res) {
        res = extract_char(f, &(f_buffer[i]));
        if (feof(f)) {
            break;
        }

        if (ferror(f)){
            fprintf(stderr, "A problem with the file %s was detected!\n", file_names[file_id]);

            // file is over
            isOpen = false;
            file_id ++ ;

            // release lock
            if ((pthread_mutex_unlock (&accessCR)) != 0) {                   /* exit monitor */                                                
               perror ("error on exiting monitor(CF)");                     /* save error in errno */
               int status = EXIT_FAILURE;
               pthread_exit(&status);
            }
            return 0;
        }
    }

    if (!feof(f)) // nonzero means eof was reached
    {
        int i = len - 1;
        while (i >= 0 && is_word_separator(&(f_buffer[i]))) {
            i--;
        }

        int end_pos = i;

        fseek(f, len - end_pos, SEEK_CUR); // go back n positions
    }
    else{
        // if eof was set then the end position is the end position and the word is complete
        // file is completely read
        isOpen = false;
        file_id ++ ;
    }
    

    #undef f_buffer

    // release lock
    if ((pthread_mutex_unlock (&accessCR)) != 0) {                   /* exit monitor */                                                
       perror ("error on exiting monitor(CF)");                     /* save error in errno */
       int status = EXIT_FAILURE;
       pthread_exit(&status);
    }
    printf("get data chunkj end\n");

    return 0;
}

void save_partial_results(struct FILE_CHUNK *file_chunk){
    // acquire lock
    if ((pthread_mutex_lock (&accessCR)) != 0) {                             /* enter monitor */                       
       perror ("error on entering monitor(CF)");                            /* save error in errno */
       int status = EXIT_FAILURE;
       pthread_exit(&status);
    }
    // get the file which the thread was processing and update the file_statistics
    #define fs file_statistics[file_chunk->file_id]

    printf("file_statistics: %s\n", fs.file_name);
    printf("file_statistics: %d\n", fs.n_words);
    printf("file_statistics: %d\n", *fs.n_words_vowels);


    printf("file_chunk: %d\n", file_chunk->file_id);
    printf("file_chunk: %d\n", file_chunk->n_words_vowels[0]);
    printf("file_chunk: %d\n", file_chunk->n_words);
    fs.n_words += (file_chunk->n_words * 1);
    for (int i = 0; i < VOWELS; i++) {
        fs.n_words_vowels[i] += file_chunk->n_words_vowels[i];
    }

    #undef fs


    // release lock
    if ((pthread_mutex_unlock (&accessCR)) != 0) {                   /* exit monitor */                                                
       perror ("error on exiting monitor(CF)");                     /* save error in errno */
       int status = EXIT_FAILURE;
       pthread_exit(&status);
    }
}

void print_processing_results(){

   // acquire lock
    if ((pthread_mutex_lock (&accessCR)) != 0) {                             /* enter monitor */                       
       perror ("error on entering monitor(CF)");                            /* save error in errno */
       int status = EXIT_FAILURE;
       pthread_exit(&status);
    }
    
    for (int i = 0; i < n_files; i++) {
        fprintf(stdout, "File %s:\n", file_names[i]);
        fprintf(stdout, "NWords %d\n", file_statistics[i].n_words);
    }

    // release lock
    if ((pthread_mutex_unlock (&accessCR)) != 0) {                   /* exit monitor */                                                
       perror ("error on exiting monitor(CF)");                     /* save error in errno */
       int status = EXIT_FAILURE;
       pthread_exit(&status);
    }
}


