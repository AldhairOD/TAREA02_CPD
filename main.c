#include "bucketsort.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>

#define LENGTH 8

FILE *fin, *fout, *fout_paralelo;

char *strings;
long int N;

void openfiles() {
  fin = fopen("./large/bucketsort.in", "r+");
  //fin = fopen("bucketsort.in", "r+");
  if (fin == NULL) {
    perror("fopen fin");
    exit(EXIT_FAILURE);
  }

  fout = fopen("bucketsort.out", "w");
  if (fout == NULL) {
    perror("fopen fout");
    exit(EXIT_FAILURE);
  }

  fout_paralelo = fopen("bucketsort_paralelo.out", "w");
  if (fout_paralelo == NULL) {
    perror("fopen fout_paralelo");
    exit(EXIT_FAILURE);
  }
}

void closefiles() {
  fclose(fin);
  fclose(fout);
  fclose(fout_paralelo);
}

void cleanup(long int *r, long int *l, char *strings) {
  free(r);
  free(l);
  free(strings);
  closefiles();
}

// Función para comparar resultados
int compare_results(char *strings, long int *r, long int *l, long int N) {
  for (long int i = 0; i < N; i++) {
    if (strcmp(strings + (r[i] * LENGTH), strings + (l[i] * LENGTH)) != 0) {
      return 0; // No son iguales
    }
  }
  return 1; // Son iguales
}

// Función para medir múltiples ejecuciones y calcular tiempo promedio
double measure_parallel_time(char *strings, long int N, int attempts) {
  double total_time = 0;
  long int *l;
  for (int j = 0; j < attempts; j++) {
    clock_t start = clock();
    l = bucket_sort_paralelo(strings, LENGTH, N);
    clock_t end = clock();
    total_time += ((double)(end - start)) / CLOCKS_PER_SEC;
    free(l);
  }
  return total_time / attempts;
}

int main(int argc, char *argv[]) {

  long int i, *r, *l;
  clock_t start, end;
  double cpu_time_used_sequential, cpu_time_used_parallel;

  openfiles();

  fscanf(fin, "%ld", &N);
  strings = (char *)malloc(N * LENGTH);
  if (strings == NULL) {
    perror("malloc strings");
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < N; i++) {
    fscanf(fin, "%s", strings + (i * LENGTH));
  }

  // Medir tiempo del algoritmo secuencial
  start = clock();
  r = bucket_sort(strings, LENGTH, N);
  end = clock();
  cpu_time_used_sequential = ((double)(end - start)) / CLOCKS_PER_SEC;

  printf("Tiempo de ejecución (secuencial): %.6f segundos\n", cpu_time_used_sequential);

  // Medir tiempo del algoritmo paralelo
  start = clock();
  l = bucket_sort_paralelo(strings, LENGTH, N);
  end = clock();
  cpu_time_used_parallel = ((double)(end - start)) / CLOCKS_PER_SEC;

// Guardar los resultados en los archivos después de validar
  for (i = 0; i < N; i++) {
    fprintf(fout, "%s\n", strings + (r[i] * LENGTH));
    fprintf(fout_paralelo, "%s\n", strings + (l[i] * LENGTH));
  }

  // Comparar resultados
  if (!compare_results(strings, r, l, N)) {
    printf("ERROR: Los resultados no son iguales entre la versión secuencial y la paralela.\n");
    cleanup(r, l, strings);
    return EXIT_FAILURE;
  }

  printf("Los resultados son iguales.\n");

  // Guardar los resultados en los archivos
  for (i = 0; i < N; i++) {
    fprintf(fout, "%s\n", strings + (r[i] * LENGTH));
    fprintf(fout_paralelo, "%s\n", strings + (l[i] * LENGTH));
  }
  printf("Tiempo de ejecución (paralelo): %.6f segundos\n", cpu_time_used_parallel);

  // Calcular tiempo promedio y speedup
  short attempts = 5;
  double prom_tiempo = measure_parallel_time(strings, N, attempts);
  double speedup = cpu_time_used_sequential / prom_tiempo;

  printf("Speedup: %.6f x\n", speedup);

  cleanup(r, l, strings);

  return EXIT_SUCCESS;
}
