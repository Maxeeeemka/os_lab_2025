#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // Дочерний процесс
        printf("Child process (PID: %d) running...\n", getpid());
        _exit(0);  // дочерний процесс завершается
    } else {
        // Родительский процесс
        printf("Parent process (PID: %d) created child with PID %d\n", getpid(), pid);
        printf("Parent will sleep for 30 seconds, during which child is a zombie.\n");
        sleep(30);  // ждём, не вызываем wait() — ребёнок станет зомби
        printf("Parent exiting\n");
    }

    return 0;
}
