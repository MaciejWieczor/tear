#include "tear.h"

extern struct editorConfig E;

/*** input ***/

/*
 * editorPrompt - input on the message line
 *
 * @prompt - some fmt string, needs to have %s
 *
 * Return - user input string on the message line
 */
char *editorPrompt(char *prompt, void (*callback)(char *, int)) {
	size_t bufsize = 128;
	char *buf = malloc(bufsize);
	size_t buflen = 0;
	buf[0] = '\0';
	while (1) {
		editorSetStatusMessage(prompt, buf);
		editorRefreshScreen();

		int c = editorReadKey();
		if (c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE) {
			if (buflen != 0)
				buf[--buflen] = '\0';
		} else if (c == '\x1b') {
			editorSetStatusMessage("");
			if (callback)
				callback(buf, c);
			free(buf);
			return NULL;
		} else if (c == '\r') {
			if (buflen != 0) {
				editorSetStatusMessage("");
				if (callback)
					callback(buf, c);
				return buf;
			}
		} else if (!iscntrl(c) && c < 128) {
			if (buflen == bufsize - 1) {
				bufsize *= 2;
				buf = realloc(buf, bufsize);
			}
			buf[buflen++] = c;
			buf[buflen] = '\0';
		}
		if (callback)
			callback(buf, c);
	}
}

/*
 * editorMoveCursor - Moves cursor incrementally based on the direction argument
 */
void editorMoveCursor(int key) {
	erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];

	switch (key) {
		case ARROW_LEFT:
			if (E.cx != 0)
				E.cx--;
			else if (E.cy > 0) {
				E.cy--;
				E.cx = E.row[E.cy].size;
			}
			break;
		case ARROW_RIGHT:
			if (row && E.cx < row->size)
				E.cx++;
			else if (row && E.cy < E.numrows - 1) {
				E.cy++;
				E.cx = 0;
			}
			break;
		case ARROW_UP:
			if (E.cy != 0)
				E.cy--;
			break;
		case ARROW_DOWN:
			if (E.cy < E.numrows - 1)
				E.cy++;
			break;
	}

	/*
	 * Resolve problem with moving from the end of a longer line to the next
	 * shorter line
	 * */
	row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
	int rowlen = row ? row->size : 0;
	if (E.cx > rowlen)
		E.cx = rowlen;
}

/*
 * editorProcessKeypress - converts a key enum received from editorReadKey()
 * to operation logic
 */
void editorProcessKeypress(void) {
	static int quit_times = TEAR_QUIT_TIMES;
	int c = editorReadKey();
	/* Here I could resolve custom keybindings */

	switch (c) {
		case '\r':
			editorInsertNewline();
			break;
		case CTRL_KEY('q'):
			if (E.dirty && quit_times > 0) {
				editorSetStatusMessage("WARNING!!! File has unsaved changes. "
				"Press Ctrl-Q %d more times to quit.", quit_times);
				quit_times--;
				return;
			}
			write(STDOUT_FILENO, "\x1b[2J", 4);
			write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
			break;
		case CTRL_KEY('s'):
			editorSave();
			break;
		case HOME_KEY:
			E.cx = 0;
			break;
		case END_KEY:
			if (E.cy < E.numrows)
				E.cx = E.row[E.cy].size;
			break;
		case CTRL_KEY('f'):
			editorFind();
			break;
		case BACKSPACE:
		/* Old school backspace code */
		case CTRL_KEY('h'):
		case DEL_KEY:
			if (c == DEL_KEY)
				editorMoveCursor(ARROW_RIGHT);
			editorDelChar();
			break;
		case PAGE_UP:
		case PAGE_DOWN:
			{
			/* Move cursor to top/bottom of screen before moving a page up/down */
			if (c == PAGE_UP)
				E.cy = E.rowoff;
			else if (c == PAGE_DOWN) {
				E.cy = E.rowoff + E.screenrows - 1;
				if (E.cy > E.numrows)
					E.cy = E.numrows;
			}
			int times = E.screenrows;
			while (times--)
				editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
			}
			break;
		case ARROW_UP:
		case ARROW_DOWN:
		case ARROW_LEFT:
		case ARROW_RIGHT:
			editorMoveCursor(c);
			break;
		/* For now the default case is to just write characters */
		case CTRL_KEY('l'):
		/* Ignore esc key - maybe come back here later for F1-F12 stuff */
		case '\x1b':
			break;
		default:
			editorInsertChar(c);
			break;
	}
	quit_times = TEAR_QUIT_TIMES;
}
