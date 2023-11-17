#define _POSIX_C_SOURCE  1
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

pid_t* ch_pid;
int pnum;

void alarm_handler()
{
  int i;
  for(i  = 0; i < pnum; i++)
    kill(ch_pid[i], SIGQUIT);
}

int main(int argc, char **argv) {
  int i;
  int seed = -1;
  int array_size = -1;
  pnum = -1;
  bool with_files = false;
  int timeout = -1;
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
              if(seed < 1)
	            {
                printf("seed is a positive number\n");
                return 1;
              }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0)
            {
                printf("array_size is a positive number\n");
                return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0)
            {
                printf("pnum is a positive number\n");
                return 1;
            }
            break;
          case 3:
            timeout = atoi(optarg);
            if (timeout <= 0)
            {
                printf("timeout is a positive number\n");
                return 1;
            }
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
  int active_child_processes = 0;
  int array_step = array_size / pnum;
  int last_step = array_size % pnum;
  if(last_step == 0) last_step = array_step;
  else array_step++;
  int pipe_fd[2];
  if(!with_files)
  {
    pipe(pipe_fd);
  }

  ch_pid = malloc(sizeof(pid_t) * pnum);

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    ch_pid[i] = child_pid;
    int local_step = i < pnum - 1 ? array_step : last_step;
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process
        struct MinMax *min_max = malloc(sizeof(struct MinMax));
        min_max[0] = GetMinMax(array, i * array_step, i * array_step + local_step);
	//printf("Find %d and %d\n", min_max[0].min, min_max[0].max);
        if (with_files)
 	{
	  char path[10];
	  sprintf(path, "%d.bin",i);
	  FILE *minmax_file;
	  if((minmax_file = fopen(path, "wb")) == NULL)
 	  {
   	    printf("ERROR openning for adding to  minmax file");
 	  }

	  if(fwrite(min_max,sizeof(struct MinMax),1, minmax_file) != 1)
	  {
	    printf("ERROR write minmax struct to file");
	    return 1;
	  }
	  fclose(minmax_file);
 	}
	 else
	 {
          // use pipe here
	  write(pipe_fd[1], min_max, sizeof(struct MinMax));
        }
	free(min_max);
        return 0;
      }

     if(!with_files) close(pipe_fd[1]);

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }
  int status;
  int pid_counter = 0;
  signal(SIGALRM, alarm_handler);
  if(timeout > 0)
  {
    while (active_child_processes > 0)
    {
      // your code here
      alarm(timeout);
      waitpid(ch_pid[pid_counter], &status, WNOHANG);
      active_child_processes -= 1;
      pid_counter++;
    }
  }
  else
  {
    while (active_child_processes > 0)
    {
      // your code here
      wait(&status);
      active_child_processes -= 1;
    }
  }

  struct MinMax *min_max = malloc(sizeof(struct MinMax));
  int min = INT_MAX;
  int max = INT_MIN;
  char filepath[10];
  FILE* minmax_file;
  size_t get_el;
  for (i = 0; i < pnum; i++)
 {
    if (with_files)
    {
      // read from files
      sprintf(filepath, "%d.bin",i);
      if((minmax_file = fopen(filepath, "rb")) == NULL)
          {
            printf("ERROR openning for reading to  minmax file");
          }

      if(fread(min_max, sizeof(struct MinMax), 1, minmax_file) != 1)
     {
	 printf("ERROR reading from file");
	 return 1;
     }
      fclose(minmax_file);
      remove(filepath);
    }
    else
    {
      // read from pipes
      read(pipe_fd[0], min_max, sizeof(struct MinMax));
    }
    if (min > min_max[0].min) min = min_max[0].min;
    if (max < min_max[0].max) max = min_max[0].max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  free(min_max);
  free(ch_pid);
  if(!with_files)  close(pipe_fd[0]);
  printf("Min: %d\n", min);
  printf("Max: %d\n", max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}