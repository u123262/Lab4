#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

#include "find_min_max.h"
#include "utils.h"

static int active_child_processes = 0;

static void timeout_function( int sig )
{
    printf("Timeout was reached!\n");
    while (active_child_processes >= 0) {
        int wpid = waitpid(-1, NULL, WNOHANG);
		
        if(wpid == -1)
        {
            if(errno == ECHILD) break;
        }
        else
        {
            active_child_processes -= 1;
		}
	}
    printf("Termination of the process!\n");
    kill(0, SIGKILL);
    return;
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  int timeout = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"timeout", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);
    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
              printf("Seed is > 0\n");
              return 1; 
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              printf("Array size is > 0\n");
              return 1; 
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) {
              printf("PNUM is > 0\n");
              return 1; 
            }
            break;
          case 3:
            timeout = atoi(optarg);
            if(timeout <= 0)
            {
                printf("Timeout is > 0\n");
                return 1;
            }
            
            signal(SIGALRM, timeout_function);
            alarm(timeout);
            
            break;
          case 4:
            with_files = true;
            break;
            

          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  int num_in_array = array_size/pnum;

  int** min_pipes = malloc(sizeof(int*) * pnum);
  int** max_pipes = malloc(sizeof(int*) * pnum);
  
  for (int i = 0; i < pnum; i++){
    min_pipes[i] = malloc(sizeof(int) * 2);
    pipe(min_pipes[i]);
  }
  
  for (int i = 0; i < pnum; i++){
    max_pipes[i] = malloc(sizeof(int) * 2);
    pipe(max_pipes[i]);
  }

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      active_child_processes += 1;
      if (child_pid == 0) {
        struct MinMax min_max = GetMinMax(array, num_in_array*i, num_in_array*(i+1));

        if (with_files) {
          char file_name[12];
          sprintf(file_name, "%d", i);

          FILE *fp = fopen(file_name, "w");
          fprintf(fp, "%d %d", min_max.min, min_max.max);
          fclose(fp);
        } else {
          write(min_pipes[i][1], &min_max.min, sizeof(int));
          write(max_pipes[i][1], &min_max.max, sizeof(int));
        }
        return 0;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  int status;
  while (active_child_processes > 0) {
    wait(&status);
    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      char file_name[12];
      sprintf(file_name, "%d", i);
      
      FILE *fp = fopen(file_name, "r");
      fscanf(fp, "%d %d", &min, &max);
      fclose(fp);
      remove(file_name);
    } else {
      read(min_pipes[i][0], &min, sizeof(int));
      read(max_pipes[i][0], &max, sizeof(int));

      close(min_pipes[i][0]);
      close(min_pipes[i][1]);
      close(max_pipes[i][0]);
      close(max_pipes[i][1]);
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }
  
  free(min_pipes);
  free(max_pipes);
  
  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}