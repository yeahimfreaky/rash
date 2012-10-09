#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#define FFLAG (O_WRONLY | O_CREAT | O_TRUNC)
#define FMODE (S_IRUSR | S_IWUSR)
#define WS " \f\n\r\t\v"  /* isspace() */

int parseandredirectin (char *cmd)
{
	int error;
	int infd;
	char *infile;

	if ( (infile = strchr(cmd, '<')) == NULL )
		return 0;

	*infile = 0;
	infile = strtok(infile + 1, WS);
	if (infile == NULL)
		return 0;

	if ( (infd = open(infile, O_RDONLY)) == -1 )
		return -1;

	if ( dup2(infd, STDIN_FILENO) == -1 ) {
		error = errno;
		close(infd);
		errno = error;
		return -1;
	}

	return close(infd);
}

int parseandredirectout (char *cmd)
{
	int error;
	int outfd;
	char *outfile;

	if ( (outfile = strchr(cmd, '>')) == NULL )
		return 0;

	*outfile = 0;
	outfile = strtok(outfile + 1, WS);
	if (outfile == NULL)
		return 0;

	if ( (outfd = open(outfile, FFLAG, FMODE)) == -1 )
		return -1;

	if ( dup2(outfd, STDOUT_FILENO) == -1 ) {
		error = errno;
		close(outfd);
		errno = error;
		return -1;
	}

	return close(outfd);
}
