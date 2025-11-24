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
    int done;
} TaskQueue;

Context contexts[NUM_CONTEXTS];
TaskQueue context_queues[NUM_CONTEXTS];
FILE *output_file;
pthread_mutex_t log_mutex;

void handle_set(const int ctx, const int val, FILE* output_file) {
    pthread_mutex_lock(&contexts[ctx].mutex);
    contexts[ctx].value = val;
    fprintf(output_file, "ctx %02d: set to value %d\n", ctx, val);
    pthread_mutex_unlock(&contexts[ctx].mutex);
}

void handle_add(const int ctx, const int val, FILE* output_file) {
    pthread_mutex_lock(&contexts[ctx].mutex);
    int before = contexts[ctx].value;
    int after = before + val;
    contexts[ctx].value = after;
    fprintf(output_file, "ctx %02d: add %d (result: %d)\n", ctx, val, after);
    pthread_mutex_unlock(&contexts[ctx].mutex);
}

void handle_sub(const int ctx, const int val, FILE* output_file) {
    pthread_mutex_lock(&contexts[ctx].mutex);
    int before = contexts[ctx].value;
    int after = before - val;
    contexts[ctx].value = after;
    fprintf(output_file, "ctx %02d: sub %d (result: %d)\n", ctx, val, after);
    pthread_mutex_unlock(&contexts[ctx].mutex);
}

void handle_mul(const int ctx, const int val, FILE* output_file) {
    pthread_mutex_lock(&contexts[ctx].mutex);

    int before = (int)contexts[ctx].value;
    int after  = before * val;
    contexts[ctx].value = after;

    fprintf(output_file, "ctx %02d: mul %d (result: %d)\n", ctx, val, after);

    pthread_mutex_unlock(&contexts[ctx].mutex);
}

void handle_div(const int ctx, const int val, FILE* output_file) {
    pthread_mutex_lock(&contexts[ctx].mutex);

    int before = (int)contexts[ctx].value;
    int after;
    if (val != 0) {
        after = before / val;
    } else {
        after = 0;
    }
    contexts[ctx].value = after;

    fprintf(output_file, "ctx %02d: div %d (result: %d)\n", ctx, val, after);

    pthread_mutex_unlock(&contexts[ctx].mutex);
}


void handle_pri(const int ctx, FILE* output_file) {

}

void handle_pia(const int ctx, FILE* output_file) {

}

void handle_fib(const int ctx, FILE* output_file) {
    pthread_mutex_lock(&contexts[ctx].mutex);
    int n = (int)contexts[ctx].value;
    if (n <= 0) {
        contexts[ctx].value = 0;
        fprintf(output_file, "ctx %02d: fib (result: 0)\n", ctx);
        pthread_mutex_unlock(&contexts[ctx].mutex);
        return;
    }
    long long a = 0, b = 1;
    for (int i = 1; i < n; i++) {
        long long tmp = a + b;
        a = b;
        b = tmp;
    }
    contexts[ctx].value = (double)b;
    fprintf(output_file, "ctx %02d: fib (result: %lld)\n", ctx, b);
    pthread_mutex_unlock(&contexts[ctx].mutex);
}

void* context_thread(void* arg) {
    // Setup
    int ctx = *(int*)arg;
    TaskQueue* q = &context_queues[ctx];

    while (1) {
        pthread_mutex_lock(&q->mutex);

        while (q->count == 0 && !q->done) {
            pthread_cond_wait(&q->consumer, &q->mutex);
        }

        if (q->count == 0 && q->done) {
            pthread_mutex_unlock(&q->mutex);
            break;
        }

        Task task = q->tasks[q->front];
        q->front = (q->front + 1) % QUEUE_SIZE;
        q->count--;

        pthread_mutex_unlock(&q->mutex);
        pthread_cond_signal(&q->producer);

        // Execute current instruction
        char* operation = task.operation;
        int ctx_id = task.context_id;
        int val = task.operand;

        if (strcmp(operation, "set") == 0) handle_set(ctx_id, val, output_file);
        else if (strcmp(operation, "add") == 0) handle_add(ctx_id, val, output_file);
        else if (strcmp(operation, "sub") == 0) handle_sub(ctx_id, val, output_file);
        else if (strcmp(operation, "mul") == 0) handle_mul(ctx_id, val, output_file);
        else if (strcmp(operation, "div") == 0) handle_div(ctx_id, val, output_file);
        else if (strcmp(operation, "pri") == 0) handle_pri(ctx_id, output_file);
        else if (strcmp(operation, "pia") == 0) handle_pia(ctx_id, output_file);
        else if (strcmp(operation, "fib") == 0) handle_fib(ctx_id, output_file);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    const char usage[] = "Usage: mathserver.out <input trace> <output trace>\n";
    char buffer[BUFFER_SIZE];

    // Parse command line arguments
    if (argc != 3) {
        printf("%s", usage);
        return 1;
    }

    char* input_trace = argv[1];
    char* output_trace = argv[2];

    // Open input and output files
    FILE* input_file = fopen(input_trace, "r");
    output_file = fopen(output_trace, "w");
    if (!input_file || !output_file) {
        perror("Error opening files");
        return 1;
    }

    // Initialize
    for (int i = 0; i < NUM_CONTEXTS; i++) {
        contexts[i].value = 0;
        pthread_mutex_init(&contexts[i].mutex, NULL);
    }

    pthread_mutex_init(&log_mutex, NULL);

    // Initialize queues
    for (int i = 0; i < NUM_CONTEXTS; i++) {
        context_queues[i].front = 0;
        context_queues[i].rear  = 0;
        context_queues[i].count = 0;
        context_queues[i].done  = 0;   // <--- add this
        pthread_mutex_init(&context_queues[i].mutex, NULL);
        pthread_cond_init(&context_queues[i].producer, NULL);
        pthread_cond_init(&context_queues[i].consumer, NULL);
    }

    pthread_t threads[NUM_CONTEXTS];
    int ids[NUM_CONTEXTS];
    for (int i = 0; i < NUM_CONTEXTS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, context_thread, &ids[i]);
    }

    // Read instructions line-by-line
    while (fgets(buffer, sizeof(buffer), input_file)) {
        buffer[strcspn(buffer, "\n")] = 0;
        // Parse
        char* tokens[4];
        int tok_i = 0;
        char* tok = strtok(buffer, " ");
        while (tok && tok_i < 4) {
            tokens[tok_i++] = tok;
            tok = strtok(NULL, " ");
        }

        // Skip empty lines
        if (tok_i == 0) continue;

        Task task;
        strcpy(task.operation, tokens[0]);
        task.context_id = atoi(tokens[1]);
        task.operand = (tok_i >= 3 ? atoi(tokens[2]) : 0);

        int ctx = task.context_id;
        TaskQueue* q = &context_queues[ctx];

        // Queue task
        pthread_mutex_lock(&q->mutex);

        // Wait if queue is full
        while (q->count == QUEUE_SIZE) {
            pthread_cond_wait(&q->producer, &q->mutex);
        }

        q->tasks[q->rear] = task;
        q->rear = (q->rear + 1) % QUEUE_SIZE;
        q->count++;

        pthread_mutex_unlock(&q->mutex);
        pthread_cond_signal(&q->consumer);
    }

    fclose(input_file);

    for (int i = 0; i < NUM_CONTEXTS; i++) {
        pthread_mutex_lock(&context_queues[i].mutex);
        context_queues[i].done = 1;
        pthread_cond_signal(&context_queues[i].consumer);
        pthread_mutex_unlock(&context_queues[i].mutex);
    }

    for (int i = 0; i < NUM_CONTEXTS; i++) {
        pthread_join(threads[i], NULL);
    }

    fclose(output_file);
    return 0;
}