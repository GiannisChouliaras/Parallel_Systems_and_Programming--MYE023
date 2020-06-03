/* 
 * Parallel program for matrix-matrix product.
 * {CJ} Ioannis Chouliaras, AM: 2631 using
 * VVD's serial program
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define N 1024
#define BOSS 0
#define _mat(i, j) (mat[(i)*n + (j)])
#define COMMUNICATOR MPI_COMM_WORLD

int A[N][N], B[N][N], C[N][N];

/* Functions for reading and writing to files */
int readmat(char *fname, int *mat, int n);
int writemat(char *fname, int *mat, int n);

int main(int argc, char **argv)
{
  int ierr, my_rank, nprocs, work, work_from, work_to;
  int i, j, k, sum;
  double start_time, end_time, comm_start_time,
      comm_end_time, total_comm_time;

  /* Start MPI */
  ierr = MPI_Init(&argc, &argv);
  if (ierr != 0)
    MPI_Abort(COMMUNICATOR, 1);

  ierr = MPI_Comm_rank(COMMUNICATOR, &my_rank);
  ierr = MPI_Comm_size(COMMUNICATOR, &nprocs);

  if (N % nprocs != 0)
  {
    if (my_rank == BOSS)
      printf("Matrix size not divisible :( \n");
    MPI_Finalize();
    exit(-1);
    /* but thanks for this. */
  }

  work = N / nprocs;
  work_from = my_rank * work;     /* every process offset to start. */
  work_to = (my_rank + 1) * work; /* where should process stop calculating.*/

  if (my_rank == BOSS) /* Welcome messages and reading arrays from file. */
  {
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("I am the process %d and i will be the BOSS here\n", my_rank);
    printf("I will read the matrices and send to workers their part.\n");
    printf("reading...\n");

    /* Read matrices from files: "A_file", "B_file"
     */
    if (readmat("Amat1024.txt", (int *)A, N) < 0)
      exit(1 + printf("file problem\n"));
    if (readmat("Bmat1024.txt", (int *)B, N) < 0)
      exit(1 + printf("file problem\n"));

    printf("Now I will send all B matrix but from A only the suitable part.\n");
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    comm_start_time = MPI_Wtime(); /* start clock for communication. */
  }
  /* send all with MPI_BCast */
  MPI_Bcast(B, N * N, MPI_INT, BOSS, COMMUNICATOR);
  /* send only the suitable part for every process with MPI_Scatter. */
  MPI_Scatter(A, N * work, MPI_INT, A[work_from], N * work, MPI_INT, BOSS, COMMUNICATOR);

  if (my_rank == BOSS) /* only for the clock. Proceed for computation. */
  {
    comm_end_time = MPI_Wtime();
    total_comm_time = comm_end_time - comm_start_time;
    start_time = MPI_Wtime(); /* start the clock for computation */
  }

  /* Parallel Computation from every process (including BOSS of course). */
  for (i = work_from; i < work_to; i++)
    for (j = 0; j < N; j++)
    {
      for (k = sum = 0; k < N; k++)
        sum += A[i][k] * B[k][j];
      C[i][j] = sum;
    }

  if (my_rank == BOSS)
  {
    end_time = MPI_Wtime();        // stop the clock for computation here.
    comm_start_time = MPI_Wtime(); // start the clock for communication
  }

  /* every process will send their C matrix to process BOSS. */
  MPI_Gather(C[work_from], N * work, MPI_INT, C, N * work, MPI_INT, BOSS, COMMUNICATOR);
  if (my_rank == BOSS) // everything is done, write to Cmat file and show the results.
  {
    comm_end_time = MPI_Wtime(); // stop the clock for communication
    total_comm_time += comm_end_time - comm_start_time;
    printf("\n\nI am process BOSS -- (%d)\n", my_rank);
    printf("Just gathered all C matrices and in my C\n");
    printf("I will write them in file.\n");

    /* Save result in "Cmat1024"*/
    writemat("Cmat1024.txt", (int *)C, N);

    printf("Everything went well... bye bye!\n\n");
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("Communication between the procceses took %.2f seconds\n", total_comm_time);
    printf("Computation of matrix C took %.2f seconds\n", end_time - start_time);
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("\n\n");
  }

  /* Finish MPI */
  ierr = MPI_Finalize();
  return 0;
}

/* Utilities to read & write matrices from/to files
 * VVD
 */
int readmat(char *fname, int *mat, int n)
{
  FILE *fp;
  int i, j;

  if ((fp = fopen(fname, "r")) == NULL)
    return (-1);
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      if (fscanf(fp, "%d", &_mat(i, j)) == EOF)
      {
        fclose(fp);
        return (-1);
      };
  fclose(fp);
  return (0);
}

int writemat(char *fname, int *mat, int n)
{
  FILE *fp;
  int i, j;

  if ((fp = fopen(fname, "w")) == NULL)
    return (-1);
  for (i = 0; i < n; i++, fprintf(fp, "\n"))
    for (j = 0; j < n; j++)
      fprintf(fp, " %d", _mat(i, j));
  fclose(fp);
  return (0);
}
