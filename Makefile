program: terminal.o row.o editor.o file.o buffer.o output.o input.o tear.o find.o syntax.o
	$(CC) -g -o tear terminal.o row.o editor.o file.o buffer.o output.o input.o tear.o find.o syntax.o -Wall -Wextra -pedantic -std=c99

.c.o:
	gcc -c -g $<
