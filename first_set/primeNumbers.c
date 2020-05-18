/*
 * name: Ioannis Chouliaras
 * AM  : 2631
 */

#include <stdio.h>
#include <omp.h>

#define UPTO 10000000

long int count,      /* number of primes */
lastprime;           /* the last prime found */


double serial_primes(long int n) {
  long int i, num, divisor, quotient, remainder;
  double start, end;

  if (n < 2) return 0.0;
  count = 1;                         /* 2 is the first prime */
  lastprime = 2;

  start = omp_get_wtime();
  for (i = 0; i < (n-1)/2; ++i) {    /* For every odd number */
    num = 2*i + 3;

    divisor = 1;
    do
    {
      divisor += 2;                  /* Divide by the next odd */
      quotient  = num / divisor;
      remainder = num % divisor;
    } while (remainder && divisor <= quotient);  /* Don't go past sqrt */

    if (remainder || divisor == num) /* num is prime */
    {
      count++;
      lastprime = num;
    }
  }
  end = omp_get_wtime();
  return end - start;
}

// do work here broda. Do not change anything in code. just add omp code.
double openmp_primes(long int n) {
  long int i, num, divisor, quotient, remainder;
  double start, end;

  if (n < 2) return 0.0;
  count = 1;                         /* 2 is the first prime */
  lastprime = 2;

  // set 4 threads here but first set dynamic(0)
  omp_set_dynamic(0);
  omp_set_num_threads(4);

  //start timer here
  start = omp_get_wtime();

  // parallel here using pragma omp parallel for..
  // set the values : (i, quotient, remainder, num and divisor) as (privates)
  // set the value : (lastPrime) as (lastprivate)
  // set the values : (count) as (shared) #default
  #pragma omp parallel for private(quotient, remainder, num, divisor)  default(shared)
  for (i = 0; i < (n-1)/2; ++i) {    /* For every odd number */
    num = 2*i + 3;

    divisor = 1;
    do
    {
      divisor += 2;                  /* Divide by the next odd */
      quotient  = num / divisor;
      remainder = num % divisor;
    } while (remainder && divisor <= quotient);  /* Don't go past sqrt */

    if (remainder || divisor == num) /* num is prime */
    {
      #pragma omp critical
        count++;
      lastprime = num;
    }
  }

  // end timer here
  end = omp_get_wtime();

  // return the difference
  return end - start;
}

int main()
{
  double serial_time, parallel_time;
  serial_time = parallel_time = 0.0;
  printf("Serial and parallel prime number calculations:\n\n");

  /* Time the following to compare performance */
  serial_time = serial_primes(UPTO);          /* time it */
  printf("[serial] count = %ld, last = %ld (time = %.2f seconds)\n", count, lastprime, serial_time);
  parallel_time = openmp_primes(UPTO);        /* time it */
  printf("[openmp] count = %ld, last = %ld (time = %.2f seconds)\n", count, lastprime, parallel_time);
  return 0;
}
