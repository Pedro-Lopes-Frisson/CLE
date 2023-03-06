#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

void help_msg();
void read_array(const char* arr);
void merge_sort(int* arr, size_t size);


int main(int argc, char * argv[]){
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
        read_array(argv[i]);
    }
    exit(EXIT_SUCCESS);
}
