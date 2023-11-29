#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "job.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

msh_t * alloc_shell(int max_jobs, int max_line, int max_history) {
    const int MAX_LINE = 1024;
    const int MAX_JOBS = 16;
    const int MAX_HISTORY = 10;
    msh_t * state;
    state = malloc(sizeof(msh_t));
    if (max_jobs == 0) {
        state -> max_jobs = MAX_JOBS;
    } else {
        state -> max_jobs = max_jobs;
    }
    if (max_line == 0) {
        state -> max_line = MAX_LINE;
    } else {
        state -> max_line = max_line;
    }
    if (max_history == 0) {
        state -> max_history = MAX_HISTORY;
    } else {
        state -> max_history = max_history;
    }
    state -> jobs = malloc(sizeof(job_t)*((state->max_jobs)+1));
    state->jobs[0].cmd_line = NULL;
    return state;
}

char * parse_tok(char * line, int * job_type) {
    static char * cursor;
    * job_type = -1;
    if (line == NULL) {
        line = cursor;
    }
    if (line[0] == '\0') {
        return NULL;
    } else if ((line[0] == ';' || line[0] == '&') && (line[1] != '\0')) {
        line = &line[1];
    }
    char * command = malloc(sizeof(char)*(strlen(line)+1));
    int command_flag = 0;
    for (int i = 0; i <= strlen(line); i++) {
        if (line[i] == '\0' || line[i] == ';' || line[i] == '&') {
            if (command_flag == 1) {
                command[i] = '\0';
                if (line[i] == '&') {
                    * job_type = 0;
                } else {
                    * job_type = 1;
                }
            cursor = &line[i];
            }
            break;
        } else {
            command[i] = line[i];
            if (command_flag == 0 && line[i] != ' ') {
                command_flag = 1;
            }
        }
    }
    if (command_flag == 0) {
        return NULL;
    }
    return command;
}

char **separate_args(char *line, int *argc, bool *is_builtin) {
    char ** argv = malloc(sizeof(char **)*50);
    * argc = 0;
    int arg_index = 0, command_flag = 0, substr_index = 0;
    for (int i = 0; i <= strlen(line); i++) {
        if (command_flag == 0) {
            if (line[i] == '\0') {
                break;
            } else if (line[i] == ' ') {
                continue;
            } else {
                command_flag = 1;
                argv[arg_index] = malloc(sizeof(char *)*50);
                argv[arg_index][substr_index] = line[i];
                substr_index++;
            }
        } else {
            if (line[i] == '\0' || line[i] == ' ') {
                command_flag = 0;
                (* argc)++;
                argv[arg_index][substr_index] = '\0';
                substr_index = 0;
                arg_index++;
            } else {
                argv[arg_index][substr_index] = line[i];
                substr_index++;
            }
        }
    }
    argv[arg_index] = NULL;
    arg_index++;
    for (arg_index; arg_index < 50; arg_index++) {
        free(argv[arg_index]);
    }
    if (* argc == 0) {
        return NULL;
    }
    return argv;
}

int evaluate(msh_t *shell, char *line) {
    if (strlen(line) > shell->max_line) {
        printf("error: reached the maximum line limit\n");
        return 1;
    }
    int max_jobs = shell -> max_jobs;
    char * job;
    int type;
    while (true) {
        job = parse_tok(line, &type);
        if (job == NULL) {
            break;
        }
        int argc;
        bool built_in = false;
        char ** argv = separate_args(job, &argc, &built_in);
        if (strcmp(argv[0], "exit") == 0) {
            return 2;
        } else {
            char * cmd_line = job;
            int child_status = -1;
            job_state_t state;
            if (type == 1) {
                state = FOREGROUND;
            } else if (type == 0) {
                state = BACKGROUND;
            }
            bool max_job_reach = true;
            for (int i = 0; i < max_jobs; i++) {
                if (shell->jobs[i].cmd_line == NULL) {
                    max_job_reach = false;
                    break;
                }
            }
            if (max_job_reach) {
                printf("error: reached the maximum jobs limit\n");
            } else {
                pid_t pid = fork();
                if (pid == 0) {
                    execve(argv[0], argv, NULL);
                } else {
                    bool job_added = add_job(shell->jobs, max_jobs, pid, state, cmd_line);
                    if (job_added) {
                        if (state == FOREGROUND) {
                            pid_t wpid = waitpid(pid, &child_status, 0);
                            if (WIFEXITED(child_status)) {
                                delete_job(shell->jobs, wpid);
                            }
                        }
                    }
                }
            }
        }
        line = NULL;
        free(argv);
    }
    int to_delete[max_jobs];
    int to_del_index = 0;
    for (int i = 0; i < max_jobs; i++) {
        if (shell->jobs[i].state == BACKGROUND) {
            int child_status = 0;
            pid_t term_pid = waitpid(shell->jobs[i].pid, &child_status, WNOHANG);
            if (term_pid == shell->jobs[i].pid) {
                if (WIFEXITED(child_status)) {
                    to_delete[to_del_index] = shell->jobs[i].pid;
                    to_del_index++;
                }
            }
        }
    }
    for (int i = 0; i < to_del_index; i++) {
        delete_job(shell->jobs, to_delete[i]);
    }
    return 0;
}

void exit_shell(msh_t *shell) {
    int max_jobs = shell->max_jobs;
    int to_delete[max_jobs];
    int to_del_index = 0;
    for (int i = 0; i < max_jobs; i++) {
        if (shell->jobs[i].state == BACKGROUND) {
            int child_status = 0;
            pid_t wpid = waitpid(shell->jobs[i].pid, &child_status, 0);
            if (wpid == shell->jobs[i].pid) {
                if (WIFEXITED(child_status)) {
                    to_delete[to_del_index] = shell->jobs[i].pid;
                    to_del_index++;
                }
            }
        }
    }
    for (int i = 0; i < to_del_index; i++) {
        delete_job(shell->jobs, to_delete[i]);
    }
    free_jobs(shell->jobs, shell->max_jobs);
    free(shell);
}