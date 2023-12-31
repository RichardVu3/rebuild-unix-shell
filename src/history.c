#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char * HISTORY_FILE_PATH = "../data/.msh_history";

history_t *alloc_history(int max_history) {
    history_t * history = malloc(sizeof(history_t));
    history -> lines = malloc(sizeof(char*)*max_history);
    for (int i = 0; i < max_history; i++) {
        (history->lines)[i] = NULL;
    }
    history -> max_history = max_history;
    history -> next = 0;
    FILE * fp; 
    int nBytesRead;
    fp = fopen(HISTORY_FILE_PATH, "r");
    if (fp != NULL) {
        char * line = NULL;
        long int len = 0;
        long nRead = getline(&line, &len, fp);
        while (nRead != -1) {
            (history->lines)[history->next] = malloc(sizeof(char)*(strlen(line)));
            if (line[strlen(line)-1] == '\n') {
                line[strlen(line)-1] = '\0';
            }
            strcpy((history->lines)[history->next], line);
            (history->next)++;
            free(line);
            line = NULL;
            if (history->next < max_history) {
                nRead = getline(&line, &len, fp);
            } else {
                break;
            }
        }
        fclose(fp);
    }
    return history;
}

void add_line_history(history_t *history, const char *cmd_line) {
    if (cmd_line != NULL && strcmp(cmd_line, "exit") != 0 && cmd_line[0] != '!') {
        if (history->next >= history->max_history) {
            for (int i = 0; i < (history->max_history)-1; i++) {
                (history->lines)[i] = malloc(sizeof(char)*strlen((history->lines)[i+1])+1);
                strcpy((history->lines)[i], (history->lines)[i+1]);
            }
            (history->next)--;
        }
        (history->lines)[history->next] = malloc(sizeof(char)*(strlen(cmd_line)+1));
        strcpy((history->lines)[history->next], cmd_line);
        (history->next)++;
    }
}

void print_history(history_t *history) {
    for (int i = 1; i <= history->next; i++) {
        printf("%5d\t%s\n", i, history->lines[i-1]);
    }
}

char *find_line_history(history_t *history, int index) {
    if (index < 1 || index > history->max_history) {
        return NULL;
    }
    return (history->lines)[index-1];
}

void free_history(history_t *history) {
    FILE * fp; 
    fp = fopen(HISTORY_FILE_PATH, "w");
    if (fp != NULL) {
        if (history->next > 0) {
            for (int i = 0; i < history->max_history; i++) {
                if ((history->lines)[i] != NULL) {
                    fprintf(fp, "%s\n", (history->lines)[i]);
                    free((history->lines)[i]);
                }
            }
        }
        fclose(fp);
    }
    free(history->lines);
    free(history);
}