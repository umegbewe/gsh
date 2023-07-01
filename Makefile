CC=gcc
CFLAGS=-I.
DEPS = shell.h
OBJ = src/main.o src/shell.o src/job_control.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

gsh: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -lreadline

.PHONY: clean

clean:
	rm -f $(OBJ) shell
