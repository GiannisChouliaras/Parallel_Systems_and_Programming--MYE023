/* Serial program for matrix-matrix product.
 *
 * VVD
 * name: Ioannis Chouliaras
 * AM  : 2631
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024
int A[N][N], B[N][N], C[N][N];

/* Functions */
int readmat(char *fname, int *mat, int n),
writemat(char *fname, int *mat, int n);
void parallel_work(int S);

/* Main Method */
int main(int argc, char ** argv)
{
  int i, j, k, sum, tasks;
  double start, end;
  
  /* check argc */
  if (argc != 2)
    exit( 1 + printf("Give me one number for the S\n") );
  
  /* Take S from argv and atoi */
  int S = atoi(argv[1]);
  
  /* Check if S is correct */
  if (N%S != 0)
    exit( 1 + printf("Wrong S number\n") );
  
  tasks = (N/S)*(N/S);
  printf("I will make %d Tasks\n", tasks);
  
  /* Read matrices from files: "A_file", "B_file"
   */
  if (readmat("Amat1024.txt", (int *) A, N) < 0)
    exit( 1 + printf("file problem\n") );
  if (readmat("Bmat1024.txt", (int *) B, N) < 0)
    exit( 1 + printf("file problem\n") );
  
  omp_set_num_threads(4);
  start = omp_get_wtime();
  #pragma omp parallel firstprivate(S)
  {
    #pragma omp single
      parallel_work(S); // call my function!
  }
  end = omp_get_wtime();
  
  /* Save result in "Cmat1024"
   */
  writemat("Cmat1024.txt", (int *) C, N);
  printf("Work took %.2f\n", end - start);
  return (0);
}


/* Utilities to read & write matrices from/to files
 * VVD
 */

#define _mat(i,j) (mat[(i)*n + (j)])


int readmat(char *fname, int *mat, int n)
{
  FILE *fp;
  int  i, j;
  
  if ((fp = fopen(fname, "r")) == NULL)
    return (-1);
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      if (fscanf(fp, "%d", &_mat(i,j)) == EOF)
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
  int  i, j;
  
  if ((fp = fopen(fname, "w")) == NULL)
    return (-1);
  for (i = 0; i < n; i++, fprintf(fp, "\n"))
    for (j = 0; j < n; j++)
      fprintf(fp, " %d", _mat(i, j));
  fclose(fp);
  return (0);
}

void parallel_work(int S)
{
  int i, j, k, sum, p, t;
  
  for (i = 0; i < N; i = i+S)
    for (j = 0; j < N; j = j+S)
    {
      #pragma omp task shared(i, j) untied
      {
        for (p = i; p < i+S; p++)
          for(t = j; t < j+S; t++) {
            for (k = sum = 0; k < N; k++)
              sum += A[p][k]*B[k][t];
            C[p][t] = sum;
          }
      }
    }
}
