#include "tear.h"

/* Global data */

struct editorConfig E;

void initEditor(void) {
	E.statusmsg[0] = '\0';
	E.statusmsg_time = 0;
	E.filename = NULL;
	E.syntax = NULL;
	E.numrows = 0;
	E.rowoff = 0;
	E.coloff = 0;
	E.row = NULL;
	E.dirty = 0;
	E.cx = 0;
	E.cy = 0;
	E.rx = 0;

	if (getWindowSize(&E.screenrows, &E.screencols) == -1)
		die("getWindowSize");

	/* Make space for status line and message line */
	E.screenrows -= 2;
}

int main(int argc, char *argv[]) {
	enableRawMode();
	initEditor();

	/* If there is no filename open empty buffer */
	if (argc >= 2)
		editorOpen(argv[1]);

	editorSetStatusMessage("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find");

	while (1) {
		editorRefreshScreen();
		editorProcessKeypress();
	}

	return 0;
}
