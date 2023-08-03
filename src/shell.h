#include <sys/types.h>

extern int num_bg_jobs;

typedef struct Job {
    pid_t pgid; // Process group ID
    int jobid; // Job ID
    char *command; // Command
    int status; // Status: 0 = running, 1 = stopped, 2 = done
    struct Job *next;
}Job;

Job *job_list;

void shell_loop(void);
char *shell_read_line(void);
char **shell_split_line(char *line);
int shell_execute(char **args);
int shell_launch(char **args, int bg);
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);
int shell_jobs(char **args);
int shell_bg(char **args);
int shell_fg(char **args);
int shell_kill(char **args);
int shell_stop(char **args);
int shell_history(char **args);
int shell_num_builtins();

Job *add_job(pid_t pgid, char *command);
void remove_job(Job *job);
Job *find_job(pid_t pgid);
void print_jobs();
