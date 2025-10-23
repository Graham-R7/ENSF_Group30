#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "parser.h"
#include <stdlib.h>

#define BUFLEN 1024
#define MAX_ARGS 64

//To Do: This base file has been provided to help you start the lab, you'll need to heavily modify it to implement all of the features

static void exec_path(char *const argv[]) {
    extern char **environ;
    char *cmd = argv[0];
    if (!cmd) _exit(127);

    if (cmd[0] == '/' || strchr(cmd, '/')) {
        execve(cmd, argv, environ);
        _exit(127);
    }

    const char *pe = getenv("PATH");
    if (!pe) _exit(127);

    char *paths = strdup(pe);
    if (!paths) _exit(127);
    char *save = NULL;

    for (char *dir = strtok_r(paths, ":", &save); dir; dir = strtok_r(NULL, ":", &save)) {
        char cand[BUFLEN];
        size_t dl = strnlen(dir, sizeof(cand)-1);
        size_t cl = strnlen(cmd, sizeof(cand)-1);
        if (dl + 1 + cl + 1 > sizeof(cand)) continue;
        strcpy(cand, dir);
        if (dl == 0 || dir[dl-1] != '/') strcat(cand, "/");
        strcat(cand, cmd);
        execve(cand, argv, environ);
    }
    free(paths);
    _exit(127);
}

int main() {
    char buffer[1024];
    char cleaned[BUFLEN];

    printf("Welcome to the Group30 shell! Enter commands, enter 'quit' to exit\n");
    do {
        //Print the terminal prompt and get input
        printf("$ ");
        char *input = fgets(buffer, sizeof(buffer), stdin);
        if(!input)
        {
            fprintf(stderr, "Error reading input\n");
            return -1;
        }

        trimstring(cleaned, buffer, BUFLEN);

        char *argv[MAX_ARGS + 1];
        for (int i = 0; i <= MAX_ARGS; ++i) argv[i] = NULL;

        int argc = tokenize(cleaned, argv, MAX_ARGS);
        if (argc < 0) {
            fprintf(stderr, "Parse error\n");
            continue;
        }
        if (argc == 0) {
            continue;
        }

        //Sample shell logic implementation
        if (strcmp(argv[0], "quit") == 0 && argc == 1) {
            for (int i = 0; i < argc; ++i) free(argv[i]);
            printf("Bye!!\n");
            return 0;
        }
        else {
            int pidx = -1;
            for (int i = 0; i < argc; ++i) {
                if (argv[i] && strcmp(argv[i], "|") == 0) { pidx = i; break; }
            }

            if (pidx >= 0) {
                if (pidx == 0 || pidx == argc - 1) {
                    printf("error: pipe with only one command\n");
                    for (int i = 0; i < argc; ++i) free(argv[i]);
                    continue;
                }

                int fds[2];
                if (pipe(fds) < 0) {
                    printf("pipe error\n");
                    for (int i = 0; i < argc; ++i) free(argv[i]);
                    continue;
                }

                pid_t left_pid = fork();
                if (left_pid < 0) {
                    printf("fork error\n");
                    close(fds[0]); close(fds[1]);
                    for (int i = 0; i < argc; ++i) free(argv[i]);
                    continue;
                }
                else if (left_pid == 0) {
                    if (dup2(fds[1], STDOUT_FILENO) < 0) { perror("dup2"); _exit(127); }
                    close(fds[0]); close(fds[1]);

                    argv[pidx] = NULL;
                    exec_path(argv);
                    _exit(127);
                }

                pid_t right_pid = fork();
                if (right_pid < 0) {
                    printf("fork error\n");
                    close(fds[0]); close(fds[1]);
                    int ts; waitpid(left_pid, &ts, 0);
                    for (int i = 0; i < argc; ++i) free(argv[i]);
                    continue;
                }
                else if (right_pid == 0) {
                    if (dup2(fds[0], STDIN_FILENO) < 0) {
                        printf("dup2 error\n");
                        _exit(127);
                    }                    close(fds[1]); close(fds[0]);

                    char **right_argv = &argv[pidx + 1];
                    exec_path(right_argv);
                    _exit(127);
                }

                close(fds[0]);
                close(fds[1]);

                int status;

                if (waitpid(right_pid, &status, 0) < 0) {
                    perror("waitpid");
                }

                for (int i = 0; i < argc; ++i) free(argv[i]);
                continue;
            } else {
                pid_t path_id = fork();

                if (path_id < 0) {
                    for (int i = 0; i < argc; ++i) free(argv[i]);
                    continue;
                }
                else if (path_id == 0) {
                    exec_path(argv);
                    _exit(127);
                }
                else {
                    int status;
                    if (waitpid(path_id, &status, 0) < 0) {
                        perror("waitpid");
                    }
                    for (int i = 0; i < argc; ++i) free(argv[i]);
                    continue;
                }
            }
        }

        //Remember to free any memory you allocate!

    } while ( 1 );

    return 0;
}