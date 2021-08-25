/*--------------------------------------*/
/* Titos Chaniotakis			*/
/* HY345 - Assignment			*/
/* cs345sh.c				*/
/*--------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

void print_prompt() {
	char *loginname = getlogin();
	if (loginname != NULL)
		printf("%s", getlogin());
	else
		printf("user");
	printf("@cs345sh");
	printf("%s", getcwd(NULL ,0));
	printf("/$ ");
}


char *read_input(int *input_redir, int *input_pipe) {
	int len = 16;
	int current_size = 0;
	char *input = malloc(len);
	current_size = len;
	char c;
	if (input != NULL) {
		int i = 0;
		int redir_app=0;
		while ((c = getchar()) != '\n' && c !=  EOF) {
			//check for redirection of output (append or not)
			if ((c == '>') & (redir_app == 0))
				redir_app = 1;
			//redirection of output (append)
			else if ((c == '>') & (redir_app == 1)) {
				(*input_redir) = 1;
				redir_app = 0;
			}
			//redirection of output
			else if ((c != '>') & (redir_app == 1)) {
				(*input_redir) = 1;
				redir_app = 0;
			}
			//redirection of input
			else if (c == '<') {
				(*input_redir) = 1;
			}
			//check for pipe
			else if (c == '|') {
				(*input_pipe) = 1;
			}
			input[i] = c;
			i++;
			if (i == current_size) {
				current_size += len;
				input = realloc(input, current_size);
			}
		}
		input[i] = '\0';
		return input;
	}
	else {
		printf("Memory allocation failed!\n");
	}
}

void execute_command(char **argv) {
	pid_t pid = fork();
	if (pid == 0) {
		//child process
		execvp(argv[0], argv);
	}
	else if (pid > 0) {
		//parent process
		int status;
		waitpid(-1, &status, 0);
	}
	else if (pid < 0) {
		//fork failed
		printf("Fork failed!\n");
	}
}



void command_pipe(char *input, int input_redir) {
	char **argv1 , **argv2;
	int argv1_size = 0;
	int argv2_size = 0;
	int j;
	char *token = strtok(input, " ");
	argv1_size++;
	argv1 = malloc(argv1_size * sizeof(char *));
	argv1[argv1_size-1] = malloc(strlen(token)*sizeof(char));
	for (j=0; j<strlen(token); j++)
		argv1[argv1_size-1][j] = token[j];
	token = strtok(NULL, " ");
	while (strcmp(token, "|") != 0) {
		argv1_size++;
		argv1 = realloc(argv1, argv1_size * sizeof(char *));
		argv1[argv1_size-1] = malloc(strlen(token)*sizeof(char));
		for (j=0; j<strlen(token); j++)
			argv1[argv1_size-1][j] = token[j];
		token = strtok(NULL, " ");
	}
	argv1[argv1_size] = 0; // now token contains "|"
	token = strtok(NULL, " "); // now token contains next of "|"
	argv2_size++;
	argv2 = malloc(argv2_size * sizeof(char *));
	argv2[argv2_size-1] = malloc(strlen(token)*sizeof(char));
	for (j=0; j<strlen(token); j++)
		argv2[argv2_size-1][j] = token[j];
	token = strtok(NULL, " ");
	while (token != NULL) {
		argv2_size++;
		argv2 = realloc(argv2, argv2_size * sizeof(char *));
		argv2[argv2_size-1] = malloc(strlen(token)*sizeof(char));
		for (j=0; j<strlen(token); j++)
			argv2[argv2_size-1][j] = token[j];
		token = strtok(NULL, " ");
	}
	argv2[argv2_size] = 0;

	int saved_stdout = -1;
	int saved_stdin = -1;
	saved_stdout = dup(1);
	int fd[2];
	pipe(fd);
	pid_t pid = fork();
	if (pid == -1)
		return;
	if (pid == 0) {
		close(fd[0]); //close read from pipe, in parent
		dup2(fd[1], STDOUT_FILENO); // Replace stdout with the write end of the pipe
		close(fd[1]); // Don't need another copy of the pipe write end hanging about
		execvp(argv1[0], argv1);
		}
	else {
		close(fd[1]); //close write to pipe, in child
		dup2(fd[0], STDIN_FILENO); // Replace stdin with the read end of the pipe
		close(fd[0]); // Don't need another copy of the pipe read end hanging about
		execvp(argv2[0], argv2);
	}
}

void command_cases(char *input, int input_redir) {
	char **argv;
	char **redir_args = NULL;
	int redir_size=0;
	int current_size;
	//if command is "change directory"
	if ((input[0] == 'c') && (input[1] =='d')) {
		char *dir = malloc((strlen(input)-2)*sizeof(char));
		int i;
		if (input[2] == ' ') {
			int j=0;
			for (i=3; i<(strlen(input)); i++) {
				dir[j] = input[i];
				j++;
			}
			dir[j] = '\0';
			chdir(dir);
		}
	}
	//if command has no redirection or append
	else if (input_redir == 0) {
		current_size = 1;
		int j;
		char *token = strtok(input, " ");
		current_size++;
		argv = malloc(current_size * sizeof(char *));
		argv[current_size-2] = malloc(strlen(token)*sizeof(char));
		for (j=0; j<strlen(token); j++)
			argv[current_size-2][j] = token[j];
		while (token != NULL) {
			token = strtok(NULL, " ");
			if (token!=NULL) {
				current_size++;
				argv = realloc(argv, current_size * sizeof(char *));
				argv[current_size-2] = malloc(strlen(token)*sizeof(char));
				for (j=0; j<strlen(token); j++) {
					argv[current_size-2][j] = token[j];
				}
			}
		}
		argv[current_size-1] = 0;
		execute_command(argv);
	}
	//if command has (at least one) redirection or append
	else if ((input_redir != 0)) {
		int j;
		current_size = 1;
		int redir_found = 0;
		char *token = strtok(input, " ");
		current_size++;
		argv = malloc(current_size * sizeof(char *));
		argv[current_size-2] = malloc(strlen(token)*sizeof(char));
		for (j=0; j<strlen(token); j++)
			argv[current_size-2][j] = token[j];
		int i=2;
		token = strtok(NULL, " ");
		while (strcmp(token, ">")!=0 && strcmp(token, ">>")!=0 && strcmp(token, "<")!=0) {
			current_size++;
			argv = realloc(argv, current_size*sizeof(char));
			argv[current_size-2] = malloc(3*sizeof(char));
			for (j=0; j<strlen(token); j++)
				argv[current_size-2][j] = token[j];
			token = strtok(NULL, " ");
		}
		argv[current_size-1] = 0;
		while (((token!=NULL) && (strcmp(token, ">")==0 || strcmp(token, ">>")==0 || strcmp(token, "<")==0 || redir_found==1))) {
			redir_found=1;
			redir_size++;
			redir_args = realloc(redir_args, redir_size*sizeof(char *));
			redir_args[redir_size-1] = malloc(strlen(token)*sizeof(char)+1);
			for (j=0; j<strlen(token); j++)
				redir_args[redir_size-1][j] = token[j];
			redir_args[redir_size-1][j] = '\0';
			token = strtok(NULL, " ");
		}
		if (redir_size > 1) {
			int saved_stdout = -1;
			int saved_stdin = -1;
			for (i=0; i<redir_size; i++) {
				if (strcmp(redir_args[i], ">") == 0) {
					int out = open(redir_args[i+1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
					saved_stdout = dup(1);
					dup2(out, 1);
					close(out);
				}
				else if (strcmp(redir_args[i], ">>") == 0) {
					int out = open(redir_args[i+1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
					saved_stdout = dup(1);
					dup2(out, 1);
					close(out);
				}
				else if (strcmp(redir_args[i], "<") == 0) {
					int in = open(redir_args[i+1], O_RDONLY);
					saved_stdin = dup(0);
					dup2(in, 0);
					close(in);
				}
			}
			execute_command(argv);
			if (saved_stdout != -1) {
				dup2(saved_stdout, 1);
				close(saved_stdout);
			}
			if (saved_stdin != -1) {
				dup2(saved_stdin, 0);
				close(saved_stdin);
			}
		}
	}
}

void env_var(char *input) {
	char *token = strtok(input, " ");
	if (strcmp(input, "env") == 0) {
		printf("HOME=%s\n", getenv("HOMEVAR"));
   		printf("PATH=%s\n", getenv("PATHVAR"));
	}
	else if (strcmp(token, "setenv") == 0) {
		token = strtok(NULL, " ");
		if (strcmp(token, "HOME") == 0) {
			token = strtok(NULL, " ");
			setenv("HOMEVAR", token, 1);
		}
		else if (strcmp(token, "PATH") == 0) {
			token = strtok(NULL, " ");
			setenv("PATHVAR", token, 1);
		}
	}
	else if (strcmp(token, "unsetenv") == 0) {
		token = strtok(NULL, " ");
		if (strcmp(token, "HOME") == 0) {
			unsetenv("HOMEVAR");
		}
		else if (strcmp(token, "PATH") == 0) {
			unsetenv("PATHVAR");
		}
	}
}

void exit_shell() {
	system("clear");
	exit(0);
}

int main() {
	system("clear");
	int size = 0;
	setenv("HOMEVAR", getenv("HOME"), 1);
	setenv("PATHVAR", getenv("PATH"), 1);
	while (1) {
		print_prompt();
		int input_redir=0;
		int input_pipe=0;
		char *input = read_input(&input_redir, &input_pipe);
		if (strcmp(input, "exit") == 0)
			exit_shell();
		else if (((input[0] == 'e') && (input[1] == 'n') && (input[2] == 'v')) ||
					((input[0] == 's') && (input[1] == 'e') && (input[2] == 't') && (input[3] == 'e') && (input[4] == 'n') && (input[5] == 'v')) ||
					((input[0] == 'u') && (input[1] == 'n') && (input[2] == 's') && (input[3] == 'e') && (input[4] == 't') && (input[5] == 'e') && (input[6] == 'n') && (input[7] == 'v'))) 
			env_var(input);
		else if (input_pipe == 1)
			command_pipe(input, input_redir);
		else
			command_cases(input, input_redir);
	}
}
