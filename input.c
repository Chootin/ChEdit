#include "input.h"
#include <curses.h>
#include "document.h"
#include "cursor.h"

/**
*** Command key combinations for ChEdit
*** Returns: 0 - No key combination found
***          1 - Key combination found but no document edit took place
***          2 - Key combination found and document edit took place
**/
int process_command(DOCUMENT *doc, CURSOR *cur, char *ch) {
	if (s_equals(ch, "\x1B[D")) {//LEFT
		decrement_x(doc, cur, false);
		return 1;
	} else if (s_equals(ch, "\x1B[C")) { //RIGHT
		increment_x(doc, cur, false);
		return 1;
	}else if (s_equals(ch, "\x1B[A")) {//UP
		decrement_y(doc, cur);
		return 1;
	} else if (s_equals(ch, "\x1B[B")) {//DOWN
		increment_y(doc, cur);
		return 1;
	} else if (s_equals(ch, "\x1B[1;5D")) {//CTRL+LEFT
		decrement_x_word(doc, cur);
		return 1;
	} else if (s_equals(ch, "\x1B[1;5C")) {//CTRL+RIGHT
		increment_x_word(doc, cur);
		return 1;
	} else if (s_equals(ch, "\x1B[1;5A")) {//CTRL+UP
		decrement_y_para(doc, cur);
		return 1;
	} else if (s_equals(ch, "\x1B[1;5B")) {//CTRL+DOWN
		increment_y_para(doc, cur);
		return 1;
	} else if (s_equals(ch, "\x1BOH") || s_equals(ch, "\x1B[H")) {//HOME
		int current_pos = cur->x + cur->horizontal_scroll;
		seek_line_start(doc, cur);
		if (current_pos == cur->x + cur->horizontal_scroll) {
			cur->x = 0;
			cur->horizontal_scroll = 0;
		}
		return 1;
	} else if (s_equals(ch, "\x1BOF") || s_equals(ch, "\x1B[F")) {//END
		STRING *line = get_line(doc, cur);
		if (line->length < cur->max_window_x) {
			cur->x = line->length;
		} else {
			cur->horizontal_scroll = line->length - cur->max_window_x;
			cur->x = line->length - cur->horizontal_scroll;
		}
		return 1;
	} else if (s_equals(ch, "\x1B[5;5~")) {//CTRL+PGUP
		goto_line(doc, cur, 0);
		return 1;
	} else if (s_equals(ch, "\x1B[6;5~")) {//CTRL+PGDN
		goto_line(doc, cur, doc->length - 1);
		seek_line_end(doc, cur);
		return 1;
	} else if (s_equals(ch, "\x1B[5~")) {//PGUP
		goto_line(doc, cur, cur->y + cur->vertical_scroll - cur->max_window_y);
		return 1;
	} else if (s_equals(ch, "\x1B[6~")) {//PGDN
		goto_line(doc, cur, cur->y + cur->vertical_scroll + cur->max_window_y + 2);
		return 1;
	} else if (s_equals(ch, "\x1B[3~")) {//DELETE
		delete_character(doc, cur);
		return 2;
	} else if (s_equals(ch, "\x1B\x7F")) {//ALT+BACKSPACE
		erase_word(doc, cur);
		return 2;
	} else if (s_equals(ch, "\x7F")) {//BACKSPACE
		erase_character(doc, cur);
		return 2;
	} else if (s_equals(ch, "\x1B[B\x1B[B")) {//SCROLL DOWN
		increment_y(doc, cur);
		return 1;
	} else if (s_equals(ch, "\x1B[A\x1B[A")) {//SCROLL UP
		decrement_y(doc, cur);
		return 1;
	} else if (s_equals(ch, "\x1B[1;2D")) {//SHIFT+LEFT
		decrement_x(doc, cur, true);
		return 1;
	} else if (s_equals(ch, "\x1B[1;2C")) {//SHIFT+RIGHT
		increment_x(doc, cur, true);
		return 1;
	}
	//TODO:
	//SHIFT+ARROWS
	//SHIFT+TAB - Remove tab from start of line
	//COPY
	//CUT
	//PASTE
	//CTRL+BACKSPACE - instead of ALT+BACKSPACE
	return 0;
}

int process_text(DOCUMENT *doc, CURSOR *cur, char *ch) {
	if (ch[0] != ERR && ch[0] != 0) {
		if (ch[0] == '\t') { //TAB
			insert_character(doc, cur, '\t');
			increment_x(doc, cur, false);
		} else if (ch[0] == '\n' || ch[0] == '\r') {//ENTER
			STRING *new_line = crop_string(get_line(doc, cur), cur->x + cur->horizontal_scroll);
			insert_new_line(doc, cur, new_line);
			increment_y(doc, cur);
			cur->x = 0;
		} else if (ch[0] >= 32 && ch[0] <= 126) {//A standard printable character
			insert_character(doc, cur, ch[0]);
			increment_x(doc, cur, false);
		} else {
			return 0;
		}
		return 2;
	}
	return 0;
}

char is_alpha(char ch) {
	return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
}

char is_numeric(char ch) {
	return ch >= '0' && ch <= '9';
}

char is_printable(char ch) {
	return ch >= ' ' && ch <= '~';
}

char is_esc(char ch) {
	return ch == '\x1B';
}
