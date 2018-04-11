#include "find_min_max.h"

#include <limits.h>

struct MinMax GetMinMax(int *array, unsigned int begin, unsigned int end) {
  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  int *current = array + begin;
  int *range_end = array + end;
  
  while (current < range_end)
    {
        if (min_max.min>*current)
        {
              min_max.min = *current;
        }
          
        if (min_max.max<*current)
        {
              min_max.max = *current;
        }
        
        ++current;
    }
  
  return min_max;
}
