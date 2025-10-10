#include "parser.h"
#include <ctype.h>

//Function to trim whitespace and ASCII control characters from buffer
//[Input] char* inputbuffer - input string to trim
//[Input] size_t bufferlen - size of input and output string buffers
//[Output] char* outputbuffer - output string after trimming 
//[Return] size_t - size of output string after trimming
size_t trimstring(char* outputbuffer, const char* inputbuffer, size_t bufferlen) {
    if (!outputbuffer || !inputbuffer || bufferlen == 0) return 0;

    size_t inlen = strlen(inputbuffer);
    size_t copylen = (inlen < bufferlen - 1) ? inlen : bufferlen - 1;
    memcpy(outputbuffer, inputbuffer, copylen);
    outputbuffer[copylen] = '\0';

    //remove trailing control characters (ASCII < '!')
    ssize_t i = (ssize_t)copylen - 1;
    while (i >= 0) {
        if ((unsigned char)outputbuffer[i] < (unsigned char)'!') {
            outputbuffer[i] = '\0';
            i--;
        } else {
            break;
        }
    }
    return strlen(outputbuffer);
}

//Function to trim the input command to just be the first word
//[Input] char* inputbuffer - input string to trim
//[Input] size_t bufferlen - size of input and output string buffers
//[Output] char* outputbuffer - output string after trimming 
//[Return] size_t - size of output string after trimming
size_t firstword(char* outputbuffer, const char* inputbuffer, size_t bufferlen) {
    if (!outputbuffer || !inputbuffer || bufferlen == 0) return 0;
    const char *p = inputbuffer;
    while (*p && isspace((unsigned char)*p)) p++;
    size_t i = 0;
    while (*p && !isspace((unsigned char)*p) && i + 1 < bufferlen) {
        outputbuffer[i++] = *p++;
    }
    outputbuffer[i] = '\0';
    return i;
}

//Function to test that string only contains valid ascii characters (non-control and not extended)
//[Input] char* inputbuffer - input string to test
//[Input] size_t bufferlen - size of input buffer
//[Return] bool - true if no invalid ASCII characters present
bool isvalidascii(const char* inputbuffer, size_t bufferlen) {
    if (!inputbuffer || bufferlen == 0) return false;
    size_t testlen = strlen(inputbuffer);
    if (testlen > bufferlen) testlen = bufferlen;
    for (size_t i = 0; i < testlen; ++i) {
        unsigned char c = (unsigned char) inputbuffer[i];
        //Accept printable ascii plus space (0x20 - 0x7E)
        if (c < 0x20 || c > 0x7E) return false;
    }
    return true;
}

//Function to find location of pipe character in input string
//[Input] char* inputbuffer - input string to test
//[Input] size_t bufferlen - size of input buffer
//[Return] int - location in the string of the pipe character, or -1 pipe character not found
int findpipe(const char* inputbuffer, size_t bufferlen){
    //TO DO: Implement this function

    return -1;
}

int tokenize(const char *input, char **argv_out, int max_args)
{
    if (!input || !argv_out || max_args <= 0) return -1;

    const char *p = input;
    int count = 0;

    while (*p && isspace((unsigned char)*p)) ++p;

    while (*p != '\0') {
        if (count >= max_args) {
            return -1;
        }

        char tokenbuf[1024];
        size_t tb = 0;

        if (*p == '"' || *p == '\'') {
            char quote = *p++;
            while (*p != '\0' && *p != quote) {
                if (*p == '\\' && p[1] != '\0') {
                    tokenbuf[tb++] = p[1];
                    p += 2;
                } else {
                    tokenbuf[tb++] = *p++;
                }
                if (tb >= sizeof(tokenbuf) - 1) break;
            }
            if (*p == quote) ++p;
            else {
                return -1;
            }
        } else {
            while (*p != '\0' && !isspace((unsigned char)*p)) {
                if (*p == '\\' && p[1] != '\0') {
                    tokenbuf[tb++] = p[1];
                    p += 2;
                } else {
                    tokenbuf[tb++] = *p++;
                }
                if (tb >= sizeof(tokenbuf) - 1) break;
            }
        }

        tokenbuf[tb] = '\0';
        argv_out[count] = strdup(tokenbuf);
        if (!argv_out[count]) {
            for (int i = 0; i < count; ++i) free(argv_out[i]);
            return -1;
        }
        count++;

        while (*p && isspace((unsigned char)*p)) ++p;
    }

    argv_out[count] = NULL;
    return count;
}