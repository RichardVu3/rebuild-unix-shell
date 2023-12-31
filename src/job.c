#include "job.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool add_job(job_t *jobs, int max_jobs, pid_t pid, job_state_t state, const char *cmd_line, int num_jobs) {
    if (num_jobs >= max_jobs) {
        return false;
    }
    job_t * job = malloc(sizeof(job_t));
    job -> cmd_line = malloc(sizeof(char)*(strlen(cmd_line)+1));
    strcpy(job -> cmd_line, cmd_line);
    job -> state = state;
    job -> pid = pid;
    job -> jid = num_jobs;
    jobs[num_jobs] = (*job);
    jobs[num_jobs+1].cmd_line = NULL;
    return true;
}

bool delete_job(job_t *jobs, pid_t pid) {
    int i = 0, fill_flag = 0;
    while (true) {
        if (jobs[i].cmd_line == NULL) {
            if (fill_flag == 0) {
                return false;
            }
            break;
        }
        if (jobs[i].pid == pid) {
            fill_flag = 1;
        }
        if (fill_flag == 1) {
            jobs[i] = jobs[i+1];
            (jobs[i].jid)--;
        }
        i++;
    }
    return true;
}

void free_jobs(job_t *jobs, int max_jobs) {
    for (int i = 0; i < max_jobs; i++) {
        free(jobs[i].cmd_line);
    }
    free(jobs);
}