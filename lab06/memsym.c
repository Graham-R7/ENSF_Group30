#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>

#define NUMBER_REGISTERS 8
#define MEMORY_SIZE 1024
#define NUMBER_PROCESSES 4
#define TLB_SIZE 8

typedef struct {
    bool valid;
    int VPN;
    int PFN;
    int PID;
} TLBEntry;

TLBEntry tlb[TLB_SIZE];

typedef struct {
    uint32_t registers[NUMBER_REGISTERS];
} ProcessState;

ProcessState process_states[NUMBER_PROCESSES];
uint32_t registers[NUMBER_REGISTERS];

int* page_tables[NUMBER_PROCESSES];

bool define_called = false;
int current_pid = 0;
uint32_t* physical_memory = NULL;

// Output file
FILE* output_file;

// TLB replacement strategy (FIFO or LRU)
char* strategy;

char** tokenize_input(char* input) {
    char** tokens = NULL;
    char* token = strtok(input, " ");
    int num_tokens = 0;

    while (token != NULL) {
        num_tokens++;
        tokens = realloc(tokens, num_tokens * sizeof(char*));
        tokens[num_tokens - 1] = malloc(strlen(token) + 1);
        strcpy(tokens[num_tokens - 1], token);
        token = strtok(NULL, " ");
    }

    num_tokens++;
    tokens = realloc(tokens, num_tokens * sizeof(char*));
    tokens[num_tokens - 1] = NULL;

    return tokens;
}

void handle_define(int off, int pfn, int vpn, FILE* output_file) {
    //put mem_size as 2^(off+pfn) using bite shifting
    size_t mem_size = (size_t)(1 << (off + pfn));
    physical_memory = (uint32_t*)calloc(mem_size, sizeof(uint32_t));

    for (int i = 0; i < TLB_SIZE; i++) {
        tlb[i].valid = false;
    }
    for (int i = 0; i < NUMBER_PROCESSES; i++) {
        page_tables[i] = (int*)calloc((size_t)(1 << vpn), sizeof(int));
        memset(page_tables[i], -1, (size_t)(1 << vpn) * sizeof(int));
    }

    define_called = true;
    fprintf(output_file, "Current PID: %d. Memory instantiation complete. OFF bits: %d. PFN bits: %d. VPN bits: %d\n", current_pid, off, pfn, vpn);
}

void handle_ctxswitch(int pid, FILE* output_file) {
    // Validate PID
    if (pid < 0 || pid >= NUMBER_PROCESSES) {
        fprintf(output_file, "Current PID: %d. Invalid context switch to process %d\n", current_pid, pid);
        exit(1);
    }

    // Save current process's register state
    for (int i = 0; i < NUMBER_REGISTERS; i++) {
        process_states[current_pid].registers[i] = registers[i];
    }

    current_pid = pid;

    // Restore new process's register state (if any, otherwise just base value)
    for (int i = 0; i < NUMBER_REGISTERS; i++) {
        registers[i] = process_states[current_pid].registers[i];
    }

    fprintf(output_file, "Current PID: %d. Switched execution context to process: %d\n", current_pid, pid);
}

void handle_map(int vpn, int pfn, FILE* output_file) {

}

void handle_unmap(int vpn, FILE* output_file) {

}

void handle_pinspect(int vpn, FILE* output_file) {

}

void handle_tinspect(int tlbn, FILE* output_file) {

}

void handle_linspect(int pl, FILE* output_file) {

}

void handle_rinspect(int rn, FILE* output_file) {

}

void handle_load(int dst, int src, FILE* output_file) {

}

void handle_store(int dst, int src, FILE* output_file) {

}

void handle_add(FILE* output_file) {

}

void process_instruction(char** tokens, FILE* output_file) {
    //Cover empty lines or commented lines
    if (tokens[0] == NULL) return;
    if (tokens[0][0] == '%') return;

    //Cover no define
    if (!define_called && strcmp(tokens[0], "define") != 0) {
        fprintf(output_file, "Error: attempt to execute instruction before define\n");
        exit(1);
    }

    if (strcmp(tokens[0], "define") == 0) {
        if (define_called) {
            fprintf(output_file, "Current PID: %d. Error: multiple calls to define in the same trace\n", current_pid);
            exit(1);
        }
        else {
            int off = atoi(tokens[1]);
            int pfn = atoi(tokens[2]);
            int vpn = atoi(tokens[3]);
            handle_define(off, pfn, vpn, output_file);
        }
    }
    else if (strcmp(tokens[0], "ctxswitch") == 0) {
        int pid = atoi(tokens[1]);
        handle_ctxswitch(pid, output_file);
    }
    else if (strcmp(tokens[0], "map") == 0) {
        int vpn = atoi(tokens[1]);
        int pfn = atoi(tokens[1]);
        handle_map(vpn, pfn, output_file);
    }
    else if (strcmp(tokens[0], "unmap") == 0) {
        int vpn = atoi(tokens[1]);
        handle_unmap(vpn, output_file);
    }
    else if (strcmp(tokens[0], "pinspect") == 0) {
        int vpn = atoi(tokens[1]);
        handle_pinspect(vpn, output_file);
    }
    else if (strcmp(tokens[0], "tinspect") == 0) {
        int tlbn = atoi(tokens[1]);
        handle_tinspect(tlbn, output_file);
    }
    else if (strcmp(tokens[0], "linspect") == 0) {
        int pl = atoi(tokens[1]);
        handle_linspect(pl, output_file);
    }
    else if (strcmp(tokens[0], "rinspect") == 0) {
        int rn = atoi(tokens[1]);
        handle_rinspect(rn, output_file);
    }
    else if (strcmp(tokens[0], "load") == 0) {
        int dst = atoi(tokens[1]);
        int src = atoi(tokens[2]);
        handle_load(dst, src, output_file);
    }
    else if (strcmp(tokens[0], "store") == 0) {
        int dst = atoi(tokens[1]);
        int src = atoi(tokens[1]);
        handle_store(dst, src, output_file);
    }
    else if (strcmp(tokens[0], "add") == 0) {
        handle_add(output_file);
    }
}

int main(int argc, char* argv[]) {
    const char usage[] = "Usage: memsym.out <strategy> <input trace> <output trace>\n";
    char* input_trace;
    char* output_trace;
    char buffer[1024];

    // Parse command line arguments
    if (argc != 4) {
        printf("%s", usage);
        return 1;
    }
    strategy = argv[1];
    input_trace = argv[2];
    output_trace = argv[3];

    // Open input and output files
    FILE* input_file = fopen(input_trace, "r");
    output_file = fopen(output_trace, "w");  

    while ( !feof(input_file) ) {
        // Read input file line by line
        char *rez = fgets(buffer, sizeof(buffer), input_file);
        if ( !rez ) {
            fprintf(stderr, "Reached end of trace. Exiting...\n");
            return -1;
        } else {
            // Remove endline character
            buffer[strlen(buffer) - 1] = '\0';
        }
        char** tokens = tokenize_input(buffer);

        // TODO: Implement your memory simulator
        process_instruction(tokens, output_file);

        // Deallocate tokens
        for (int i = 0; tokens[i] != NULL; i++)
            free(tokens[i]);
        free(tokens);
    }

    // Close input and output files
    fclose(input_file);
    fclose(output_file);

    return 0;
}