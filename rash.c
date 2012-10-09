/******************************************
 * << RASH >> the Really Awkward SHell << *
 ******************************************/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>

#include "log.h"

#define WS " \f\n\r\t\v"  /* isspace() */
#define PROMPT "->) "

int input_loop(void);
int builtin_cmd(char **);
int execute_cmd(char **);
void sig_handler(int);
void cleanup(void);

int parseandredirectin (char *);
int parseandredirectout (char *);

static char *linebuf;
static data_t rash_log_data;
char *argvv[MAX_CANON], *p, *tokp;

int main( int argc, char **argv )
{
	int exitcode;

	/* allocate space for char buffer */
	linebuf = calloc( MAX_CANON,sizeof(char) );
	if( linebuf == NULL ) {
		fprintf( stderr,"malloc problem\n" );
		exit(EXIT_FAILURE);
	}

	/* ignore ^c */
	if( signal(SIGINT, sig_handler) == SIG_ERR )
		fprintf( stderr, "signal() error!\n" );

	/* main loop
	 * save the exit code, because we want to save the log regardless of
	 * if there was failure or success */
	exitcode = input_loop();

	if( savelog("rash.log") != 0 )
		perror("rash: savelog() failed");

	puts("");

	cleanup();
	exit(exitcode);
}

int input_loop(void)
{
	int childpid, i, linelen;

	for( printf("Welcome to RASH!\n\t\tBe safe\n\n\tType ctrl-d or \"exit\" to exit\n")
	    , printf(PROMPT)
	    ; fgets( linebuf, MAX_CANON, stdin ) != NULL
	    ; printf(PROMPT) )
	{
		/* append to log */
		rash_log_data.time = time(NULL);

		linelen = strlen(linebuf);
		rash_log_data.string = calloc(linelen + 1, sizeof(char));
		strncpy(rash_log_data.string, linebuf, linelen);
		addmsg(rash_log_data);

		/* lex command into argvv */
		i = 0;
		for( p = strtok_r(linebuf, WS, &tokp); p;
		    p = strtok_r(NULL, WS, &tokp) )
		{
			argvv[i++] = p;
		}
		argvv[i] = NULL;

		if( argvv[0] != NULL ) {
			if( builtin_cmd(argvv) ) {
				/* exit should be in builtin_cmd() */
				if( !strcmp("exit",argvv[0]) )
					return EXIT_SUCCESS;

				if( (childpid = fork()) == -1 )
					perror("Failed to fork child");
				else if( childpid == 0 ) {
					if( execute_cmd(argvv) )
						return EXIT_FAILURE;
				} else {
					wait(NULL);
				}
			}
		}
	}
	return EXIT_SUCCESS;
}

/* returns 0 if builtin command was found, 1 otherwise */
int builtin_cmd(char **cmd)
{
	/*
	 * built-in shell commands
	 */

	/* cd: change directory */
	if( !strcmp("cd",cmd[0]) ) {
		if( cmd[1]==NULL )
			cmd[1] = getenv("HOME");

		if( chdir(cmd[1]) )
			printf("rash: can't cd to %s\n", cmd[1]);


		return 0;
	}

	/* builtin command not found */
	return 1;
}

int execute_cmd(char **cmd)
{
	parseandredirectin(cmd[0]);
	execvp( cmd[0] , cmd );
	perror("execute_cmd failed");
	return 1;
}

void sig_handler(int sig)
{
	/* TODO: make a note that we got it */
	if( sig == SIGINT );
}

/* free any allocated memory */
void cleanup(void)
{
	free(linebuf);
}
