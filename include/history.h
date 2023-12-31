#ifndef _HISTORY_H_
#define _HISTORY_H_

extern const char *HISTORY_FILE_PATH;

//Represents the state of the history of the shell
typedef struct history {
    char **lines;
    int max_history;
    int next;
} history_t;

/*
* alloc_history: Allocating memory for a new history_t value and Open the
* HISTORY_FILE_PATH file and load all prior history into char **lines 
* up until max_history is reached. 
*
* max_history: the maximum history line the history state can hold
*
* Returns: the allocated history_t state
*/
history_t *alloc_history(int max_history);

/*
* add_line_history: adds the parameter cmd_line to the history of lines
* If cmd_line is empty or `exit` then do nothing
* If the history of lines is full, the function deletes the oldest command
* line history, then shift all elements over by one and then insert the
* cmd_line at the last element in char **lines.
*
* history: the history state to add the cmd_line
*
* cmd_line: the command to be added to the history state
*
* Returns: nothing
*/
void add_line_history(history_t *history, const char *cmd_line);

/*
* print_history: printing all history lines
*
* history: the history state to add the cmd_line
*
* Returns: nothing
*/
void print_history(history_t *history);

/*
* find_line_history: find the cmd line in the history at index
*
* history: the history state to find the cmd_line
*
* index: the index in the history
*
* Returns: NULL if index is out of bound in history, else the cmd_line at index
*/
char *find_line_history(history_t *history, int index);

/*
* find_line_history: save all history to the HISTORY_FILE_PATH file, then
* frees the history_t and all allocated memory associated with the state
*
* history: the history state to find the cmd_line
*
* Returns: nothing
*/
void free_history(history_t *history);

#endif