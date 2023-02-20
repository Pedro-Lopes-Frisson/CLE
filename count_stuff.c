#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "constants_symbols.h"

#define MAX_VOWELS 6 // a e i o u y

void help_msg();

// return EOF if EOF is REACHED 0 otherwise
// This function can abort the program if the file is detected to be corrupt
int extract_char(FILE * f, unsigned char *  utf_8chr);

// Process text file in way that we compile number of words and number of words that contain a vowel
void process_file(const char *fname, size_t * words_count, size_t *words_with_vowel);

// Print statistics
void print_statistics(size_t * words_count, size_t *words_with_vowel);

int main(int argc, char *argv[])
{
    int opt;
    unsigned char *char_arr;
    unsigned char *vowel;
    while ((opt = getopt(argc, argv, "hp:c:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            printf("Hex Values include: ");
            for (size_t i = 0; i < strlen(optarg); i++)
                printf("%X, ", optarg[i]);
            printf("\n");
            break;
        case 'c':
            printf("Hex Values include: ");
            for (size_t i = 0; i < strlen(optarg); i++)
                printf("%X, ", optarg[i]);
            printf("\n");
            break;
        case 'h':
            help_msg();
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    size_t word_count[1];
    size_t word_vowel_count[1];

    for (int i = optind; i < argc; i++)
    {
        printf("%s\n", argv[i]);
        process_file(argv[i], &word_count[0], &word_vowel_count[0]);
    }
    exit(EXIT_SUCCESS);
}

void help_msg()
{
    printf("Help\n");
}

void process_file(const char *fname, size_t *words_count, size_t *words_with_vowel)
{
    FILE *f = fopen(fname, "rb");
    unsigned char c;
    // 4 bytes + '\0'
    unsigned char char_utf8[5] ;
    int inWord = 0, res = 0, bytes_to_read;        // flag to say if you are insde a word or not
    size_t word_count = 0, word_count_vowel = 0;

    res = extract_char(f, char_utf8);
    while (res != EOF)
    {
        printf("\n---------------------------------\n");

        //printf("First CHAR: 0x%X \n", c);
        //printf("First CHAR: 0x%X, 0x%X \n", c, (c & 0xA));
        //printf("First CHAR: 0x%X, 0x%X \n", c, (c & 0xF0));
        //printf("First CHAR: 0x%X, 0x%X \n", c, (c & 0xF0));
        //printf("First CHAR: 0x%X, 0x%X \n", c, (c & 0xFA));
        //
        


        printf("TODO: DETERMIN IF THE current char matches the vowel we are counting\n");
        // use wctomb to get the multiby representation of the byte that way we can match à and a
        if (! ( (char_utf8[0] >= 0x41 && char_utf8[0] <= 0x5A) || (char_utf8[0] >= 0x61 && char_utf8[0] <= 0x7A) || (char_utf8[0] >= 0x30 && char_utf8[0] <= 0x39) || strcmp((char *)char_utf8, "-") == 0 || strcmp((char *)char_utf8, "'")) && inWord == 1){
            word_count++;
            inWord = 0;
        }

        if (( (char_utf8[0] >= 0x41 && char_utf8[0] <= 0x5A) || (char_utf8[0] >= 0x61 && char_utf8[0] <= 0x7A) || (char_utf8[0] >= 0x30 && char_utf8[0] <= 0x39) )  && inWord == 0){
            inWord = 1;
        }
        

        bytes_to_read = 0;
        res = extract_char(f, char_utf8);

        printf("String: |%s|\n", char_utf8);
        for (int i = 0; i < bytes_to_read + 1; i++)
            printf("Valor bool 0x%X ", char_utf8[i]);
        printf("\n");
    }
    printf("The file %s has:\n%zu words and %zu have the vowel .\n", fname, word_count, word_count_vowel );
    free(char_utf8);
    fclose(f);
}




int extract_char(FILE * f, unsigned char *  utf_8chr){
    char c;
    int res, bytes_to_read = 0;
    res = fread(&c, sizeof(unsigned char), 1, f);

    if ((c & 0x000000A0) == 0b0000)
    {
        printf("VALUES %X - 0 \n", c& 0x000000A0);
        bytes_to_read = 0;
    }
    if ((c & 0x000000F0) == 0xC0)
    {
        printf("\tVALUES %X - 1\n", c& 0x000000F0);
        bytes_to_read = 1;
    }
    if ((c & 0x000000F0) == 0xE0)
    {
        printf("\tVALUES %X - 2\n", c& 0x000000F0);
        bytes_to_read = 2;
    }
    if ((c & 0x000000FA) == 0XF0)
    {
        printf("\tVALUES %X - 3\n", c& 0x000000FA);
        bytes_to_read = 3;
    }

    utf_8chr[0] = c;
    utf_8chr[bytes_to_read + 1] = '\0';

    for (int i = 0; i < bytes_to_read; i++) {
        res = fread(&(utf_8chr[i+1]), sizeof(char), 1, f);
        if((utf_8chr[i+1] & 0x000000C0) != 0b10000000){
            fprintf(stderr, "UTF-8 file is corrupted %ld.",ftell(f));
            exit(EXIT_FAILURE);
        }
        if(res == 0) return EOF;
    }

        printf("String: |%s|\n", utf_8chr);
        for (int i = 0; i < bytes_to_read + 1; i++)
            printf("Valor bool 0x%X ", utf_8chr[i]);
        printf("\n");

    return res == 0 ? EOF : 0;
}
