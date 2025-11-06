#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    bool with_files = false;
    int timeout = -1;  // таймаут в секундах, -1 = не задан

    static struct option options[] = {
        {"seed", required_argument, 0, 0},
        {"array_size", required_argument, 0, 0},
        {"pnum", required_argument, 0, 0},
        {"by_files", no_argument, 0, 'f'},
        {"timeout", required_argument, 0, 0},
        {0, 0, 0, 0}
    };

    while (true) {
        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);
        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0: seed = atoi(optarg); break;
                    case 1: array_size = atoi(optarg); break;
                    case 2: pnum = atoi(optarg); break;
                    case 3: with_files = true; break;
                    case 4:
                        timeout = atoi(optarg);
                        if (timeout <= 0) {
                            printf("Timeout must be positive\n");
                            return 1;
                        }
                        break;
                    default:
                        printf("Unknown option index %d\n", option_index);
                        return 1;
                }
                break;
            case 'f': with_files = true; break;
            default: printf("Unknown option\n"); return 1;
        }
    }

    if (seed == -1 || array_size == -1 || pnum == -1) {
        printf("Usage: %s --seed num --array_size num --pnum num [--by_files] [--timeout num]\n", argv[0]);
        return 1;
    }

    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);
    int segment_size = array_size / pnum;

    pid_t *child_pids = malloc(sizeof(pid_t) * pnum);

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
            // дочерний процесс
            int begin = i * segment_size;
            int end = (i == pnum - 1) ? array_size : begin + segment_size;
            struct MinMax local_min_max = GetMinMax(array, begin, end);

            if (with_files) {
                char filename[256];
                sprintf(filename, "min_max_%d.txt", i);
                FILE *fp = fopen(filename, "w");
                if (!fp) { perror("fopen failed"); exit(1); }
                if (fprintf(fp, "%d %d", local_min_max.min, local_min_max.max) < 0) {
                    perror("fprintf failed"); exit(1);
                }
                fclose(fp);
            } else {
                ssize_t written = write(pipefd[1], &local_min_max, sizeof(struct MinMax));
                if (written != sizeof(struct MinMax)) { perror("write failed"); exit(1); }
            }

            free(array);
            exit(0);
        } else if (child_pid > 0) {
            child_pids[i] = child_pid;  // сохраняем PID
        } else {
            perror("fork failed");
            free(array);
            return 1;
        }
    }

    if (!with_files)
        close(pipefd[1]);  // закрываем запись в родителе

    // Обработка таймаута
    if (timeout > 0) {
        sleep(timeout);
        printf("Timeout reached, killing child processes...\n");
        for (int i = 0; i < pnum; i++) {
            kill(child_pids[i], SIGKILL);
        }
    }

    // ожидание всех дочерних процессов
    for (int i = 0; i < pnum; i++) {
        int status;
        waitpid(child_pids[i], &status, 0);
    }

    struct MinMax final_min_max;
    final_min_max.min = INT_MAX;
    final_min_max.max = INT_MIN;

    for (int i = 0; i < pnum; i++) {
        struct MinMax local_min_max;
        if (with_files) {
            char filename[256];
            sprintf(filename, "min_max_%d.txt", i);
            FILE *fp = fopen(filename, "r");
            if (!fp) { perror("fopen failed"); free(child_pids); free(array); return 1; }
            if (fscanf(fp, "%d %d", &local_min_max.min, &local_min_max.max) != 2) {
                perror("fscanf failed"); fclose(fp); free(child_pids); free(array); return 1;
            }
            fclose(fp);
        } else {
            ssize_t read_bytes = read(pipefd[0], &local_min_max, sizeof(struct MinMax));
            if (read_bytes != sizeof(struct MinMax)) { perror("read failed"); free(child_pids); free(array); return 1; }
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

    printf("Min: %d\n", final_min_max.min);
    printf("Max: %d\n", final_min_max.max);
    printf("Elapsed time: %fms\n", elapsed_time);

    free(child_pids);
    free(array);
    fflush(NULL);
    return 0;
}
