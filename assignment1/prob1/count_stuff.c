#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define MAX_VOWELS 6 // a e i o u y

void help_msg();

// return EOF if EOF is REACHED 0 otherwise
// This function can abort the program if the file is detected to be corrupt
int extract_char(FILE * f, char *  utf_8chr);

// Process text file in way that we compile number of words and number of words that contain a vowel
void process_file(const char *fname, size_t * words_count, size_t *words_with_vowel);

// Print statistics
void print_statistics(size_t * words_count, size_t *words_with_vowel);

int main(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "hp:c:")) != -1)
    {
        switch (opt)
        {
        case 'p':
//printf("Hex Values include: ");
            for (size_t i = 0; i < strlen(optarg); i++)
//printf("%X, ", optarg[i]);
//printf("\n");
            break;
        case 'c':
//printf("Hex Values include: ");
            for (size_t i = 0; i < strlen(optarg); i++)
//printf("%X, ", optarg[i]);
//printf("\n");
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
    size_t word_count[(argc - optind)];
    size_t word_vowel_count[(argc - optind + 1) * MAX_VOWELS];

    int idx_start = 0;
    for (int i = optind; i < argc; i++, idx_start=((i - optind) * MAX_VOWELS) )
    {
        process_file(argv[i], &word_count[(i-optind)], &word_vowel_count[idx_start]);
        printf("\nFile name: %s\n", argv[i]);
        printf("Total number of words = %zu\n", word_count[i-optind]);
        printf("N. of words with an \n\t%13s%13s%13s%13s%13s%13s \n\t", "A", "E", "I", "O", "U", "Y" );
        for (int j = idx_start; j < idx_start + MAX_VOWELS; j++) {
            printf("%13zu", word_vowel_count[j]);
        }
        printf("\n");
    }
    exit(EXIT_SUCCESS);
}

void help_msg()
{
//printf("Help\n");
}


void reset_flags(int* inWord, int* has_A, int* has_E, int* has_I, int* has_O, int* has_U, int* has_Y){
    *inWord = 0;
    *has_A = 0;
    *has_E = 0;
    *has_I = 0;
    *has_O = 0;
    *has_U = 0;
    *has_Y = 0;
    return;
}

void update_status(int* flag, size_t* stats){
    if (*flag == 0){
        *flag = 1;
        *stats = *stats + 1;
    }
    return;
}

void process_file(const char *fname, size_t *words_count, size_t *words_with_vowel)
{
    FILE *f = fopen(fname, "rb");
    unsigned char c;
    // 4 bytes + '\0'
    char char_utf8[5] ;

    size_t word_count = 0,
        word_count_vowel = 0,
        total_words = 0,
        total_words_A =0, // total number of words with the A
        total_words_E =0, // total number of words with the E
        total_words_I =0, // total number of words with the I
        total_words_O =0, // total number of words with the O
        total_words_U =0, // total number of words with the U
        total_words_Y =0; // total number of words with the Y

    int inWord = 0, res = 0, bytes_to_read = 0,        // flag to say if you are insde a word or not
        has_A = 0, // flag to say if word has an A
        has_E = 0, // flag to say if word has an E
        has_I = 0, // flag to say if word has an I
        has_O = 0, // flag to say if word has an O
        has_U = 0, // flag to say if word has an U
        has_Y = 0; // flag to say if word has an Y

    res = extract_char(f, char_utf8);

    while (res != EOF)
    {
        //printf("First CHAR: %ld  %s\n", strlen(char_utf8), char_utf8);

        if ((((unsigned char)char_utf8[0] >= 0x41 && (unsigned char)char_utf8[0] <= 0x5a) 
            || ((unsigned char)char_utf8[0] >= 0x61 && (unsigned char)char_utf8[0] <= 0x7a)
            || ((unsigned char)char_utf8[0] >= 0x30 && (unsigned char)char_utf8[0] <= 0x39)
            || ((unsigned char)char_utf8[0] == 0xC3))){
            update_status(&inWord, &total_words);
        } else if ((unsigned char)char_utf8[0] == 0x27 && inWord == 1){
            inWord = 1;
        } else if (strlen(char_utf8) == 3){
            if (inWord == 1 && (unsigned char)char_utf8[0] == 0xE2 && (unsigned char)char_utf8[1] == 0x80 && (
                (unsigned char)char_utf8[2] == 0x98 || (unsigned char)char_utf8[2] == 0x99)){
                inWord = 1;
            } else {
                reset_flags(&inWord, &has_A, &has_E, &has_I, &has_O, &has_U, &has_Y);
            }
        } else {
            reset_flags(&inWord, &has_A, &has_E, &has_I, &has_O, &has_U, &has_Y);
        }

        if (inWord == 1){
            if (strlen(char_utf8) == 2 && (unsigned char)char_utf8[0] == 0xC3){
                if (((unsigned char)char_utf8[1] >= 0xA0 && (unsigned char)char_utf8[1] <= 0xA3) ||
                    ((unsigned char)char_utf8[1] >= 0x80 && (unsigned char)char_utf8[1] <= 0x83)){
                    update_status(&has_A, &total_words_A);
                } 
                if (((unsigned char)char_utf8[1] >= 0xA8 && (unsigned char)char_utf8[1] <= 0xAA) ||
                    ((unsigned char)char_utf8[1] >= 0x88 && (unsigned char)char_utf8[1] <= 0x8A)){
                    update_status(&has_E, &total_words_E);
                }
                if (((unsigned char)char_utf8[1] >= 0xAC && (unsigned char)char_utf8[1] <= 0xAD) ||
                    ((unsigned char)char_utf8[1] >= 0x8C && (unsigned char)char_utf8[1] <= 0x8D)){
                    update_status(&has_I, &total_words_I);
                }
                if (((unsigned char)char_utf8[1] >= 0xB2 && (unsigned char)char_utf8[1] <= 0xB5) ||
                    ((unsigned char)char_utf8[1] >= 0x92 && (unsigned char)char_utf8[1] <= 0x95)){
                    update_status(&has_O, &total_words_O);
                }
                if (((unsigned char)char_utf8[1] >= 0xB9 && (unsigned char)char_utf8[1] <= 0xBA) ||
                    ((unsigned char)char_utf8[1] >= 0x99 && (unsigned char)char_utf8[1] <= 0x9A)){
                    update_status(&has_U, &total_words_U);
                }
            } else {
                if ((unsigned char)char_utf8[0] == 0x41 || (unsigned char)char_utf8[0] == 0x61){
                    update_status(&has_A, &total_words_A);
                } else if ((unsigned char)char_utf8[0] == 0x45 || (unsigned char)char_utf8[0] == 0x65){
                    update_status(&has_E, &total_words_E);
                } else if ((unsigned char)char_utf8[0] == 0x49 || (unsigned char)char_utf8[0] == 0x69){
                    update_status(&has_I, &total_words_I); 
                } else if ((unsigned char)char_utf8[0] == 0x4F || (unsigned char)char_utf8[0] == 0x6F){
                    update_status(&has_O, &total_words_O); 
                } else if ((unsigned char)char_utf8[0] == 0x55 || (unsigned char)char_utf8[0] == 0x75){
                    update_status(&has_U, &total_words_U); 
                } else if ((unsigned char)char_utf8[0] == 0x59 || (unsigned char)char_utf8[0] == 0x79){
                    update_status(&has_Y, &total_words_Y);
                }
            }

        }
        res = extract_char(f, char_utf8);
    }

    words_count[0] = total_words;

    words_with_vowel[0] = total_words_A;
    words_with_vowel[1] = total_words_E;
    words_with_vowel[2] = total_words_I;
    words_with_vowel[3] = total_words_O;
    words_with_vowel[4] = total_words_U;
    words_with_vowel[5] = total_words_Y;

    fclose(f);
    return;
}




int extract_char(FILE * f, char *  utf_8chr){
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
    utf_8chr[bytes_to_read + 1] = '\0';

    for (int i = 0; i < bytes_to_read; i++) {
        res = fread(&(utf_8chr[i+1]), sizeof(char), 1, f);
        if((utf_8chr[i+1] & 0x000000C0) != 0b10000000){
            fprintf(stderr, "UTF-8 file is corrupted %ld.",ftell(f));
            exit(EXIT_FAILURE);
        }
        if(res == 0) return EOF;
    }

    // printf("Char8:%s | Len:%li | Bytes:%i\n", utf_8chr, strlen(utf_8chr), bytes_to_read);

//printf("String: |%s|\n", utf_8chr);
//printf("Valor bool 0x%X ", utf_8chr[i]);
//printf("\n");

    return res == 0 ? EOF : 0;
}
