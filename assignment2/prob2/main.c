#include <mpi.h>
#include "distributor.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

void bitonic_sort(int *arr_1, int n, int flag);
void merge(int *arr_1, int n, int flag);

int main (int argc, char *argv[])
{
  MPI_Comm presentComm, nextComm;
  MPI_Group presentGroup, nextGroup;
  int gMemb[8];
  int rank, nProc, nProcNow, length, nIter;
  int *values = NULL, *recData;
  int i, j;
  
  static char *file_name;

  static FILE *f;

  static int number_of_values;

  static int iterations_required;

  MPI_Init (&argc, &argv);
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);
  MPI_Comm_size (MPI_COMM_WORLD, &nProc);

  if (nProc > 8)
    { if (rank == 0) printf ("Too many processes! It should be a power of 2, less or equal to 8.\n");
      MPI_Finalize ();
      return EXIT_FAILURE;
    }
  if ((nProc == 0) || ((nProc & (nProc - 1)) != 0))
    { if (rank == 0) printf ("Number of processes should be a power of 2, less or equal to 8.\n");
      MPI_Finalize ();
      return EXIT_FAILURE;
    }

  nIter = (int) (log2 (nProc) + 1.1);

  if (rank == 0)
    { /* Read file*/
      file_name = argv[1];

      f = fopen(file_name, "rb");
      size_t res;
      
      if (f == NULL) {
        fprintf(stderr, "File could not be opened %s.\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
      
      res = fread(&number_of_values, sizeof(int), 1, f);
      if (res == 0) {
        fprintf(stderr, "Error reading Size of array\n");
        exit(EXIT_FAILURE);
      }
      // fprintf(stdout, "Res %ld |Number %d\n", res,number_of_values / (1024));

      values = malloc(number_of_values * sizeof(int));

      for (int i = 0; i < number_of_values; i++) {
        
        res = fread(&(values[i]), sizeof(int), 1, f);
        if (res == 0) {
          fprintf(stderr, "Error reading value of array\n");
          exit(EXIT_FAILURE);
        }
        // fprintf(stdout, "%[%d]:%d\n",i, values[i]);
      }
      /**/

      for (i = 1; i < nProc; i++){
        MPI_Send (&number_of_values, 100, MPI_INT, i, 0, MPI_COMM_WORLD);
        MPI_Send (&nIter, 100, MPI_INT, i, 0, MPI_COMM_WORLD);
      }
    }
  else
    {
      int rec_num_vals, rec_nIter;
      MPI_Recv (&rec_num_vals, 100, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      number_of_values = rec_num_vals;
      MPI_Recv (&rec_nIter, 100, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      nIter = rec_nIter;
      values = malloc(number_of_values * sizeof(int));
    }
  /**/
  recData = malloc (number_of_values * sizeof (int));

  nProcNow = nProc;
  presentComm = MPI_COMM_WORLD;
  MPI_Comm_group (presentComm, &presentGroup);

  for (i = 0; i < 8; i++)
    gMemb[i] = i;
  for (j = 0; j < nIter; j++)
    { if (j > 0)
      { MPI_Group_incl (presentGroup, nProcNow, gMemb, &nextGroup);
        MPI_Comm_create(presentComm, nextGroup, &nextComm);
        presentGroup = nextGroup;
        presentComm = nextComm; 
        // printf ("Interation: %d\n",j);
        // printf ("Group: %d| Interation: %d\n", presentGroup, j);
        if (rank >= nProcNow)
          { free (recData);
          // while (true)
          // {
          //   /* code */
          // }
            MPI_Finalize ();
            exit(EXIT_SUCCESS);
          }
      }
    MPI_Comm_size (presentComm, &nProc);
    MPI_Scatter (values, number_of_values / nProcNow, MPI_INT, recData, number_of_values / nProcNow, MPI_INT, 0, presentComm);
    // printf ("Scattered data received by process %d with length = %d for a group of %d process(es).\n", rank, (number_of_values / nProcNow), nProc);
    if (j == 0)
      bitonic_sort(recData, number_of_values / nProcNow, rank % 2);
    else
      merge(recData, number_of_values / nProcNow, rank % 2);
    MPI_Gather (recData, number_of_values / nProcNow, MPI_INT, values, number_of_values / nProcNow, MPI_INT, 0, presentComm);
    // if (rank == 0)
      // printf ("Gathered data received by process 0 with length = %d for a group of %d process(es).\n", number_of_values, nProc);
    nProcNow = nProcNow >> 1;
  }

  /* Validate */
  if (rank == 0){ 
    for (int i = 0; i < number_of_values - 1; i++) {
    if (values[i] > values[i + 1]) {
      fprintf(stderr, "Values are not ordered %d, %d\n", values[i], values[i + 1]);
        break;
      }
    }
  }
  printf("Values Are ordered\n");
  MPI_Finalize ();

  return EXIT_SUCCESS;
}

/**
 * Worker 
 */

void bitonic_sort(int *arr_1, int n, int flag) {
  // the two first loops are the merging part
  // the last one is the sorting part
  for (int k = 2; k <= n; k = k * 2) {
    // sort the subsequence of k members to bitonic sequences
    for (int j = k / 2; j > 0; j = j / 2) {
      // Sort bitonic sequence of length k according to direction needed
      for (int i = 0; i < n; i++) {
        // sort elements
        int ij = i ^ j;// get the other index to be sorted
        if (ij > i) {
          int dir = ((i & k) == 0);// get direction
          //printf("k %d,j %d,i %d,ij %d,dir %d\n",k,j,i,ij,dir);
          //CAPS
          //Ascending
          if (flag == 0) {
            if ((arr_1[i] > arr_1[ij]) == dir) {
              int temp = arr_1[i];
              arr_1[i] = arr_1[ij];
              arr_1[ij] = temp;
            }
          }
          // Descending
          else {
            if ((arr_1[i] < arr_1[ij]) == dir) {
              int temp = arr_1[i];
              arr_1[i] = arr_1[ij];
              arr_1[ij] = temp;
            }
          }
          //END CAPS
        }
      }
    }
  }
}

void merge(int *arr_1, int n, int flag) {
  int k = n;
  for (int j = k / 2; j > 0; j = j / 2) {
    // Sort bitonic sequence of length k according to direction needed
    for (int i = 0; i < n; i++) {
      // sort elements
      int ij = i ^ j;// get the other index to be sorted
      if (ij > i) {
        // int dir = ((i & k) == 0); // get direction
        //printf("k %d,j %d,i %d,ij %d,dir %d\n",k,j,i,ij,dir);
        //CAPS
        // Ascending
        if (flag == 0) {
          // printf("%i → ", dir);
          if ((arr_1[i] > arr_1[ij])) {
            // printf("→→[%i|%i]",arr_1[i],arr_1[ij]);
            int temp = arr_1[i];
            arr_1[i] = arr_1[ij];
            arr_1[ij] = temp;
          }
        }
        // Descending
        else {
          if ((arr_1[i] < arr_1[ij])) {
            int temp = arr_1[i];
            arr_1[i] = arr_1[ij];
            arr_1[ij] = temp;
          }
        }
        //END CAPS
      }
    }
    k = k / 2;
  }
}
