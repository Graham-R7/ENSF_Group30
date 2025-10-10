#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "parser.h"

#define BUFLEN 1024
#define MAX_ARGS 64

//To Do: This base file has been provided to help you start the lab, you'll need to heavily modify it to implement all of the features

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
            pid_t path_id = fork();

            if (path_id < 0) {
                for (int i = 0; i < argc; ++i) free(argv[i]);
                continue;
            }
            else if (path_id == 0) {
                //takes parent's environment for use in exec
                extern char **environ;
                execve(argv[0], argv, environ);

                //Returned execve means failure, something went wrong
                perror("execve");
                _exit(127);
            }
            else {
                //Parent = wait for child
                int status;
                if (waitpid(path_id, &status, 0) < 0) {
                    perror("waitpid");
                }
                else {
                    if (WIFEXITED(status)) {
                        int exitcode = WEXITSTATUS(status);
                        //For seeing what happens
                        printf("Child exited with %d\n", exitcode);
                        (void)exitcode;
                    }
                    else if (WIFSIGNALED(status)) {
                        int signal = WTERMSIG(status);
                        printf("Child terminated by signal %d\n", signal);
                    }
                }
                for (int i = 0; i < argc; ++i) free(argv[i]);
            }
        }

        //Remember to free any memory you allocate!
    } while ( 1 );

    return 0;
}
