#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s seed array_size\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // Дочерний процесс запускает sequential_min_max
        execv("./sequential_min_max", argv);
        perror("execv failed");
        exit(1);
    } else {
        // Родитель ждёт завершения дочернего процесса
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            printf("Child exited with code %d\n", WEXITSTATUS(status));
    }

    return 0;
}
