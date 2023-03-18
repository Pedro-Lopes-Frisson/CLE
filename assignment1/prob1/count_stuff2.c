#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define MAX_VOWELS 6 // a e i o u y

void help_msg();

// return EOF if EOF is REACHED 0 otherwise
// This function can abort the program if the file is detected to be corrupt
int extract_char(char c, char *  utf_8chr);

// Process text file in way that we compile number of words and number of words that contain a vowel
void process_file(char * buffer, size_t * buffer_size, size_t * words_count, size_t *words_with_vowel);


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
        FILE *f = fopen(argv[i], "rd");
        char buffer[8000];
        fread(buffer, 8000, 1, f);
        size_t buffer_size = sizeof(buffer);
        process_file(buffer, &buffer_size,&word_count[(i-optind)], &word_vowel_count[idx_start]);
        fclose(f);
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


void reset_flags(int* inWord, int *has_vowel){
    *inWord = 0;
    for(int j = 0; j < MAX_VOWELS; j++){
        has_vowel[j] = 0;
    }
    return;
}

void update_status(int* flag, size_t* stats){
    if (*flag == 0){
        *flag = 1;
        *stats = *stats + 1;
    }
    return;
}

void process_file(char * buffer, size_t * buffer_size,size_t *words_count, size_t *words_with_vowel)
{
    // FILE *f = fopen(fname, "rb");
    // unsigned char c;
    // 4 bytes + '\0'
    char char_utf8[5] ;

    int has_vowel[MAX_VOWELS];
    for(int j = 0; j < MAX_VOWELS; j++){
        words_with_vowel[j] = 0;
        has_vowel[j] = 0;
    }

    size_t word_count = 0,
        total_words = 0;

    int inWord = 0, res = 0, bytes_to_read = 0;        // flag to say if you are insde a word or not

    // res = extract_char(buffer, char_utf8);

    for(int i = 0; i<buffer_size[0]; i++){
        
        if(buffer[i] == 0) {
            break;
        }

        bytes_to_read = 0;
        if ((buffer[i] & 0x000000A0) == 0b0000)
        {
        // printf("VALUES %X - 0 \n", buffer[i]& 0x000000A0);
            bytes_to_read = 0;
        }
        if ((buffer[i] & 0x000000F0) == 0xC0)
        {
        // printf("\tVALUES %X - 1\n", buffer[i]& 0x000000F0);
            bytes_to_read = 1;
        }
        if ((buffer[i] & 0x000000F0) == 0xE0)
        {
        // printf("\tVALUES %X - 2\n", buffer[i]& 0x000000F0);
            bytes_to_read = 2;
        }
        if ((buffer[i] & 0x000000FA) == 0XF0)
        {
        // printf("\tVALUES %X - 3\n", buffer[i]& 0x000000FA);
            bytes_to_read = 3;
        }

        char_utf8[bytes_to_read + 1] = '\0';

        for(int l=0; l<=bytes_to_read;l++){
            char_utf8[l]=buffer[i+l];
        }
        
        // printf("Char8:%s | Len:%li | Bytes:%i\n", char_utf8, strlen(char_utf8), bytes_to_read);

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
                reset_flags(&inWord, &has_vowel[0]);
            }
        } else {
            reset_flags(&inWord, &has_vowel[0]);
        }

        if (inWord == 1){
            if (strlen(char_utf8) == 2 && (unsigned char)char_utf8[0] == 0xC3){
                if (((unsigned char)char_utf8[1] >= 0xA0 && (unsigned char)char_utf8[1] <= 0xA3) ||
                    ((unsigned char)char_utf8[1] >= 0x80 && (unsigned char)char_utf8[1] <= 0x83)){
                    update_status(&has_vowel[0], &words_with_vowel[0]);
                } 
                if (((unsigned char)char_utf8[1] >= 0xA8 && (unsigned char)char_utf8[1] <= 0xAA) ||
                    ((unsigned char)char_utf8[1] >= 0x88 && (unsigned char)char_utf8[1] <= 0x8A)){
                    update_status(&has_vowel[1], &words_with_vowel[1]);
                }
                if (((unsigned char)char_utf8[1] >= 0xAC && (unsigned char)char_utf8[1] <= 0xAD) ||
                    ((unsigned char)char_utf8[1] >= 0x8C && (unsigned char)char_utf8[1] <= 0x8D)){
                    update_status(&has_vowel[2], &words_with_vowel[2]);
                }
                if (((unsigned char)char_utf8[1] >= 0xB2 && (unsigned char)char_utf8[1] <= 0xB5) ||
                    ((unsigned char)char_utf8[1] >= 0x92 && (unsigned char)char_utf8[1] <= 0x95)){
                    update_status(&has_vowel[3], &words_with_vowel[3]);
                }
                if (((unsigned char)char_utf8[1] >= 0xB9 && (unsigned char)char_utf8[1] <= 0xBA) ||
                    ((unsigned char)char_utf8[1] >= 0x99 && (unsigned char)char_utf8[1] <= 0x9A)){
                    update_status(&has_vowel[4], &words_with_vowel[4]);
                }
            } else {
                if ((unsigned char)char_utf8[0] == 0x41 || (unsigned char)char_utf8[0] == 0x61){
                    update_status(&has_vowel[0], &words_with_vowel[0]);
                } else if ((unsigned char)char_utf8[0] == 0x45 || (unsigned char)char_utf8[0] == 0x65){
                    update_status(&has_vowel[1], &words_with_vowel[1]);
                } else if ((unsigned char)char_utf8[0] == 0x49 || (unsigned char)char_utf8[0] == 0x69){
                    update_status(&has_vowel[2], &words_with_vowel[2]); 
                } else if ((unsigned char)char_utf8[0] == 0x4F || (unsigned char)char_utf8[0] == 0x6F){
                    update_status(&has_vowel[3], &words_with_vowel[3]); 
                } else if ((unsigned char)char_utf8[0] == 0x55 || (unsigned char)char_utf8[0] == 0x75){
                    update_status(&has_vowel[4], &words_with_vowel[4]); 
                } else if ((unsigned char)char_utf8[0] == 0x59 || (unsigned char)char_utf8[0] == 0x79){
                    update_status(&has_vowel[5], &words_with_vowel[5]);
                }
            }

        }
        memset(char_utf8, 0, sizeof(char_utf8));
        // char_utf8[bytes_to_read+1] = '\0';   
        i = i+bytes_to_read;
        // res = extract_char(buffer, char_utf8);
    }

    words_count[0] = total_words;

    return;
}


// int extract_char(char c, char * utf_8chr){
//     // char c;
//     printf("C:%c | Hex: %X | Bytes:", c, c);
//     int res, bytes_to_read = 0;
//     // res = fread(&c, sizeof(unsigned char), 1, f);
//     if(res == 0)
//         return  EOF;

//     if ((c & 0x000000A0) == 0b0000)
//     {
// //printf("VALUES %X - 0 \n", c& 0x000000A0);
//         bytes_to_read = 0;
//     }
//     if ((c & 0x000000F0) == 0xC0)
//     {
// //printf("\tVALUES %X - 1\n", c& 0x000000F0);
//         bytes_to_read = 1;
//     }
//     if ((c & 0x000000F0) == 0xE0)
//     {
// //printf("\tVALUES %X - 2\n", c& 0x000000F0);
//         bytes_to_read = 2;
//     }
//     if ((c & 0x000000FA) == 0XF0)
//     {
// //printf("\tVALUES %X - 3\n", c& 0x000000FA);
//         bytes_to_read = 3;
//     }

//     // utf_8chr[0] = c[0];
//     utf_8chr[bytes_to_read + 1] = '\0';

//     printf("%i\n", bytes_to_read);
//     for (int i = 0; i < bytes_to_read; i++) {
//         utf_8chr[i] = c;
//         // res = fread(&(utf_8chr[i+1]), sizeof(char), 1, f);
//         // if((utf_8chr[i+1] & 0x000000C0) != 0b10000000){
//         //     // fprintf(stderr, "UTF-8 file is corrupted %ld.",ftell(f));
//         //     printf("%s \nCounter go brrrrrrrrrr\n", c);
//         //     exit(EXIT_FAILURE);
//         // }
//         if(res == 0) return EOF;
//     }
//     // printf("Char: %s → Hex: %s", c, utf_8chr);
// //printf("String: |%s|\n", utf_8chr);
// //printf("Valor bool 0x%X ", utf_8chr[i]);
// //printf("\n");

//     return res == 0 ? EOF : 0;
// }
