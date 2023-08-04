## gsh

gsh is a custom Unix shell implemented in C. It supports a variety of functionalities that you would expect from a Unix shell, including executing commands, managing background jobs, and command history.

### Features

* **Command Execution**: The shell can execute most Unix commands, and any command not recognized as a built-in command will be executed as an external program.

* **Background Jobs**: Jobs can be run in the background by appending `&` to the command line.

* **Job Management**: Use the `jobs` command to print all current jobs running in the background.

* **Command History**: The shell maintains a history of executed commands, and you can navigate through previous commands using the up and down arrow keys.

### Building and Running

To build the shell, use the provided Makefile by running `make`. This will compile the source files and produce an executable named `gsh`.

You can then run the shell by executing `./gsh`.

```bash
$ make
$ ./gsh
```

#### Future Work

* Fix the issue with the `jobs` command not displaying background jobs.
* Improve error handling and robustness.
* Command autocompletion.
* Aliases
* Shell customization
* Wildcard matching
* Environment variables
* Piping and redirection
* Script execution

#### Known Issues

1. The `jobs` command is not currently printing the list of background jobs. This issue is being investigated.
---

