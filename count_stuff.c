#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

void help_msg();
void process_file(const char *fname, const char vowel);

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
        printf("%s\n", argv[i]);
        process_file(argv[i], vowel);
    }
    exit(EXIT_SUCCESS);
}

void help_msg()
{
    printf("Help\n");
}

void process_file(const char *fname, const char vowel)
{
    printf("Counting vowel %c\n", vowel);

    FILE *f = fopen(fname, "rb");
    unsigned char c;
    unsigned char *char_utf8;
    int inWord = 0, // flag to say if you are insde a word or not
        res, // determines if the fread function had success or not
        bytes_to_read = 0; // bytes to read
    res = fread(&c, sizeof(unsigned char), 1, f);
    while (res == 1)
    {
        printf("\n---------------------------------\n");
        //printf("First CHAR: 0x%X \n", c);
        //printf("First CHAR: 0x%X, 0x%X \n", c, (c & 0xA));
        //printf("First CHAR: 0x%X, 0x%X \n", c, (c & 0xF0));
        //printf("First CHAR: 0x%X, 0x%X \n", c, (c & 0xF0));
        //printf("First CHAR: 0x%X, 0x%X \n", c, (c & 0xFA));
        if ((c & 0xA0) == 0b0000)
        {
            printf("VALUES %X - 0 \n", c);
            bytes_to_read = 0;
        }
        if ((c & 0xF0) == 0xC0)
        {
            printf("\tVALUES %X - 1\n", c);
            bytes_to_read = 1;
        }
        if ((c & 0xF0) == 0xE0)
        {
            printf("\tVALUES %X - 2\n", c);
            bytes_to_read = 2;
        }
        if ((c & 0xFA) == 0XF0)
        {
            printf("\tVALUES %X - 3\n", c);
            bytes_to_read = 3;
        }

        char_utf8 = (unsigned char *)malloc((bytes_to_read + 2) * sizeof(unsigned char));
        //printf("Size %ul, calc %d \n", strlen(char_utf8), bytes_to_read + 2);
        char_utf8[0] = c;
        char_utf8[bytes_to_read + 1] = '\0';

        if (bytes_to_read != 0)
            res = fread(&(char_utf8[1]), sizeof(unsigned char), bytes_to_read, f);

        printf("String: |%s|\n", char_utf8);
        printf("Valor bool %X , %X Controlo %d\n", char_utf8[0] , char_utf8[1], 1==1 );

        printf("TODO: Determine if inside word or if we left if we left increment counter\n");
        printf("TODO: DETERMIN IF THE current char matches the vowel we are counting\n");


        bytes_to_read = 0;
        res = fread(&c, sizeof(unsigned char), 1, f);
        free(char_utf8);
    }
    fclose(f);
}
