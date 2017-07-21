#include "cursor.h"
#include "define.h"

void seek_line_end(DOCUMENT *doc, CURSOR *cur) {
	STRING *line = get_line(doc, cur);

	if (cur->x + cur->horizontal_scroll > line->length) {
		if (line->length > cur->max_window_x) {
			cur->horizontal_scroll = line->length - cur->max_window_x;
			cur->x = line->length - cur->horizontal_scroll;
		} else {
			cur->x = line->length;
			cur->horizontal_scroll = 0;
		}
		if (cur->x < 0) {
			cur->x = 0;
		}
	}
}

void seek_line_start(DOCUMENT *doc, CURSOR *cur) {
	cur->x = 0;
	cur->horizontal_scroll = 0;
	STRING *line = get_line(doc, cur);

	for (int i = 0; i < line->length; i++) {
		char ch = line->array[i];
		if (ch == '\t' || ch == ' ') {
			increment_x(doc, cur, false);
		} else {
			break;
		}
	}
}	

void decrement_y(DOCUMENT *doc, CURSOR *cur) {
	if (cur->y > 0) {
		cur->y--;
	} else if (cur->vertical_scroll > 0) {
		cur->vertical_scroll--;
	}

	seek_line_end(doc, cur);
}

void increment_y(DOCUMENT *doc, CURSOR *cur) {
	if (cur->y + cur->vertical_scroll < doc->length - 1) {
		if (cur->y < cur->max_window_y) {
			cur->y++;
		} else {
			cur->vertical_scroll++;
		}
	}

	seek_line_end(doc, cur);
}

void decrement_y_para(DOCUMENT *doc, CURSOR *cur) {
	decrement_y(doc, cur);
	for (int i = cur->y + cur->vertical_scroll - 1; i >= 0; i--) {
		if (doc->lines[i]->length > 0) {
			decrement_y(doc, cur);
		} else {
			break;
		}
	}
}

void increment_y_para(DOCUMENT *doc, CURSOR *cur) {
	increment_y(doc, cur);
	for (int i = cur->y + cur->vertical_scroll; i < doc->length; i++) {
		if (doc->lines[i]->length > 0) {
			increment_y(doc, cur);
		} else {
			break;
		}
	}
	increment_y(doc, cur);
}

void shift_x(CURSOR *cur, char enabled) {
}

void decrement_x(DOCUMENT *doc, CURSOR *cur, char shift) {
	if (cur->x > 0) {
		cur->x--;
	} else if (cur->horizontal_scroll > 0) {
		cur->horizontal_scroll--;
	}

	shift_x(cur, shift);
}

void increment_x(DOCUMENT *doc, CURSOR *cur, char shift) {
	STRING *line = get_line(doc, cur);

	if (cur->x + cur->horizontal_scroll < line->length) {
		shift_x(cur, shift);
		if (!shift) {
			if (cur->x + get_tab_offset(doc, cur) >= cur->max_window_x) {
				cur->horizontal_scroll++;
			} else {
				cur->x++;
			}
		}
	}
}

char get_cursor_char(DOCUMENT *doc, CURSOR *cur) {
	STRING *line = get_line(doc, cur);
	return line->array[cur->x + cur->horizontal_scroll];
}

void decrement_x_word(DOCUMENT *doc, CURSOR *cur) {
	decrement_x(doc, cur, false);
	while (cur->x > 0 || cur->horizontal_scroll > 0) {
		decrement_x(doc, cur, false);
		char ch = get_cursor_char(doc, cur);
		if (ch == ' ' || ch == '\t') {
			increment_x(doc, cur, false);
			break;
		}
	}
}

void increment_x_word(DOCUMENT *doc, CURSOR *cur) {
	STRING *line = get_line(doc, cur);
	while (cur->x + cur->horizontal_scroll < line->length) {
		increment_x(doc, cur, false);
		char ch = get_cursor_char(doc, cur);
		if (ch == ' ' || ch == '\t') {
			increment_x(doc, cur, false);
			break;
		}
	}
}

void goto_line(DOCUMENT *doc, CURSOR *cur, int line_number) {
	if (line_number >= doc->length) {
		line_number = doc->length - 1;
	} else if (line_number < 0) {
		line_number = 0;
	}

	if (line_number == 0) {
		cur->y = 0;
		cur->vertical_scroll = 0;
	} else if (doc->length < cur->max_window_y) {
		cur->y = line_number;
		cur->vertical_scroll = 0;
	} else if (line_number < doc->length - cur->max_window_y) {
		cur->y = 0;
		cur->vertical_scroll = line_number - 1;
	} else {
		cur->vertical_scroll = doc->length - cur->max_window_y - 1;
		cur->y = line_number - cur->vertical_scroll;
	}
	cur->x = 0;
	cur->horizontal_scroll = 0;
}

int get_tab_offset(DOCUMENT *doc, CURSOR *cur) {
	STRING *line = get_line(doc, cur);
	int tab_offset = 0;
	for (int x = cur->horizontal_scroll; x < cur->horizontal_scroll + cur->max_window_x; x++) {
		if (line->array[x] == '\t') {
			tab_offset += TAB_WIDTH - 1;
		}
	}
	return tab_offset;
}

