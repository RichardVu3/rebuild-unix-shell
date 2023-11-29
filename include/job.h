#ifndef _JOB_H_
#define _JOB_H_

#include <sys/types.h>
#include <stdbool.h>

typedef enum job_state{FOREGROUND, BACKGROUND, SUSPENDED, UNDEFINED} job_state_t;

// Represents a job in a shell.
typedef struct job {
    char * cmd_line;     // The command line for this specific job.
    job_state_t state;  // The current state for this job
    pid_t pid;          // The process id for this job
    int jid;            // The job number for this job. when initialized, jid is always -1
} job_t;

/*
* add_job: adds a new job to the array
*
* jobs: the array to add a new job into
*
* max_jobs: The maximum number of jobs that can be added.
*
* pid: the process ID of the job to add.
*
* state: the state of the job.
*
* cmd_line: the command line of the job.
*
* Returns: if there are no more jobs left to allocate (i.e., max_jobs has been reached) then return false; otherwise, true to indicate the job was added.
*/
bool add_job(job_t *jobs, int max_jobs, pid_t pid, job_state_t state, const char *cmd_line);

/*
* delete_job: remove a job from the array of jobs based on the pid_t provided
*
* jobs: the array to remove the pid
*
* pid: the pid defining the job to be removed
*
* Returns: true if there is a pid inside the jobs and successfully deletes it, false otherwise
*/
bool delete_job(job_t *jobs, pid_t pid);

/*
* free_jobs: deallocate the jobs array
*
* jobs: the array with placeholders to be freed
*
* max_jobs: The maximum number of placeholders of jobs
*
* Returns: nothing
*/
void free_jobs(job_t *jobs, int max_jobs);

#endif