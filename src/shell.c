#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "signal_handlers.h"
#include "signintsafe.h"

msh_t * shell = NULL;
char * CONTINUE = "continue"; 

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
    state -> history = alloc_history(state->max_history);
    state -> fg_pid = 0;
    initialize_signal_handlers();
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
    char ** argv = malloc(sizeof(char *)*50);
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
                argv[arg_index] = malloc(sizeof(char)*50);
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
    int type = 0;
    add_line_history(shell->history, line);
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
            char * builtin = builtin_cmd(argv);
            if (builtin == NULL) {
                line = NULL;
                free(argv);
                continue;
            } else if (strcmp(builtin, "continue") == 0) {
                char * cmd_line = job;
                int child_status = -1;
                job_state_t state;
                if (type == 1) {
                    state = FOREGROUND;
                } else if (type == 0) {
                    state = BACKGROUND;
                }
                int num_jobs = 0;
                bool max_job_reach = true;
                for (int i = 0; i < max_jobs; i++) {
                    if (shell->jobs[i].cmd_line == NULL) {
                        max_job_reach = false;
                        num_jobs = i;
                        break;
                    }
                }
                if (max_job_reach) {
                    printf("error: reached the maximum jobs limit\n");
                } else {
                    sigset_t mask_all, prev_all, mask_one, prev_one;
                    sigfillset(&mask_all);
                    sigprocmask(SIG_BLOCK, &mask_all, &prev_all);
                    pid_t pid = fork();
                    if (pid == 0) {
                        setpgid(0, 0);
                        sigprocmask(SIG_SETMASK, &prev_all, NULL);
                        if (execve(argv[0], argv, NULL) < 0) {
                            printf("%s: Command not found.\n", argv[0]);
                            exit(0);
                        }
                    } else {
                        bool job_added = add_job(shell->jobs, max_jobs, pid, state, cmd_line, num_jobs);
                        if (state == FOREGROUND) {
                            shell->fg_pid = pid;
                        }
                        sigprocmask(SIG_SETMASK, &prev_all, NULL);
                        if (state == FOREGROUND) {
                            waitfg(pid);
                        }
                    }
                }
            } else {
                evaluate(shell, builtin);
            }
        }
        line = NULL;
        free(argv);
    }
    return 0;
}

void waitfg(pid_t pid) {
    sigset_t empty;
    sigemptyset(&empty);
    while (shell->fg_pid == pid) {
        sigsuspend(&empty);
    }
}

void exit_shell(msh_t *shell) {
    // Send SIGCONT to every Suspended jobs
    while (1) {
        for (int i = 0; i < shell->max_jobs; i++) {
            if (shell->jobs[i].state == SUSPENDED) {
                kill(-(shell->jobs[i].pid), SIGCONT);
                break;
            }
        }
        if (shell->jobs[0].cmd_line == NULL) {
            free_jobs(shell->jobs, shell->max_jobs);
            free_history(shell->history);
            free(shell);
            break;
        } else {
            sleep(1);
        }
    }
}

char * builtin_cmd(char **argv) {
    if (strcmp(argv[0], "jobs") == 0) {
        for (int i = 0; i < shell->max_jobs; i++) {
            if (shell->jobs[i].cmd_line == NULL) {
                break;
            } else {
                char * state_job = NULL;
                if (shell->jobs[i].state == SUSPENDED) {
                    state_job = "Stopped";
                } else if (shell->jobs[i].state == BACKGROUND) {
                    state_job = "RUNNING";
                }
                printf("[%d] %d %s %s\n", (shell->jobs[i].jid)+1, shell->jobs[i].pid, state_job, shell->jobs[i].cmd_line);
            }
        }
    } else if (strcmp(argv[0], "history") == 0) {
        print_history(shell->history);
    } else if (argv[0][0] == '!') {
        int history_line = atoi(&argv[0][1]);
        char * command = find_line_history(shell->history, history_line);
        if (command != NULL) {
            printf("%s\n", command);
            return command;
        }
    } else if (strncmp(argv[0], "fg", 2) == 0) {
        pid_t pid_cont = 0;
        if (argv[1][0] == '%') {
            for (int i = 0; i < shell->max_jobs; i++) {
                if (shell->jobs[i].jid == (atoi(&argv[1][1])-1)) {
                    pid_cont = shell->jobs[i].pid;
                    shell->jobs[i].state = FOREGROUND;
                    break;
                }
            }
        } else {
            pid_cont = atoi(argv[1]);
            for (int i = 0; i < shell->max_jobs; i++) {
                if (shell->jobs[i].pid == pid_cont) {
                    shell->jobs[i].state = FOREGROUND;
                    break;
                }
            }
        }
        shell->fg_pid = pid_cont;
        kill(-pid_cont, SIGCONT);
        waitfg(pid_cont);
    } else if (strncmp(argv[0], "bg", 2) == 0) {
        pid_t pid_cont = 0;
        if (argv[1][0] == '%') {
            for (int i = 0; i < shell->max_jobs; i++) {
                if (shell->jobs[i].jid == (atoi(&argv[1][1])-1)) {
                    pid_cont = shell->jobs[i].pid;
                    shell->jobs[i].state = BACKGROUND;
                    break;
                }
            }
        } else {
            pid_cont = atoi(argv[1]);
            for (int i = 0; i < shell->max_jobs; i++) {
                if (shell->jobs[i].pid == pid_cont) {
                    shell->jobs[i].state = BACKGROUND;
                    break;
                }
            }
        }
        kill(-pid_cont, SIGCONT);
    } else if (strcmp(argv[0], "kill") == 0) {
        int sign_number = atoi(argv[1]);
        if (sign_number == 2 || sign_number == 9 || sign_number == 18 || sign_number == 19) {
            for (int i = 0; i < shell->max_jobs; i++) {
                if (shell->jobs[i].pid == atoi(argv[2])) {
                    if (sign_number == 2) {
                        kill(-(shell->jobs[i].pid), SIGINT);
                    } else if (sign_number == 9) {
                        kill(-(shell->jobs[i].pid), SIGKILL);
                    } else if (sign_number == 18) {
                        kill(-(shell->jobs[i].pid), SIGCONT);
                    } else if (sign_number == 19) {
                        kill(-(shell->jobs[i].pid), SIGTSTP);
                    }
                    break;
                }
            }
        } else {
            printf("error: invalid signal number\n");
        }
    }
    else {
        return CONTINUE;
    }
    return NULL;
}