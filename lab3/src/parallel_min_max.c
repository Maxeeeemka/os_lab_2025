#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <limits.h>

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    bool with_files = false;

    while (true) {
        static struct option options[] = {
            {"seed", required_argument, 0, 0},
            {"array_size", required_argument, 0, 0},
            {"pnum", required_argument, 0, 0},
            {"by_files", no_argument, 0, 'f'},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        seed = atoi(optarg);
                        if (seed <= 0) {
                            printf("Seed must be positive\n");
                            return 1;
                        }
                        break;
                    case 1:
                        array_size = atoi(optarg);
                        if (array_size <= 0) {
                            printf("Array size must be positive\n");
                            return 1;
                        }
                        break;
                    case 2:
                        pnum = atoi(optarg);
                        if (pnum <= 0) {
                            printf("Pnum must be positive\n");
                            return 1;
                        }
                        break;
                    case 3:
                        with_files = true;
                        break;
                    default:
                        printf("Index %d is out of options\n", option_index);
                }
                break;
            case 'f':
                with_files = true;
                break;
            default:
                printf("getopt returned character code 0%o?\n", c);
        }
    }

    if (seed == -1 || array_size == -1 || pnum == -1) {
        printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--by_files]\n", argv[0]);
        return 1;
    }

    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);

    int segment_size = array_size / pnum;

    int pipefd[2];
    if (!with_files) {
    if (pipe(pipefd) == -1) {  
        perror("pipe failed");
        return 1;
    }
}

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    for (int i = 0; i < pnum; i++) {
        pid_t child_pid = fork();
        if (child_pid == 0) {
            int begin = i * segment_size;
            int end = (i == pnum - 1) ? array_size : begin + segment_size;

            struct MinMax local_min_max = GetMinMax(array, begin, end);

            if (with_files) {
                // записываем результат в файл
                char filename[256];
                sprintf(filename, "min_max_%d.txt", i);
                FILE *fp = fopen(filename, "w");
                fprintf(fp, "%d %d", local_min_max.min, local_min_max.max);
                fclose(fp);
            } else {
                // записываем результат в pipe
                if (write(pipefd[1], &local_min_max, sizeof(struct MinMax)) != sizeof(struct MinMax)) 
                {
                  perror("write failed");
                  exit(1);
                }
            }

            free(array);
            exit(0);
        } else if (child_pid < 0) {
            printf("Fork failed!\n");
            return 1;
        }
    }

    // Родитель закрывает запись в pipe
    if (!with_files) {
    if (pipe(pipefd) == -1) {  
        perror("pipe failed");
        return 1;
    }
}



    while (wait(NULL) > 0);

    struct MinMax final_min_max;
    final_min_max.min = INT_MAX;
    final_min_max.max = INT_MIN;

    for (int i = 0; i < pnum; i++) {
        struct MinMax local_min_max;
        if (with_files) {
            char filename[256];
            sprintf(filename, "min_max_%d.txt", i);
            FILE *fp = fopen(filename, "r");
            if (fscanf(fp, "%d %d", &local_min_max.min, &local_min_max.max) != 2) 
            {
              perror("fscanf failed");
              exit(1);
            }
            fclose(fp);
        } else {
            if (read(pipefd[0], &local_min_max, sizeof(struct MinMax)) != sizeof(struct MinMax)) 
            {
              perror("read failed");
              exit(1);
            }

        }

        if (local_min_max.min < final_min_max.min)
            final_min_max.min = local_min_max.min;
        if (local_min_max.max > final_min_max.max)
            final_min_max.max = local_min_max.max;
    }

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    free(array);

    printf("Min: %d\n", final_min_max.min);
    printf("Max: %d\n", final_min_max.max);
    printf("Elapsed time: %fms\n", elapsed_time);
    fflush(NULL);
    return 0; 
  }
