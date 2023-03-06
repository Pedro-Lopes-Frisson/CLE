#include <errno.h>
#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))



// return EOF if EOF is REACHED 0 otherwise
// This function can abort the program if the file is detected to be corrupt
int extract_int(FILE * f, int * num);

// Process text file in way that we compile number of words and number of words that contain a vowel
void process_file(const char *fname);
void bitonic_sort(int * arr, int n) {
    // the two first loops are the merging part
    // the last one is the sorting part
     for (int k = 2; k <= n; k = k * 2) {
        // sort the subsequence of k members to bitonic sequences
        for (int j = k / 2; j > 0; j = j / 2) {
            // Sort bitonic sequence of length k according to direction needed
            for (int i = 0; i < n; i++) {
                // sort elements
                int ij = i ^ j; // get the other index to be sorted
                if (ij > i) {
                    int dir = ((i & k) == 0); // get direction
                    //printf("k %d,j %d,i %d,ij %d,dir %d\n",k,j,i,ij,dir);
                    //CAPS
                    if ((arr[i] > arr[ij]) == dir) {
                        int temp = arr[i];
                        arr[i] = arr[ij];
                        arr[ij] = temp;
                    }
                    //END CAPS
                }
            }
        }
    }
}
void help_msg();


void help_msg() {
    return;
}
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
    //int nums[] = {8,4,7,5,1,2,3,6};
    //bitonic_sort(nums, ARRAY_SIZE(nums));

    int idx_start = 0;
    for (int i = optind; i < argc; i++)
    {
        process_file(argv[i]);
    }

    exit(EXIT_SUCCESS);
}

int extract_int(FILE * f, int *num){
    return fread(num, sizeof(*num), 1, f);
}


void process_file(const char *fname){
    FILE *f = fopen(fname, "rb");
    int number_of_values;
    int *values;
    int res;
    if (f == NULL){
        fprintf(stderr, "File could not be opened %s.\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    res = extract_int(f, &number_of_values);
    if (res == 0){
        fprintf(stderr, "Error reading Size of array\n");
        exit(EXIT_FAILURE);
    }
    //fprintf(stdout, "Number %d\n", number_of_values / (1024));

    values = (int *) malloc(sizeof(int) * number_of_values);
    for (int i = 0; i < number_of_values;i++) {
        res = extract_int(f, &(values[i]));
        if (res == 0){
            fprintf(stderr, "Error reading value of array\n");
            exit(EXIT_FAILURE);
        }

        //fprintf(stdout, "[%d]\t %d\n",i, values[i]);
    }

    bitonic_sort(values, number_of_values);

    for(int i = 0; i < number_of_values - 1; i++){
        if(values[i] > values[i+1]){
            fprintf(stderr, "Values are not ordered %d, %d\n", values[i], values[i+1]);
        }
    }
    printf("\n");
    printf("\n");
    printf("\n");
    printf("\n");

    for(int i = 0; i < number_of_values - 1; i++){
        fprintf(stdout, "%d, ",values[i]);
    }
    fprintf(stdout, "%d\n",values[number_of_values - 1]);
}
