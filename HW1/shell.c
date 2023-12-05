#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "io.h"
#include "parse.h"
#include "process.h"
#include "shell.h"
#include <stdbool.h>

#define INPUT_STRING_SIZE 80
#define MAXIMUMTOKS 100
#define REDIRECT_IN "<"
#define REDIRECT_OUT ">"
#define BACKGROUND "&"


int cmd_quit(tok_t arg[]) {
  printf("Bye\n");
  exit(0);
  return 1;
}

int cmd_help(tok_t arg[]);
int cmd_pwd(tok_t arg[]);
int cmd_cd(tok_t arg[]);
int cmd_wait(tok_t arg[]);

/* Command Lookup table */
typedef int cmd_fun_t (tok_t args[]); /* cmd functions take token array and return int */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_quit, "quit", "quit the command shell"},
  {cmd_pwd, "pwd", "show the current working directory"},
  {cmd_cd, "cd", "change the current working directory"},
  {cmd_wait, "wait", "wait for child processes to finish"}
};

int cmd_help(tok_t arg[]) {
  int i;
  for (i = 0; i < (sizeof(cmd_table) / sizeof(fun_desc_t)); i++) {
    printf("%s - %s\n",cmd_table[i].cmd, cmd_table[i].doc);
  }
  return 1;
}

int cmd_wait(tok_t arg[]) {
    int status, wpid;
    while ((wpid = wait(&status)) > 0);
    return 1;
}

int cmd_pwd(tok_t arg[]) {
  char path[2048];
  if (getcwd(path, sizeof(path)) != NULL){
    fprintf(stdout, "%s\n", path);
  }
  else{
    fprintf(stdout, "error!");
  }
  return 1;
}


int cmd_cd(tok_t arg[]) 
{
  if (chdir(arg[0]) != 0){
    printf("cd: error!\n");
    return 1;
  }
  return 0;
}


int lookup(char cmd[]) {
  int i;
  for (i=0; i < (sizeof(cmd_table)/sizeof(fun_desc_t)); i++) {
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0)) return i;
  }
  return -1;
}

void init_shell()
{
  /* Check if we are running interactively */
  shell_terminal = STDIN_FILENO;

  /** Note that we cannot take control of the terminal if the shell
      is not interactive */
  shell_is_interactive = isatty(shell_terminal);

  if(shell_is_interactive){

    /* force into foreground */
    while(tcgetpgrp (shell_terminal) != (shell_pgid = getpgrp()))
      kill( - shell_pgid, SIGTTIN);

    shell_pgid = getpid();
    /* Put shell in its own process group */
    if(setpgid(shell_pgid, shell_pgid) < 0){
      perror("Couldn't put the shell in its own process group");
      exit(1);
    }

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);
    tcgetattr(shell_terminal, &shell_tmodes);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
  }
  /** YOUR CODE HERE */
}

/**
 * Add a process to our process list
 */
void add_process(process* p)
{
  /** YOUR CODE HERE */
}

/**
 * Creates a process given the inputString from stdin
 */
process* create_process(char* inputString)
{
  /** YOUR CODE HERE */
  return NULL;
}



int shell (int argc, char *argv[]) {
  char *s = malloc(INPUT_STRING_SIZE+1);			/* user input string */
  tok_t *t;			/* tokens parsed from input */
  int lineNum = 0;
  int fundex = -1;
  pid_t pid = getpid();		/* get current processes PID */
  pid_t ppid = getppid();	/* get parents PID */
  pid_t cpid, tcpid, cpgid;

  init_shell();

  // printf("%s running as PID %d under %d\n",argv[0],pid,ppid);

  lineNum=0;
  // fprintf(stdout, "%d: ", lineNum);
  while ((s = freadln(stdin))){
    t = getToks(s); /* break the line into tokens */
    fundex = lookup(t[0]); /* Is first token a shell literal */
    if(fundex >= 0) cmd_table[fundex].fun(&t[1]);
    else {
    	int check = background_index(t);
    	pid_t child_pid = fork();
    	if (child_pid < 0){
    		continue;
    	} else if (child_pid == 0){
    		
		signal(SIGTSTP, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGTTIN, SIG_DFL);
		signal(SIGTTOU, SIG_DFL);
    		
    		int out_index = isDirectTok(t, REDIRECT_OUT);
	    	if (out_index > 0) {
			int file_desc = open(t[out_index + 1], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
			dup2(file_desc, STDOUT_FILENO);
			t[out_index] = t[out_index + 1] = NULL;
	    	}
    		int in_index = isDirectTok(t, REDIRECT_IN);
	    	if (in_index > 0) {
			int in_file_desc = open(t[in_index + 1], O_RDONLY);
			dup2(in_file_desc, STDIN_FILENO);
			t[in_index] = t[in_index + 1] = NULL;
	    	}
    		program_path(t);
    		execv(t[0], t);
    		exit(0);
    	} else{
    		if (check == 0){
    			put_in_background(child_pid);
    		} 
    	}
    }
    // fprintf(stdout, "%d: ", lineNum);
  }
  return 0;
}

void program_path(tok_t *tokens){
	if (access(tokens[0], F_OK)==0){
		return;
	}
	const char *exe_path = tokens[0];
	tok_t *path_vars = getToks(getenv("PATH"));
	
	for (int i=0; i < MAXIMUMTOKS - 1 && path_vars[i]; ++i){
		char *r_path = (char *) malloc(sizeof(char) * MAXLINE);
		strcat(r_path, path_vars[i]);
		strcat(r_path, "/");
		strcat(r_path, exe_path);
		strcat(r_path, "\0");
		if (access(r_path, F_OK)==0){
			tokens[0] = r_path;
			return;
		}
	}
}


int background_index(tok_t *t){
    int back_index = isDirectTok(t, BACKGROUND);
    if (back_index > 0)
        t[back_index] = NULL;
    return back_index;
}



void put_in_background(pid_t child_pid){
    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    setpgid(child_pid,child_pid);
    tcsetpgrp(STDIN_FILENO, child_pid);
    int status;
    waitpid(child_pid, &status, WUNTRACED);
    tcsetpgrp(STDIN_FILENO, shell_pgid);
}




