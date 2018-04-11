#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct SumArgs {
  int *array;
  int begin;
  int end;
};

int Sum(struct SumArgs *args) {
  int sum = 0;
  for(int i = args->begin; i<args->end; i++)
  {
        sum += args->array[i];
  }
  return sum;
}

void *ThreadSum(void *args) {
  struct SumArgs *sum_args = (struct SumArgs *)args;
  return (void *)(size_t)Sum(sum_args);
}