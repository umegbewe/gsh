#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <errno.h>
#include "shell.h"

#define SHELL_TOK_BUFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"

#define EXIT_FAILURE 1

char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "fg",
    "bg",
    "kill",
    "stop",
};

int (*builtin_func[]) (char **) = {
    &shell_cd,
    &shell_help,
    &shell_exit,
    &shell_jobs,
    &shell_fg,
    &shell_bg,
    &shell_kill,
    &shell_stop,
};


void shell_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        line = shell_read_line();
        args = shell_split_line(line);
        status = shell_execute(args);

        free(line);
        free(args);
    } while(status);
}

char *shell_read_line(void) {
    rl_on_new_line();

    char *line = readline("musk> ");
    if (strlen(line) > 0) {
        add_history(line);
    }
    return line;
}

char **shell_split_line(char *line) {
    int bufsize = SHELL_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "myshell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, SHELL_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += SHELL_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "myshell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, SHELL_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}


int shell_execute(char **args) {

    int i;

    if(args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < shell_num_builtins(); i++) {
        if(strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    // check if command ends with & for background execution
    int bg = 0; // background flag
    for (i = 0; args[i] != NULL; i++) {}
    if (i > 0 && strcmp(args[i-1], "&") == 0) {
        bg = 1; // set background flag
        args[i-1] = NULL; // remove & from args
    }

    return shell_launch(args, bg);
}

int shell_launch(char **args, int bg) {
    pid_t pid, pgid;
    int status;

    pid = fork();
    if (pid == 0) {
        // child process
        // set process group ID to child's PID
        setpgid(0, 0);

        // If process is a foreground one, give it control of the terminal
        if (!bg) {
            tcsetpgrp(STDIN_FILENO, getpid());
        }

        if (execvp(args[0], args) == -1) {
            perror("gsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("gsh");
    } else {
        // parent process (shell)
        // set process group ID for the job
        setpgid(pid, pid);
        pgid = getpgid(pid);

        // Always create the job, whether it's background or foreground
        Job *job = add_job(pgid, args[0]);

        if (bg) {
            // if job is a background one, print its info
            printf("[%d] %d\n", ++num_bg_jobs, pid);
            // Don't remove the job - let it finish in the background
        } else {
            // If process is a foreground one, wait for it to finish ang give it control of the terminal
            tcsetpgrp(STDIN_FILENO, pid);
            do {
                waitpid(pid, &status, WUNTRACED);
                tcsetpgrp(STDIN_FILENO, getpgrp());
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            tcsetpgrp(STDIN_FILENO, getpgrp());

            // Update job status and remove it only after it's done
            job->status = 2;
            remove_job(job);
        }
    }

    return 1;
}


int shell_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "shell: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("gsh");
        }
    }
    return 1;
}

int shell_help(char **args) {
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (int i = 0; i < shell_num_builtins(); i++) {
        printf(" %s\n", builtin_str[i]);
    }

    return 1;
}

int shell_exit(char **args) {
    return 0;
}

int shell_jobs(char **args) {
    print_jobs();
    return 1;
}

int shell_fg(char **args) {
    Job *job;
    int status;
    pid_t pid, pgid;

    if (args[1]) {
        pgid = atoi(args[1]);
        job = find_job(pgid);
    } else {
        job = job_list;
    }

    if (job == NULL) {
        printf("shell: fg: job not found\n");
        return 1;
    }

    // give terminal control of job
    tcsetpgrp(STDIN_FILENO, job->pgid);

    // if job is stopped, send SIGCONT
    if (job->status == 1) {
        kill(-job->pgid, SIGCONT);
    }

    // wait for job to finish
    do {
        waitpid(job->pgid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));

    // Give terminal control back to shell
    tcsetpgrp(STDIN_FILENO, getpgrp());

    job->status = 2; //done
    remove_job(job);

    return 1;
}

int shell_bg(char **args) {
    Job *job;
    pid_t pgid;

    if (args[1]) {
        pgid = atoi(args[1]);
        job = find_job(pgid);
    } else {
        job = job_list;
    }

    if (job == NULL) {
        printf("shell: kill: job not found\n");
        return 1;
    }

    // if job is stopped, send SIGCONT
    if (job->status == 1) {
        kill(-job->pgid, SIGCONT);
    }

    job->status = 0; // running

    return 1;
}

int shell_kill(char **args) {
    Job *job;
    pid_t pgid;

    if (args[1]) {
        pgid = atoi(args[1]);
        job = find_job(pgid);
    } else {
        job = job_list;
    }

    if (job == NULL) {
        printf("shell: kill: job not found\n");
        return 1;
    }

    kill(-job->pgid, SIGTERM);

    job->status = 2;
    remove_job(job);

    return 1;
}

int shell_stop(char **args) {
    Job *job;
    pid_t pgid;

    if (args[1]) {
        pgid = atoi(args[1]);
        job = find_job(pgid);
    } else {
        job = job_list;
    }

    if (job == NULL) {
        printf("shell: stop: job not found\n");
        return 1;
    }

    kill(-job->pgid, SIGSTOP);

    job->status = 1;

    return 1;
}
int shell_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}





