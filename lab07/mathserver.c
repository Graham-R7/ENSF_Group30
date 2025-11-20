#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <stdarg.h>

#define NUM_CONTEXTS 16
#define BUFFER_SIZE 1024
#define QUEUE_SIZE 128

typedef struct {
    double value;
    pthread_mutex_t mutex;
} Context;

typedef struct {
    char operation[10];
    int context_id;
    int operand;
} Task;

typedef struct {
    Task tasks[QUEUE_SIZE];
    int front, rear, count;
    pthread_mutex_t mutex;
    pthread_cond_t producer, consumer;
} TaskQueue;

int main(int argc, char* argv[]) {
    const char usage[] = "Usage: mathserver.out <input trace> <output trace>\n";
    char* input_trace;
    char buffer[BUFFER_SIZE];

    // Parse command line arguments
    if (argc != 3) {
        printf("%s", usage);
        return 1;
    }
    
    input_trace = argv[1];

    // Open input and output files
    FILE* input_file = fopen(input_trace, "r");


    while ( !feof(input_file) ) {
        // Read input file line by line
        char *rez = fgets(buffer, sizeof(buffer), input_file);
        if ( !rez )
            break;
        else {
            // Remove endline character
            buffer[strlen(buffer) - 1] = '\0';
        }

        // CONTINUE...
    }
    fclose(input_file);

    return 0;
}