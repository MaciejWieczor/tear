#include "tear.h"

extern struct editorConfig E;

/*** output ***/

/*
 * editorScroll - adjust the row/column offsets if scrolling happens
 */
void editorScroll(void) {
	E.rx = 0;
	if (E.cy < E.numrows)
		E.rx = editorRowCxToRx(&E.row[E.cy], E.cx);
	if (E.cy < E.rowoff)
		E.rowoff = E.cy;
	if (E.cy >= E.rowoff + E.screenrows)
		E.rowoff = E.cy - E.screenrows + 1;
	if (E.rx < E.coloff)
		E.coloff = E.rx;
	if (E.rx >= E.coloff + E.screencols)
		E.coloff = E.rx - E.screencols + 1;
}

/*
 * editorDrawRows - draws the visible part of the loaded file
 */
void editorDrawRows(struct abuf *ab) {
	int y;
	for (y = 0; y < E.screenrows; y++) {
		int filerow = E.rowoff + y;
		if (filerow >= E.numrows) {
			/* Welcome string */
			if (E.numrows == 0 && y == E.screenrows / 3) {
				char welcome[80];
				int welcomelen = snprintf(welcome, sizeof(welcome),
				"Tear editor -- version %s", TEAR_VERSION);
				if (welcomelen > E.screencols)
					welcomelen = E.screencols;
				int padding = (E.screencols - welcomelen) / 2;
				if (padding) {
					abAppend(ab, "~", 1);
					padding--;
				}
				while (padding--)
					abAppend(ab, " ", 1);
				abAppend(ab, welcome, welcomelen);
			/* End of the welcome string */
			} else {
				abAppend(ab, "~", 1);
			}
		} else {
			int len = E.row[filerow].rsize - E.coloff;
			if (len < 0)
				len = 0;
			if (len >= E.screencols)
				len = E.screencols;
			char *c = &E.row[filerow].render[E.coloff];
			unsigned char *hl = &E.row[filerow].hl[E.coloff];
			int current_color = -1;
			int j;
			for (j = 0; j < len; j++) {
				if (hl[j] == HL_NORMAL) {
					if (current_color != -1) {
						abAppend(ab, "\x1b[39m", 5);
						current_color = -1;
					}
					abAppend(ab, &c[j], 1);
				} else {
					int color = editorSyntaxToColor(hl[j]);
					if (color != current_color) {
						current_color = color;
						char buf[16];
						int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", color);
						abAppend(ab, buf, clen);
					}
					abAppend(ab, &c[j], 1);
				}
			}
			abAppend(ab, "\x1b[39m", 5);
		}

		abAppend(ab, "\x1b[K", 3);
		abAppend(ab, "\r\n", 2);
	}
}

void editorDrawStatusBar(struct abuf *ab) {
	/* Invert color escape sequence (black text on white bg) (argument 7) */
	abAppend(ab, "\x1b[7m", 4);

	/* Write the current line over the total number of lines */
	char rstatus[80];
	int rlen = snprintf(rstatus, sizeof(rstatus), "%s | %d/%d",
		E.syntax ? E.syntax->filetype : "no ft", E.cy + 1, E.numrows);

	/* Write 20 chars of the filename and the number of lines in the file */
	char status[80];
	int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
			   E.filename ? E.filename : "[No Name]", E.numrows,
			   E.dirty ? "(modified)" : "");
	if (len > E.screencols)
		len = E.screencols;
	abAppend(ab, status, len);

	while (len < E.screencols) {
		/* At the end of the line write the current line */
		if (E.screencols - len == rlen) {
			abAppend(ab, rstatus, rlen);
			break;
		}
		/* Write spaces till end line to ensure white background */
		else {
			abAppend(ab, " ", 1);
			len++;
		}
	}
	/* Go back to normal formatting (default 0 argument) */
	abAppend(ab, "\x1b[m", 3);
	abAppend(ab, "\r\n", 2);
}

void editorDrawMessageBar(struct abuf *ab) {
	abAppend(ab, "\x1b[K", 3);
	int msglen = strlen(E.statusmsg);
	if (msglen > E.screencols)
		msglen = E.screencols;
	if (msglen && time(NULL) - E.statusmsg_time < 5)
		abAppend(ab, E.statusmsg, msglen);
}

/*
 * editorRefreshScreen - clears the screen, builds the buffer to draw and draws it
 */
void editorRefreshScreen(void) {
	struct abuf ab = ABUF_INIT;
	char buf[32];

	editorScroll();

	/* Hide cursor */
	abAppend(&ab, "\x1b[?25l", 6);
	abAppend(&ab, "\x1b[H", 3);

	editorDrawRows(&ab);
	editorDrawStatusBar(&ab);
	editorDrawMessageBar(&ab);

	snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy - E.rowoff + 1,
						  E.rx - E.coloff + 1);
	abAppend(&ab, buf, strlen(buf));

	/* Hide cursor */
	abAppend(&ab, "\x1b[?25h", 6);

	write(STDOUT_FILENO, ab.b, ab.len);

	abFree(&ab);
}

void editorSetStatusMessage(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
	va_end(ap);
	E.statusmsg_time = time(NULL);
}
