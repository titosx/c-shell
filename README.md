# C Shell
This project is a unix shell and was implemented as a University exercise.

## Features
- Basic commands e.g. `ls`, `cat`, `man`, `pwd`
- `cd` command to change directory
- `exit` command to exit the shell
- Pipe implementation e.g. `[user]@cs345sh/[dir]$ ls -al | wc -l`
- I/O redirection or append e.g. `[user]@cs345sh/[dir]$ cat < [input]`, `[user]@cs345sh/[dir]$ ls -al > [output]`, `[user]@cs345sh/[dir]$ ls -al >> log.txt`
- Environment Variables (HOME and PATH): 
	- `setenv [VAR] [value]`, sets or changes environment variables,
	- `unsetenv [VAR] [value]` deletes environment variables, 
	- `env` shows environment variables

## Run
Use gcc cs345sh.c -o cs345sh to compile, and then ./cs345sh to run.