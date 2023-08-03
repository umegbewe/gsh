#include <stdio.h>
#include <stdlib.h>
#include "shell.h"

extern Job *job_list;

Job *add_job(pid_t pgid, char *command) {
    Job *new_job = malloc(sizeof(Job));
    if (!new_job) {
        perror("Error allocating memory for new job");
        exit(EXIT_FAILURE);
    }

    new_job->pgid = pgid;
    new_job->command = command;
    
    if (!new_job->command) {
        perror("Error duplicating command string");
        free(new_job);
        exit(EXIT_FAILURE);
    }

    new_job->status = 0;


    if (job_list == NULL) {
        new_job->jobid = 1;
        new_job->next = NULL;
        job_list = new_job;
    } else {
        Job *current = job_list;
        while (current->next != NULL) {
            current = current->next;
        }
        new_job->jobid = current->jobid + 1;
        new_job->next = NULL;
        current->next = new_job;
    }

    return new_job;
}

void remove_job(Job *job) {
    if (job_list == NULL) {
        return;
    }

    if (job_list == job) {
        job_list = job_list->next;
    } else {
        Job *current = job_list;
        while (current != NULL && current->next != job) {
            current = current->next;
        }
        if (current != NULL) {
            current->next = job->next;
        }
    }
    free(job);
}

Job *find_job(pid_t pgid) {
    Job *job = job_list;
    while (job != NULL && job->pgid != pgid) {
        job = job->next;
    }
    return job;
}

void print_jobs() {
    Job *job = job_list;
    while (job != NULL) {
        printf("[%d] %s (%s)\n", job->jobid, job->command,
               job->status == 0 ? "running" :
               job->status == 1 ? "stopped" : "done");
        job = job->next;
    }
}
