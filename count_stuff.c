#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

void help_msg();
int process_file(const char *fname, const char vowel);

int main(int argc, char *argv[])
{
    int opt;
    unsigned char *char_arr;
    unsigned char vowel;
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

    for (int i = optind; i < argc; i++)
    {
        // printf("%s\n", argv[i]);

        int total_words = 0;
        total_words = process_file(argv[i], vowel);
    }
    exit(EXIT_SUCCESS);
}

void help_msg()
{
    printf("Help\n");
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

void update_status(int* flag, int* stats){
    if (*flag == 0){
        *flag = 1;
        *stats = *stats + 1;
    }
    return;
}

int process_file(const char *fname, const char vowel)
{
    // printf("Counting vowel %c\n", vowel);

    FILE *f = fopen(fname, "rb");
    unsigned char c;
    unsigned char *char_utf8;
    int inWord = 0, // flag to say if you are insde a word or not
        res, // determines if the fread function had success or not
        bytes_to_read = 0, // bytes to read

        total_words = 0, // total number of words
        has_A = 0, // flag to say if word has an A
        has_E = 0, // flag to say if word has an E
        has_I = 0, // flag to say if word has an I
        has_O = 0, // flag to say if word has an O
        has_U = 0, // flag to say if word has an U
        has_Y = 0, // flag to say if word has an Y
        total_words_A = 0, // total number of words with the A
        total_words_E = 0, // total number of words with the E
        total_words_I = 0, // total number of words with the I
        total_words_O = 0, // total number of words with the O
        total_words_U = 0, // total number of words with the U
        total_words_Y = 0; // total number of words with the Y

    res = fread(&c, sizeof(unsigned char), 1, f);
    while (res == 1)
    {
        //printf("\n---------------------------------\n");
        //printf("First CHAR: 0x%X \n", c);
        //printf("First CHAR: 0x%X, 0x%X \n", c, (c & 0xA));
        //printf("First CHAR: 0x%X, 0x%X \n", c, (c & 0xF0));
        //printf("First CHAR: 0x%X, 0x%X \n", c, (c & 0xF0));
        //printf("First CHAR: 0x%X, 0x%X \n", c, (c & 0xFA));
        if ((c & 0xA0) == 0b0000)
        {
            // printf("VALUES %X - 0 \n", c);
            bytes_to_read = 0;
        }
        if ((c & 0xF0) == 0xC0)
        {
            // printf("\tVALUES %X - 1\n", c);
            bytes_to_read = 1;
        }
        if ((c & 0xF0) == 0xE0)
        {
            // printf("\tVALUES %X - 2\n", c);
            bytes_to_read = 2;
        }
        if ((c & 0xFA) == 0XF0)
        {
            // printf("\tVALUES %X - 3\n", c);
            bytes_to_read = 3;
        }

        char_utf8 = (unsigned char *)malloc((bytes_to_read + 2) * sizeof(unsigned char));
        //printf("Size %ul, calc %d \n", strlen(char_utf8), bytes_to_read + 2);
        char_utf8[0] = c;
        char_utf8[bytes_to_read + 1] = '\0';

        if (bytes_to_read != 0)
            res = fread(&(char_utf8[1]), sizeof(unsigned char), bytes_to_read, f);

        // if (strlen(char_utf8) == 3){
        //     if (char_utf8[0] == 0xE2 && char_utf8[1] == 0x80 && (
        //         char_utf8[2] == 0x9C || char_utf8[2] == 0x9D ||
        //         char_utf8[2] == 0x93 || char_utf8[2] == 0xA6 )){
        //             inWord = 0;
        //     } else if (inWord == 1 && char_utf8[0] == 0xE2 && char_utf8[1] == 0x80 && (
        //         char_utf8[2] == 0x98 || char_utf8[2] == 0x99)){
        //         inWord = 1;
        //     }
        // // } else if (strlen(char_utf8) == 2){
        // //     if (char_utf8[1] == 0xC2 && (char_utf8[2] == 0xAB || char_utf8[2] == 0xBB )) inWord = 0;
        // //     else inWord = 1;
        // } else if ( char_utf8[0] == 0x20 || char_utf8[0] == 0x09 || char_utf8[0] == 0x0A || 
        //     char_utf8[0] == 0x0D || char_utf8[0] == 0x2D || char_utf8[0] == 0x22 || 
        //     char_utf8[0] == 0x5B || char_utf8[0] == 0x5D || char_utf8[0] == 0x28 || 
        //     char_utf8[0] == 0x29 || char_utf8[0] == 0x2E || char_utf8[0] == 0x2C || 
        //     char_utf8[0] == 0x3A || char_utf8[0] == 0x3B || char_utf8[0] == 0x3F || 
        //     char_utf8[0] == 0x21 ){
        //     inWord = 0;
        // } else {
        //     if(inWord == 0){
        //         inWord = 1;
        //         total_words++;
        //         if (char_utf8[0] > 0x7a) printf("Word %s\n", char_utf8);
        //     }
        // }

        // Check if 1st bit is in range of the latin letters area
        // This is
        // Problem: × →→ c3 97 →→ MULTIPLICATION SIGN && ÷ →→ c3 b7 →→ DIVISION SIGN
        if (((char_utf8[0] >= 0x41 && char_utf8[0] <= 0x5a) 
            || (char_utf8[0] >= 0x61 && char_utf8[0] <= 0x7a)
            || (char_utf8[0] >= 0x30 && char_utf8[0] <= 0x39)
            || (char_utf8[0] == 0xC3))){
            update_status(&inWord, &total_words);
        } else if (char_utf8[0] == 0x27 && inWord == 1){
            inWord = 1;
        } else if (strlen(char_utf8) == 3){
            if (inWord == 1 && char_utf8[0] == 0xE2 && char_utf8[1] == 0x80 && (
                char_utf8[2] == 0x98 || char_utf8[2] == 0x99)){
                inWord = 1;
            } else {
                reset_flags(&inWord, &has_A, &has_E, &has_I, &has_O, &has_U, &has_Y);
            }
        } else {
            reset_flags(&inWord, &has_A, &has_E, &has_I, &has_O, &has_U, &has_Y);
        }

        if (inWord == 1){
            if (strlen(char_utf8) == 2 && char_utf8[0] == 0xC3){
                if ((char_utf8[1] >= 0xA0 && char_utf8[1] <= 0xA3) ||
                    (char_utf8[1] >= 0x80 && char_utf8[1] <= 0x83)){
                    update_status(&has_A, &total_words_A);
                } 
                if ((char_utf8[1] >= 0xA8 && char_utf8[1] <= 0xAA) ||
                    (char_utf8[1] >= 0x88 && char_utf8[1] <= 0x8A)){
                    update_status(&has_E, &total_words_E);
                }
                if ((char_utf8[1] >= 0xAC && char_utf8[1] <= 0xAD) ||
                    (char_utf8[1] >= 0x8C && char_utf8[1] <= 0x8D)){
                    update_status(&has_I, &total_words_I);
                }
                if ((char_utf8[1] >= 0xB2 && char_utf8[1] <= 0xB5) ||
                    (char_utf8[1] >= 0x92 && char_utf8[1] <= 0x95)){
                    update_status(&has_O, &total_words_O);
                }
                if ((char_utf8[1] >= 0xB9 && char_utf8[1] <= 0xBA) ||
                    (char_utf8[1] >= 0x99 && char_utf8[1] <= 0x9A)){
                    update_status(&has_U, &total_words_U);
                }
            } else {
                if (char_utf8[0] == 0x41 || char_utf8[0] == 0x61){
                    update_status(&has_A, &total_words_A);
                } else if (char_utf8[0] == 0x45 || char_utf8[0] == 0x65){
                    update_status(&has_E, &total_words_E);
                } else if (char_utf8[0] == 0x49 || char_utf8[0] == 0x69){
                    update_status(&has_I, &total_words_I); 
                } else if (char_utf8[0] == 0x4F || char_utf8[0] == 0x6F){
                    update_status(&has_O, &total_words_O); 
                } else if (char_utf8[0] == 0x55 || char_utf8[0] == 0x75){
                    update_status(&has_U, &total_words_U); 
                } else if (char_utf8[0] == 0x59 || char_utf8[0] == 0x79){
                    update_status(&has_Y, &total_words_Y);
                }
            }
        }
        
        // printf("String: |%s|\n", char_utf8);
        // printf("Valor bool %X , %X Controlo %d\n", char_utf8[0] , char_utf8[1], 1==1 );

        // printf("TODO: Determine if inside word or if we left if we left increment counter\n");
        // printf("TODO: DETERMIN IF THE current char matches the vowel we are counting\n");


        bytes_to_read = 0;
        res = fread(&c, sizeof(unsigned char), 1, f);
        free(char_utf8);

        // printf("Number of words: %i\n", total_words);

        // printf("Number of words with vowel %c: %i\n", v, total_words_vowel);
    }
    fclose(f);

    printf("\nFile name: %s\n", fname);
    printf("Total number of words = %i\n", total_words);
    printf("N. of words with an \n \t A \t E \t I \t O \t U \t Y \n");
    printf("\t %i \t %i \t %i \t %i \t %i \t %i \n", total_words_A, total_words_E, total_words_I, total_words_O, total_words_U, total_words_Y);

    return total_words;
}