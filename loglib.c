#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "log.h"

typedef struct list_struct
{
	data_t item;
	struct list_struct *next;
} log_t;

static log_t *headptr = NULL;
static log_t *tailptr = NULL;

/* pretty much taken from the book's adddata() of Program 2.7 */
int addmsg(data_t data)
{
	log_t *newnode;
	int nodesize;

	nodesize = sizeof(log_t) + strlen(data.string) + 1;
	if( (newnode = (log_t *)(malloc(nodesize))) == NULL )
		return -1;

	newnode->item.time = data.time;
	newnode->item.string = (char *)newnode + sizeof(log_t);
	strcpy(newnode->item.string, data.string);
	newnode->next = NULL;
	if( headptr==NULL )
		headptr = newnode;
	else
		tailptr->next = newnode;
	tailptr = newnode;

	return 0;
}

/* free all the memory and reset the linked list to its initial state */
void clearlog(void)
{
	log_t *p;

	for( p = headptr; p != NULL; p = p->next )
		free( p );

	headptr->next = tailptr;
}

/* getlog() returns a pointer to the log string. The caller is responsible for
 * freeing the memory */
char *getlog(void)
{
	log_t *p;
	char *s;
	char buf[MAX_CANON], therest[MAX_CANON];
	int logsize = 0;
	struct tm *ts;

	/* find out how big the log is */
	for( p = headptr; p != NULL; p = p->next )
		logsize += strlen(p->item.string) + 12 + 1;

	/* allocate space for the log (must be freed by caller */
	s = calloc(logsize, sizeof(char));

	for( p = headptr; p != NULL; p = p->next ) {
		/* format time */
		ts = localtime(&(p->item.time));
		strftime(buf, sizeof(buf), "%H:%M:%S", ts);

		snprintf(therest, MAX_CANON, " ~~ %s", p->item.string);
		strncat(buf, therest, MAX_CANON);
		strncat(s, buf, MAX_CANON);
		if( p->next == NULL )
			return s;
	}

	return NULL;
}

/* save the log as a file called 'filename' in the current working dir */
int savelog(char *filename)
{
	FILE *f;
	char *str, *fullfilename;

	fullfilename = calloc(MAX_CANON, sizeof(char));
	if( getcwd(fullfilename, MAX_CANON) == NULL )
		return 1;

	strncat(fullfilename, "/", 1);
	strncat(fullfilename, filename, MAX_CANON);

	if( (f = fopen(fullfilename,"w")) == NULL )
		return errno;

	if( (str = getlog()) != NULL ) {
		if( fprintf(f, "%s", str) < 0 ) {
			free(str);
			return errno;
		} else {
			free(str);
		}
	}

	return 0;
}
