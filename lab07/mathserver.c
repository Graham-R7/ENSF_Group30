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

TaskQueue task_queue;
Context contexts[NUM_CONTEXTS];
FILE *output_file;

void handle_set(const int ctx, const int val, FILE* output_file) {

}

void handle_add(const int ctx, const int val, FILE* output_file) {

}

void handle_sub(const int ctx, const int val, FILE* output_file) {

}

void handle_mul(const int ctx, const int val, FILE* output_file) {

}

void handle_div(const int ctx, const int val, FILE* output_file) {

}

void handle_pri(const int ctx, FILE* output_file) {

}

void handle_pia(const int ctx, FILE* output_file) {

}

void handle_fib(const int ctx, FILE* output_file) {

}

void process_instruction(char** tokens, FILE* output_file) {
    if (tokens[0] == NULL) return;

    if (strcmp(tokens[0], "set") == 0) {
        int ctx = atoi(tokens[1]);
        int val = atoi(tokens[2]);
        handle_set(ctx, val, output_file);
    } else if (strcmp(tokens[0], "add") == 0) {
        int ctx = atoi(tokens[1]);
        int val = atoi(tokens[2]);
        handle_add(ctx, val, output_file);
    } else if (strcmp(tokens[0], "sub") == 0) {
        int ctx = atoi(tokens[1]);
        int val = atoi(tokens[2]);
        handle_sub(ctx, val, output_file);
    } else if (strcmp(tokens[0], "mul") == 0) {
        int ctx = atoi(tokens[1]);
        int val = atoi(tokens[2]);
        handle_mul(ctx, val, output_file);
    } else if (strcmp(tokens[0], "div") == 0) {
        int ctx = atoi(tokens[1]);
        int val = atoi(tokens[2]);
        handle_div(ctx, val, output_file);
    } else if (strcmp(tokens[0], "pri") == 0) {
        int ctx = atoi(tokens[1]);
        handle_pri(ctx, output_file);
    } else if (strcmp(tokens[0], "pia") == 0) {
        int ctx = atoi(tokens[1]);
        handle_pia(ctx, output_file);
    } else if (strcmp(tokens[0], "fib") == 0) {
        int ctx = atoi(tokens[1]);
        handle_fib(ctx, output_file);
    } else {
        fprintf(output_file, "Error: Unknown instruction %s\n", tokens[0]);
    }
}

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
    output_file = fopen("temp.txt", "w");
    if (!input_file || !output_file) {
        perror("Error opening files");
        return 1;
    }

    // Making Contexts(16)
    for (int i = 0; i < NUM_CONTEXTS; i++) {
        contexts[i].value = 0;
        pthread_mutex_init(&contexts[i].mutex, NULL);
    }

    // Making threads(16)
    pthread_t threads[NUM_CONTEXTS];
    for (int i = 0; i < NUM_CONTEXTS; i++) {
       
    }

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