#include "bucketsort.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define N_BUCKETS 94
#define LENGTH 8

typedef struct {
  long int *data;
  int length;
  long int total;
} bucket;

void sort(char *a, bucket *bucket) {
  int j, i;
  long int key;
  for (j = 1; j < bucket->total; j++) {
    key = bucket->data[j];
    i = j - 1;
    while (i >= 0 && strcmp(a + bucket->data[i] * bucket->length,
                            a + key * bucket->length) > 0) {
      bucket->data[i + 1] = bucket->data[i];
      i--;
    }
    bucket->data[i + 1] = key;
  }
}

long int *bucket_sort(char *a, int length, long int size) {
  long int i;
  bucket buckets[N_BUCKETS];
  long int *returns = (long int *)malloc(sizeof(long int) * size);
  long int *bucket_data[N_BUCKETS];

  // Allocate memory for each bucket
  for (i = 0; i < N_BUCKETS; i++) {
    bucket_data[i] = (long int *)malloc(sizeof(long int) * size);
    buckets[i].data = bucket_data[i];
    buckets[i].length = length;
    buckets[i].total = 0;
  }

  // Distribute elements into buckets
  for (i = 0; i < size; i++) {
    int bucket_index = *(a + i * length) - 0x21;
    if (bucket_index < 0 || bucket_index >= N_BUCKETS) {
      fprintf(stderr, "Invalid bucket index: %d\n", bucket_index);
      exit(EXIT_FAILURE);
    }
    buckets[bucket_index].data[buckets[bucket_index].total++] = i;
  }

  // Sort each bucket
  for (i = 0; i < N_BUCKETS; i++)
    sort(a, &buckets[i]);

  // Collect sorted elements
  long int index = 0;
  for (i = 0; i < N_BUCKETS; i++) {
    for (int j = 0; j < buckets[i].total; j++) {
      returns[index++] = buckets[i].data[j];
    }
  }

  // Free bucket memory
  for (i = 0; i < N_BUCKETS; i++) {
    free(bucket_data[i]);
  }

  return returns;
}

long int *bucket_sort_paralelo(char *a, int length, long int size) {
  long int i;
  bucket buckets[N_BUCKETS];
  long int *returns = (long int *)malloc(sizeof(long int) * size);
  long int *bucket_data[N_BUCKETS];

  // Inicializar buckets globales
  #pragma omp parallel for schedule(static)
  for (i = 0; i < N_BUCKETS; i++) {
    bucket_data[i] = (long int *)malloc(sizeof(long int) * size);
    buckets[i].data = bucket_data[i];
    buckets[i].length = length;
    buckets[i].total = 0;
  }

  // Crear buckets locales por hilo y distribuir elementos
  #pragma omp parallel
  {
    int num_threads = omp_get_num_threads();
    int thread_num = omp_get_thread_num();

    // Preasignar memoria para los buckets locales
    long int *local_bucket_data[N_BUCKETS];
    bucket local_buckets[N_BUCKETS];
    for (int b = 0; b < N_BUCKETS; b++) {
      local_bucket_data[b] = (long int *)malloc(sizeof(long int) * (size / num_threads + 1));
      local_buckets[b].data = local_bucket_data[b];
      local_buckets[b].length = length;
      local_buckets[b].total = 0;
    }

    // Distribuir elementos en los buckets locales
    #pragma omp for schedule(static)
    for (i = 0; i < size; i++) {
      int bucket_index = *(a + i * length) - 0x21;
      if (bucket_index < 0 || bucket_index >= N_BUCKETS) {
        fprintf(stderr, "Invalid bucket index: %d\n", bucket_index);
        exit(EXIT_FAILURE);
      }
      local_buckets[bucket_index].data[local_buckets[bucket_index].total++] = i;
    }

    // Fusión de buckets locales con los globales
    #pragma omp critical
    for (int b = 0; b < N_BUCKETS; b++) {
      int local_total = local_buckets[b].total;
      if (local_total > 0) {
        memcpy(buckets[b].data + buckets[b].total, local_buckets[b].data, local_total * sizeof(long int));
        buckets[b].total += local_total;
      }
      free(local_bucket_data[b]);
    }
  }

  // Verificación para asegurar que los datos están ordenados correctamente
  for (int b = 0; b < N_BUCKETS; b++) {
    if (buckets[b].total > size) {
      fprintf(stderr, "ERROR: El número de elementos en el bucket %d excede el tamaño permitido.\n", b);
      exit(EXIT_FAILURE);
    }
  }

  // Ordenar cada bucket global
  #pragma omp parallel for schedule(static)
  for (i = 0; i < N_BUCKETS; i++) {
    sort(a, &buckets[i]);
  }

  // Recolectar elementos ordenados de los buckets globales
  long int index = 0;
  for (i = 0; i < N_BUCKETS; i++) {
    for (int j = 0; j < buckets[i].total; j++) {
      returns[index++] = buckets[i].data[j];
    }
  }

  // Liberar memoria de los buckets globales
  #pragma omp parallel for schedule(static)
  for (i = 0; i < N_BUCKETS; i++) {
    free(bucket_data[i]);
  }

  return returns;
}

