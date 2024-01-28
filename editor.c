#include "tear.h"

extern struct editorConfig E;

/*
 * editorInsertChar - insert char c at current cursor position
 */
void editorInsertChar(int c) {
	if (E.cy == E.numrows)
		editorInsertRow(E.numrows, "", 0);
	editorRowInsertChar(&E.row[E.cy], E.cx, c);
	E.cx++;
}

void editorDelChar(void) {
	if (E.cy == E.numrows)
		return;
	if (E.cx == 0 && E.cy == 0)
		return;

	erow *row = &E.row[E.cy];
	/* Delete character */
	if (E.cx > 0) {
		editorRowDelChar(row, E.cx - 1);
		E.cx--;
	/* On first index of the line. Move the line to the back of the
	 * previous line */
        } else {
		E.cx = E.row[E.cy - 1].size;
		editorRowAppendString(&E.row[E.cy - 1], row->chars, row->size);
		editorDelRow(E.cy);
		E.cy--;
	}
}
