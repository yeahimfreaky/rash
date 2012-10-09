CC=gcc
CFLAGS=-Wall -g

rash: rash.o loglib.o parseandredirect.o
	${CC} ${CFLAGS} -o $@ $^

.PHONY: clean

clean:
	rm -rf rash *.o *~ \#*\#
