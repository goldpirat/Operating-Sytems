#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "quiz.h"

char* fetch(char *url) {
    int pipefd[2];
    pid_t pid;

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return NULL;
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        return NULL;
    } else if (pid == 0) {
        /* Child process */
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        execlp("curl", "curl", "-s", url, NULL);
        perror("execlp");
        exit(1);
    } else {
        /* Parent process */
        close(pipefd[1]);
        char *response = NULL;
        size_t length = 0;
        char buf[1024];
        ssize_t bytes_read;

        while ((bytes_read = read(pipefd[0], buf, sizeof(buf))) > 0) {
            char *temp = realloc(response, length + bytes_read + 1);
            if (!temp) {
                free(response);
                close(pipefd[0]);
                return NULL;
            }
            response = temp;
            memcpy(response + length, buf, bytes_read);
            length += bytes_read;
            response[length] = '\0';
        }
        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            /* Curl failed */
            free(response);
            return NULL;
        }

        return response;
    }
}
