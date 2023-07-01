#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

int num_bg_jobs = 0;

int main(int argc, char **argv) {

    if (argc > 1) {
        FILE *fp = fopen(argv[1], "r");
        if (fp == NULL) {
           printf("musk: could not open file\n");
           return 1;
        }

        char *line = NULL;
        size_t bufSize = 0;
        while (getline(&line, &bufSize, fp) != -1) {
            char **args = shell_split_line(line);
            shell_execute(args);

            free(line);
            free(args);
            line = NULL;
            args = NULL;
            bufSize = 0;
        }
        free(line);
        fclose(fp);
    } else {
        signal(SIGTTOU, SIG_IGN);
        shell_loop();
    }

    return 0;
}
