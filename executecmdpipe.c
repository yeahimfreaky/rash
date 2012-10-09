#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void executeredirect( char *, int, int );
int makeargv( const char *, const char *, char *** );

static void perror_exit( char *s )
{
	perror(s);
	exit(1);
}

void executecmd( char *cmds )
{
	int child;
	int count;
	int fds[2];
	int i;
	char **pipelist;

	count = makeargv( cmds, "|", &pipelist );
	if( count <= 0 ) {
		fprintf( stderr, "Failed to find any commands\n" );
		exit(1);
	}
	for( i=0; i < count-1; i++ ) {
		if( pipe(fds) == -1 )
			perror_exit("Failed to create pipes");
		else if( (child = fork()) == -1 )
			perror_exit("Failed to create process to run command");
		else if( child ) {
			if( dup2(fds[1], STDOUT_FILENO) == -1 )
				perror_exit("Failed to connect pipeline");
			if( close(fds[0]) || close(fds[1]) )
				perror_exit("Failed to close needed files");
			executeredirect( pipelist[i], i==0, 0 );
			exit(1);
		}
		if( dup2(fds[0], STDIN_FILENO) == -1 )   /* child code */
			perror_exit("Failed to connect last component");
		if( close(fds[0]) || close(fds[1]) )
			perror_exit("Failed to do final close");
	}
	executeredirect( pipelist[i], i==0, 1 );  /* handle the last one */
	exit(1);
}
