#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "parser.h"

#define BUFLEN 1024

//To Do: This base file has been provided to help you start the lab, you'll need to heavily modify it to implement all of the features

int main() {
    char buffer[1024];
    char* parsedinput;
    char* args[3];
    char newline;

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
        
        //Clean and parse the input string
        parsedinput = (char*) malloc(BUFLEN * sizeof(char));
        size_t parselength = trimstring(parsedinput, input, BUFLEN);

        //Sample shell logic implementation
        if ( strcmp(parsedinput, "quit") == 0 ) {
            printf("Bye!!\n");
            return 0;
        }
        else if (parsedinput[0] == '\0') {
            continue;
        }
        else {
            pid_t path_id = fork();

            if (path_id < 0) {
                perror("fork");
                continue;
            }
            else if (path_id == 0) {
                //Child: execve expects argv array = passing parsedinput and a NULL terminator
                char *const argv[] = {parsedinput, NULL};
                extern char **environ; //takes parent's environment for use in exec
                execve(parsedinput, argv, environ);

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
            }
        }
        
        //Remember to free any memory you allocate!
        free(parsedinput);
    } while ( 1 );

    return 0;
}