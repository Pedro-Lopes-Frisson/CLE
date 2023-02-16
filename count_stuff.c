#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

void help_msg();
void process_file(const char *fname);

int main(int argc, char *argv[])
{
    int opt;
    unsigned char *char_arr;
    while ((opt = getopt(argc, argv, "hp:")) != -1)
    {
        switch (opt)
        {
        case 'p':
            printf("Hex Values include: ");
            for (int i = 0; i < strlen(optarg); i++)
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
        process_file(argv[i]);
    }
    exit(EXIT_SUCCESS);
}

void help_msg()
{

    printf("Help\n");
}

void process_file(const char *fname)
{
    FILE *f = fopen(fname, "rb");
    char c;
    fread(&c, sizeof(char), 1, f);

    if ((c & 0xA) == 0b0000)
    {
        printf("%X - 1 \n", c);
    }
    if ((c & 0xc) == 0b1100)
    {
        printf("%X - 2\n", c);
    }
    if ((c & 0xf) == 0b1110)
    {
        printf("%X - 3\n", c);
    }
    if ((c & 0xFA) == 0XF0)
    {
        printf("%X - 4\n", c);
    }
    fclose(f);
}
