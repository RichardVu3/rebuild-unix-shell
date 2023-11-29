#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE
#include <unistd.h> 


int main(int argc, char *argv[]) {
    // Initialize the shell state
    int opt;
    int s = 0, j = 0, l = 0, error_flag = 0, v = 0;
    char * content = NULL;
    while((opt = getopt(argc, argv, ":s:j:l:")) != -1) {  
        switch(opt) {
            case 's':
                if (sscanf(optarg, "%d", &v) != 1 || v <= 0) {
                    error_flag = 1;
                }
                s = v;
                break;
            case 'j':
                if (sscanf(optarg, "%d", &v) != 1 || v <= 0) {
                    error_flag = 1;
                }
                j = v;
                break;
            case 'l':
                if (sscanf(optarg, "%d", &v) != 1 || v <= 0) {
                    error_flag = 1;
                }
                l = v;
                break;
            case ':':
                error_flag = 1; // option needs a value
                break;
            case '?':
                error_flag = 1; // optopt
                break;
        }
    }
    for (; optind < argc; optind++) {      
        error_flag = 1;
    }
    if (error_flag == 1) {
        printf("usage: msh [-s NUMBER] [-j NUMBER] [-l NUMBER]\n");
        return 1;
    }
    msh_t * shell = alloc_shell(j, l, s);

    // The Shell MSH to interact with
    printf("msh> ");
    char *line = NULL, line_copy[200];
    long int len = 0;
    long nRead = getline(&line, &len, stdin);
    while (nRead != -1) {
        if (line[strlen(line)-1] == '\n') {
            strncpy(line_copy, line, strlen(line)-1);
            line_copy[strlen(line)-1] = '\0';
        } else {
            strncpy(line_copy, line, strlen(line));
            line_copy[strlen(line)] = '\0';
        }
        if (strcmp(line_copy, "exit") == 0) {
            break;
        } else {
            int eval = evaluate(shell, line_copy);
            if (eval == 2) {
                break;
            }
            free(line);
            line = NULL;
            char line_copy[200];
            printf("msh> ");
            nRead = getline(&line, &len, stdin);
        }
    }
    exit_shell(shell);
    return 0;
}