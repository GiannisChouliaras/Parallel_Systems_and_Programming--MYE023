/*
 * Parallel program for calculating  the prime numbers.
 * {CJ} Ioannis Chouliaras, AM: 2631 using
 * VVD's serial program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define UPTO 10000000
#define COMMUNICATOR MPI_COMM_WORLD
#define BOSS 0

int main(int argc, char **argv)
{
  long int finalCount, last,
      i, num, divisor, quotient, remainder, count, lastprime;
  int procid, nprocs, ierr;
  double start_time, end_time;
  double comm_start_time, comm_end_time;

  /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  * Start MPI */
  ierr = MPI_Init(&argc, &argv);
  if (ierr != 0)
    MPI_Abort(COMMUNICATOR, 1);

  ierr = MPI_Comm_rank(COMMUNICATOR, &procid);
  if (ierr != MPI_SUCCESS)
    MPI_Abort(COMMUNICATOR, 1);

  ierr = MPI_Comm_size(COMMUNICATOR, &nprocs);
  if (ierr != MPI_SUCCESS)
    MPI_Abort(COMMUNICATOR, 1);
  /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

  if (nprocs % 2 != 0)
  {
    printf("Number of procs must be even!\n"); /* .. thanks!*/
    ierr = MPI_Finalize();
    exit(-1);
  }

  if (procid == BOSS) /* Welcome message from BOSS */
  {
    if (UPTO < 2)
      return 0;
    printf("Hello, I am the process 0 (boss).\n");
    printf("Me and my crew will find the prime numbers.\n");
    start_time = MPI_Wtime(); /* start the computation clock */
  }

  /* Start the function --> computation */
  count = 0; /* cheat (?) */
  lastprime = 2;

  for (i = procid; i < (UPTO - 1) / 2; i = i + nprocs)
  {
    num = 2 * i + 3;
    divisor = 1;

    do
    {
      divisor += 2;
      quotient = num / divisor;
      remainder = num % divisor;
    } while (remainder && divisor <= quotient);

    if (remainder || divisor == num)
    {
      count++;
      lastprime = num;
    }
  }
  /* function finished */

  if (procid == BOSS)
  {
    end_time = MPI_Wtime();        /* stop computation clock */
    comm_start_time = MPI_Wtime(); /* start communication clock */
  }

  MPI_Reduce(&count, &finalCount, 1, MPI_LONG, MPI_SUM, BOSS, COMMUNICATOR);
  MPI_Reduce(&lastprime, &last, 1, MPI_LONG, MPI_MAX, BOSS, COMMUNICATOR);

  if (procid == BOSS)
  {
    comm_end_time = MPI_Wtime();                         // stop communication time
    printf("Calm, I will count 2 as a prime number!\n"); // cheat (?)
    finalCount += 1;
    printf("total count: %ld\nMax prime: %ld\n", finalCount, last);
    printf("\n\n");
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("Communication time between processes took %.2f seconds\n", comm_end_time - comm_start_time);
    printf("Computation time took %.2f seconds\n", end_time - start_time);
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("\n\n");
  }
  /* Finish MPI*/
  ierr = MPI_Finalize();
  return 0;
}
