#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    int timestamp;
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
int time_counter = 0;
int off_bits = 0;
int pfn_bits = 0;
int vpn_bits = 0;
int phys_words = 0;
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
    off_bits = off;
    pfn_bits = pfn;
    vpn_bits = vpn;

    /* total physical words (used by bounds checks) */
    phys_words = (1 << (off_bits + pfn_bits));

    size_t num_words = ((size_t)1 << (off_bits + pfn_bits));
    physical_memory = calloc(num_words, sizeof(uint32_t));

    /* Clearing TLB (also initialize fields to safe defaults) */
    for (int i = 0; i < TLB_SIZE; i++) {
        tlb[i].valid = false;
        tlb[i].VPN = -1;
        tlb[i].PFN = -1;
        tlb[i].PID = -1;
        tlb[i].timestamp = 0;
    }

    /* Allocating page tables: each process will get 2^vpn entries */
    size_t num_pages = ((size_t)1 << vpn_bits);
    for (int i = 0; i < NUMBER_PROCESSES; i++) {
        page_tables[i] = malloc(num_pages * sizeof(int));
        for (int j = 0; j < num_pages; j++) {
            page_tables[i][j] = -1;
        }
    }

    define_called = true;
    fprintf(output_file,
        "Current PID: %d. Memory instantiation complete. OFF bits: %d. PFN bits: %d. VPN bits: %d\n",
        current_pid, off_bits, pfn_bits, vpn_bits);
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

int find_tlb_entry(int pid, int vpn) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid && tlb[i].PID == pid && tlb[i].VPN == vpn)
            return i;
    }
    return -1;
}

int select_tlb_slot(void) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (!tlb[i].valid) return i;
    }
    int victim = 0;
    uint32_t best = tlb[0].timestamp;
    for (int i = 1; i < TLB_SIZE; i++) {
        if (tlb[i].timestamp < best) {
            best = tlb[i].timestamp;
            victim = i;
        }
    }
    return victim;
}

int get_reg_index(const char *reg) {
    if (strcmp(reg, "r1") == 0) return 1;
    if (strcmp(reg, "r2") == 0) return 2;

    fprintf(output_file,
            "Current PID: %d. Error: invalid register operand %s\n",
            current_pid, reg);
    exit(1);
}

int translate(int vaddr) {
    int vpn = vaddr >> off_bits;
    int offset = vaddr & ((1u << off_bits) - 1u);

    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid && tlb[i].PID == current_pid && tlb[i].VPN == vpn) {
            fprintf(output_file, "Current PID: %d. Translating. Lookup for VPN %d hit in TLB entry %d. PFN is %d\n", current_pid, vpn, i, tlb[i].PFN);

            if (strcmp(strategy, "LRU") == 0) tlb[i].timestamp = time_counter; 

            int pfn = tlb[i].PFN;
            return (pfn << off_bits) | offset;
        }
    }

    fprintf(output_file, "Current PID: %d. Translating. Lookup for VPN %d caused a TLB miss\n", current_pid, vpn);

    int pfn = page_tables[current_pid][vpn];
    if (pfn == -1) {
        fprintf(output_file, "Current PID: %d. Translating. Translation for VPN %d not found in page table\n", current_pid, vpn);
        exit(1);
    }

    fprintf(output_file, "Current PID: %d. Translating. Successfully mapped VPN %d to PFN %d\n", current_pid, vpn, pfn);

    int phys = (pfn << off_bits) | offset;
    return phys;
}

void handle_map(int vpn, int pfn, FILE* output_file) {
    page_tables[current_pid][vpn] = pfn;
    int idx = find_tlb_entry(current_pid, vpn);
    if (idx == -1) {
        idx = select_tlb_slot();
    }
    tlb[idx].valid = true;
    tlb[idx].VPN = vpn;
    tlb[idx].PFN = pfn;
    tlb[idx].PID = current_pid;
    tlb[idx].timestamp = time_counter;
    fprintf(output_file, "Current PID: %d. Mapped virtual page number %d to physical frame number %d\n", current_pid, vpn, pfn);
}

void handle_unmap(int vpn, FILE* output_file) {
    page_tables[current_pid][vpn] = -1;

    // Invalidate corresponding TLB entries
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid &&
            tlb[i].PID == current_pid &&
            tlb[i].VPN == vpn) 
        {
            tlb[i].valid = false;
        }
    }

    fprintf(output_file, "Current PID: %d. Unmapped virtual page number %d\n", current_pid, vpn);
}

void handle_pinspect(int vpn, FILE* output_file) {
    int pfn = page_tables[current_pid][vpn];
    int valid = (pfn >= 0) ? 1 : 0;

    int printed_pfn = valid ? pfn : 0;

    fprintf(output_file,"Current PID: %d. Inspected page table entry %d. Physical frame number: %d. Valid: %d\n", current_pid, vpn, printed_pfn, valid);
}

void handle_tinspect(int tlbn, FILE* output_file) {
    TLBEntry *e = &tlb[tlbn];
    fprintf(output_file, "Current PID: %d. Inspected TLB entry %d. VPN: %d. PFN: %d. Valid: %d. PID: %d. Timestamp: %d\n", current_pid, tlbn, e->VPN, e->PFN, e->valid ? 1 : 0, e->PID, e->timestamp);
}

void handle_linspect(int pl, FILE* output_file) {
    uint32_t value = 0;
    if (pl >= 0 && pl < phys_words && physical_memory != NULL) {
        value = physical_memory[pl];
    }
    fprintf(output_file, "Current PID: %d. Inspected physical location %d. Value: %u\n", current_pid, pl, value);
}

void handle_rinspect(int rn, FILE* output_file) {
    int reg_index;

    if (rn == 1) reg_index = 1;
    else if (rn == 2) reg_index = 2;
    else {
        fprintf(output_file, "Current PID: %d. Error: invalid register operand r%d\n", current_pid, rn);
        exit(1);
    }

    uint32_t value = registers[reg_index];
    fprintf(output_file, "Current PID: %d. Inspected register r%d. Content: %u\n", current_pid, rn, value);
}

void handle_load(char* dst_tok, char* src_tok, FILE* output_file) {
    int dst_index = -1;
    if (strcmp(dst_tok, "r1") == 0) { dst_index = 1; }
    else if (strcmp(dst_tok, "r2") == 0) { dst_index = 2; }
    else {
        fprintf(output_file, "Current PID: %d. Error: invalid register operand %s\n", current_pid, dst_tok);
        exit(1);
    }

    if (src_tok[0] == '#') {
        long val = strtol(src_tok + 1, NULL, 0);
        registers[dst_index] = (uint32_t)val;
        memmove(src_tok, src_tok + 1, strlen(src_tok));
        fprintf(output_file, "Current PID: %d. Loaded immediate %s into register %s\n", current_pid, src_tok, dst_tok);
    } else {
        long vaddr = strtol(src_tok, NULL, 10);
        int phys = translate((int)vaddr);

        if (phys < 0 || phys >= phys_words) {
            fprintf(output_file, "Current PID: %d. Error: physical address %d out of bounds\n", current_pid, phys);
            exit(1);
        }

        uint32_t val = physical_memory[phys];
        registers[dst_index] = val;
        fprintf(output_file, "Current PID: %d. Loaded value of location %s (%u) into register %s\n", current_pid, src_tok, val, dst_tok);
    }
}

void handle_store(char* dst_tok, char* src_tok, FILE* output_file) {
    int vaddr = (int)strtol(dst_tok, NULL, 10);
    int phys = translate(vaddr);

    if (phys < 0 || phys >= phys_words) {
        fprintf(output_file, "Current PID: %d. Error: physical address %d out of bounds\n", current_pid, phys);
        exit(1);
    }

    int value = 0;

        if (src_tok[0] == '#') {
        value = (int)strtoul(src_tok + 1, NULL, 10);
        physical_memory[phys] = value;
        fprintf(output_file, "Current PID: %d. Stored immediate %u into location %d\n", current_pid, value, vaddr);
    } else if (src_tok[0] == 'r') {
        int idx = get_reg_index(src_tok);
        value = (int)registers[idx];
        physical_memory[phys] = value;
        fprintf(output_file, "Current PID: %d. Stored value of register %s (%u) into location %d\n", current_pid, src_tok, value, vaddr);
    } else {
        fprintf(output_file, "Current PID: %d. Error: invalid source operand %s\n", current_pid, src_tok);
        exit(1);
    }
}

void handle_add(FILE* output_file) {
    int r1_value = registers[1];
    int r2_value = registers[2];
    int result = r1_value + r2_value;

    registers[1] = result;

    fprintf(output_file, "Current PID: %d. Added contents of registers r1 (%d) and r2 (%d). Result: %d\n", current_pid, r1_value, r2_value, result);
}

void process_instruction(char** tokens, FILE* output_file) {
    //Cover empty lines or commented lines
    if (tokens[0] == NULL) return;
    if (tokens[0][0] == '%') return;

    time_counter++;

    //Cover no define
    if (!define_called && strcmp(tokens[0], "define") != 0) {
        fprintf(output_file, "Current PID: %d. Error: attempt to execute instruction before define\n", current_pid);
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
        int pfn = atoi(tokens[2]);
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
        int rn = atoi(tokens[1]+1);
        handle_rinspect(rn, output_file);
    }
    else if (strcmp(tokens[0], "load") == 0) {
        handle_load(tokens[1], tokens[2], output_file);
    }
    else if (strcmp(tokens[0], "store") == 0) {
        handle_store(tokens[1], tokens[2], output_file);
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