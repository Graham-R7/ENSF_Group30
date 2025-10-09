#include "util.h"
#include <ctype.h>

int* read_next_line(FILE* fin) {
    // TODO: This function reads the next line from the input file
    // The line is a comma-separated list of integers
    // Return the list of integers as an array where the first element
    // is the number of integers in the rest of the array
    // Return NULL if there are no more lines to read
    if (fin == NULL){
        return NULL;
    }
    char buff[2048]; //A buffer which holds one line
    if (fgets(buff, sizeof buff, fin) == NULL){
        return NULL;
    }
    //Cleans buffer line
    size_t length = strlen (buff);
    while (length > 0 && (buff[length-1] == '\n' || buff[length-1] == '\r')){
        buff[--length] = '\0';
    }

    //Counts how many items are in one line to allocate space properly
    size_t count = 0;
    char *p = buff;
    while (*p){
        while (*p && isspace((unsigned char)*p)){
            p++;
        }
        if (!*p){
            break;
        }
        if (*p == ','){
            p++; continue;
        }

        count++;
        while (*p && *p != ',') p++;
        if (*p == ',') p++;
    }

    //Allocate space according to count
    int *result = malloc((count + 1) * sizeof *result);
    if (result == NULL){
        return NULL;
    }
    result[0] = (int)count;
    if (count == 0){
        return result;
    }

    //Find each number and copy it into the allocated space
    size_t index = 1;
    p = buff;
    while (*p && index <= count) {
        while (*p && (isspace((unsigned char)*p) || *p == ',')){
            p++;
        }
        if (!*p) break;
        char *start = p;
        while (*p && *p != ','){
            p++;
        }
        char saved = *p;
        *p = '\0';

        char *end = start + strlen(start);
        while (end > start && isspace((unsigned char)*(end - 1))){
            *(--end) = '\0';
        }

        if (*start != '\0') {
            char *endptr = NULL;
            long val = strtol(start, &endptr, 10);
            if (endptr == start) {
                result[index++] = 0;
            } 
            else {
                result[index++] = (int)val;
            }
        }

        *p = saved;
        if (*p == ',') p++;
    }
    return result;
}


float compute_average(int* line) {
    // TODO: Compute the average of the integers in the vector
    // Recall that the first element of the vector is the number of integers
    if(line == NULL){
        return 0.0f;
    }
    int n = line[0];
    if (n <= 0){
        return 0.0f;
    }

    double sum = 0;
    for (int i = 1; i <= n; ++i) {
        sum += (double)line[i];
    }

    double avg = (double)sum / (double)n;
    return (double)avg;
}


float compute_stdev(int* line) {
    // TODO: Compute the standard deviation of the integers in the vector
    // Recall that the first element of the vector is the number of integers
    if (line == NULL){
        return 0.0f;
    }
    int n = line[0];
    if (n <= 1){
        return 0.0f;
    }

    double mean = (double)compute_average(line);
    double sumsquare = 0.0;
    for (int i = 1; i <= n; ++i) {
        double differ = (double)line[i] - mean;
        sumsquare += differ * differ;
    }

    double variance = sumsquare / (double)n;
    double stdev = sqrt(variance);
    return (double)stdev;
}