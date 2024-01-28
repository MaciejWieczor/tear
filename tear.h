#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <termios.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#define CTRL_KEY(k)		((k) & 0x1f)

#define TEAR_VERSION		"0.0.1"
#define TEAR_QUIT_TIMES		3

#define TAB_STOP		8

typedef struct erow {
	int size;			/* Row character size */
	int rsize;			/* Row rendered size (tabs) */
	char *chars;			/* String in the line */
	char *render;			/* String with rendered characters */
} erow;

struct editorConfig {
	int cx, cy;			/* Cursor position in character buffer */
	int rx;				/* Cursor position after rendering special characters (tabs) */

	int screenrows;			/* Height of the terminal */
	int screencols;			/* Width of the terminal */

	int rowoff;			/* Height offset for scrolling */
	int coloff;			/* Width offset for scrolling */

	int numrows;			/* Number of rows in the buffer */
	erow *row;			/* Line array */
	int dirty;			/* Flag set if buffer is unsaved */

	char *filename;			/* Name of the opened file */
	char statusmsg[80];		/* Status message string */
	time_t statusmsg_time;		/* Timestamp to erase after elapsed */

	struct termios orig_termios;	/* Terminal flags etc */
};

enum editorKey {
	BACKSPACE = 127,
	ARROW_LEFT = 1000,
	ARROW_RIGHT,
	ARROW_UP,
	ARROW_DOWN,
	DEL_KEY,
	HOME_KEY,
	END_KEY,
	PAGE_UP,
	PAGE_DOWN
};

struct abuf {
	char *b;
	int len;
};

#define ABUF_INIT {NULL, 0}

/* terminal */
void die(const char *s);
void disableRawMode(void);
void enableRawMode(void);
int editorReadKey(void);
int getCursorPosition(int *rows, int *cols);
int getWindowSize(int *rows, int *cols);

/* row operations */
int editorRowCxToRx(erow *row, int cx);
int editorRowRxToCx(erow *row, int rx);
void editorUpdateRow(erow *row);
void editorInsertRow(int at, char *s, size_t len);
void editorFreeRow(erow *row);
void editorDelRow(int at);
void editorRowInsertChar(erow *row, int at, int c);
void editorInsertNewline(void);
void editorRowAppendString(erow *row, char *s, size_t len);
void editorRowDelChar(erow *row, int at);

/* editor operations */
void editorInsertChar(int c);
void editorDelChar(void);
char *editorRowsToString(int *buflen);
void editorOpen(char *filename);
void editorSave(void);

/* append buffers */
void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);

/* output */
void editorScroll(void);
void editorDrawRows(struct abuf *ab);
void editorDrawStatusBar(struct abuf *ab);
void editorDrawMessageBar(struct abuf *ab);
void editorRefreshScreen(void);
void editorSetStatusMessage(const char *fmt, ...);

/* input */
char *editorPrompt(char *prompt, void (*callback)(char *, int));
void editorMoveCursor(int key);
void editorProcessKeypress(void);

/* find */
void editorFind(void);
