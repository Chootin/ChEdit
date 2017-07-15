#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include "chedit.h"

#define DEBUG_STRINGS (6)
#define DEBUG_COLUMNS (19)
#define TAB_WIDTH (4)
#define CHARACTER_INPUT_ARR_LENGTH (6)
#define STRING_NUMBER_WIDTH (4)
#define CURSOR_SPEED (25)
#define FIND_STRING_LENGTH (50)
#define GOTO_STRING_LENGTH (10)

volatile int interrupt = 0;

void curses_setup() {
	initscr();
	noecho();
	curs_set(0);
	nodelay(stdscr, true);
}

void draw_diag_win(WINDOW *diag_win, int interrupt, int max_y, int max_x, int cur_y, int cur_x, char *ch) {
	wclear(diag_win);
	mvwprintw(diag_win, 0, 0, "Debug");
	mvwprintw(diag_win, 1, 0, "Interrupt: %d", interrupt);
	mvwprintw(diag_win, 2, 0, "Width: %d", max_x);
	mvwprintw(diag_win, 3, 0, "Height: %d", max_y);
	mvwprintw(diag_win, 4, 0, "X: %d, Y: %d", cur_x, cur_y);
	mvwprintw(diag_win, 5, 0, "%d %d %d %d %d %d %d", ch[0], ch[1], ch[2], ch[3], ch[4], ch[5], ch[6]);
	wnoutrefresh(diag_win);
}

void draw_title_bar(WINDOW *window, int max_x, char *savepath, char *savefile, char unsaved_changes) {
	wclear(window);
	wstandout(window);
	mvwhline(window, 0, 0, ' ', max_x);
	if (unsaved_changes) {
		mvwprintw(window, 0, STRING_NUMBER_WIDTH, "ChEdit: %s/%s*", savepath, savefile);
	} else {
		mvwprintw(window, 0, STRING_NUMBER_WIDTH, "ChEdit: %s/%s", savepath, savefile);
	}
	wstandend(window);
	wnoutrefresh(window);
}

int get_line_display_length(STRING *line) {
	int length = 0;
	for (int i = 0; i < line->length; i++) {
		if (line->array[i] == '\t') {
			length += TAB_WIDTH;
		} else {
			length++;
		}
	}
}

char tick_cursor(CURSOR *cur) {
	if (cur->highlight_tick++ == CURSOR_SPEED) {
		cur->highlight_tick = 0;
		cur->highlight = !cur->highlight;
		return TRUE;
	}
	return FALSE;
}

char cursor_active_here(CURSOR *cur, int y, int x) {
	int cur_pos = cur->x + cur->horizontal_scroll;
	if (cur->y == y) {
		if (cur->selection) {
			if (x >= cur_pos && x < cur_pos + cur->select_x) {
				return TRUE;
			}
		} else {
			return cur->highlight && cur->x + cur->horizontal_scroll == x;
		}
	}
	return FALSE;
}

STRING * get_line(DOCUMENT *doc, CURSOR *cur) {
	return doc->lines[cur->y + cur->vertical_scroll];
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

void draw_text(WINDOW *window, DOCUMENT *doc, CURSOR *cur) {
	wclear(window);
	STRING **lines = doc->lines;

	for (int y = 0; y < doc->length - cur->vertical_scroll && y <= cur->max_window_y; y++) {
		int draw_x = 0;
		int start_x = 0;
		int offset = cur->x + get_tab_offset(doc, cur) - cur->max_window_x;
		STRING *line = lines[y + cur->vertical_scroll];
		if (y == cur->y) {
			start_x = cur->horizontal_scroll;
			if (offset > 0) {
				start_x += offset;
			}
		}
		for (int x = start_x; x < line->length && draw_x <= cur->max_window_x; x++) {
			char ch = line->array[x];
			if (cursor_active_here(cur, y, x)) {
				wstandout(window);
			}
			if (ch == '\t') {
				for (int i = 0; i < TAB_WIDTH; i++) {
					mvwaddch(window, y, draw_x, ' ');
					draw_x++;
				}
			} else {
				mvwaddch(window, y, draw_x, ch);
				draw_x++;
			}
			wstandend(window);
		}

		if (cur->highlight && cur->y == y && cur->x + cur->horizontal_scroll == line->length) {
			wstandout(window);
			mvwaddch(window, y, draw_x, ' ');
			wstandend(window);
		}
	}
	wnoutrefresh(window);
}

void reset_cursor_highlight(CURSOR *cur) {
	cur->highlight = 1;
	cur->highlight_tick = 0;
}

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
			increment_x(doc, cur, FALSE);
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

	reset_cursor_highlight(cur);
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

	reset_cursor_highlight(cur);
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

void decrement_x(CURSOR *cur, char shift) {
	if (cur->x > 0) {
		cur->x--;
	} else if (cur->horizontal_scroll > 0) {
		cur->horizontal_scroll--;
	}

	reset_cursor_highlight(cur);
}

void increment_x(DOCUMENT *doc, CURSOR *cur, char shift) {
	STRING *line = get_line(doc, cur);

	if (cur->x + cur->horizontal_scroll < line->length) {
		if (cur->x + get_tab_offset(doc, cur) >= cur->max_window_x) {
			cur->horizontal_scroll++;
		} else {
			cur->x++;
		}
	}

	reset_cursor_highlight(cur);
}

char get_cursor_char(DOCUMENT *doc, CURSOR *cur) {
	STRING *line = get_line(doc, cur);
	return line->array[cur->x + cur->horizontal_scroll];
}

void decrement_x_word(DOCUMENT *doc, CURSOR *cur) {
	decrement_x(cur, FALSE);
	while (cur->x > 0 || cur->horizontal_scroll > 0) {
		decrement_x(cur, FALSE);
		char ch = get_cursor_char(doc, cur);
		if (ch == ' ' || ch == '\t') {
			increment_x(doc, cur, FALSE);
			break;
		}
	}
}

void increment_x_word(DOCUMENT *doc, CURSOR *cur) {
	STRING *line = get_line(doc, cur);
	while (cur->x + cur->horizontal_scroll < line->length) {
		increment_x(doc, cur, FALSE);
		char ch = get_cursor_char(doc, cur);
		if (ch == ' ' || ch == '\t') {
			increment_x(doc, cur, FALSE);
			break;
		}
	}
}

void delete_line(DOCUMENT *doc, CURSOR *cur) {
	int y = cur->y-- + cur->vertical_scroll;
	doc->length--;

	free(doc->lines[y]->array);
	free(doc->lines[y]);
	
	for (int i = y; i < doc->length; i++) {
		doc->lines[i] = doc->lines[i + 1];
	}
}

void append_line(STRING *line, STRING *append) {
	int new_length = line->length + append->length;
	char *old_array = line->array;
	char *new_array = (char *) malloc(new_length * sizeof(char));

	for (int i = 0; i < line->length; i++) {
		new_array[i] = old_array[i];
	}

	for (int i = 0; i < append->length; i++) {
		new_array[i + line->length] = append->array[i];
	}

	line->length = new_length;
	free(old_array);
	line->array = new_array;
}

void delete_character(DOCUMENT *doc, CURSOR *cur) {
	STRING *line = get_line(doc, cur);
	if (line->length > 0 && cur->x + cur->horizontal_scroll < line->length) {
		char *array = line->array;
		for (int i = cur->x + cur->horizontal_scroll; i < line->length; i++) {
			array[i] = array[i + 1];
		}
		line->length--;
	} else if (cur->y + cur->vertical_scroll < doc->length - 1) {
		STRING *line = get_line(doc, cur);
		STRING *next_line = doc->lines[cur->y + cur->vertical_scroll + 1];
		append_line(line, next_line);
		cur->y++;
		delete_line(doc, cur);
	}
	reset_cursor_highlight(cur);
}

void erase_character(DOCUMENT *doc, CURSOR *cur) {
	if (cur->x + cur->horizontal_scroll == 0) {
		if (cur->y + cur->vertical_scroll > 0) {
			STRING *line = get_line(doc, cur);
			STRING *last_line = doc->lines[cur->y + cur->vertical_scroll - 1];
			int new_cur_x = last_line->length;
			if (line->length > 0) {
				append_line(last_line, line);
			}
			delete_line(doc, cur);
			if (new_cur_x > cur->max_window_x) {
				cur->x = cur->max_window_x;
				cur->horizontal_scroll = new_cur_x - cur->x;
			} else {
				cur->x = new_cur_x;
			}
			
			reset_cursor_highlight(cur);
		}
	} else {
		decrement_x(cur, FALSE);
		delete_character(doc, cur);
	}
}

void erase_word(DOCUMENT *doc, CURSOR *cur) {
	STRING *line = get_line(doc, cur);
	for (int i = cur->x + cur->horizontal_scroll - 1; i > 0; i--) {
		char ch = line->array[i];
		if (ch == ' ' || ch == '\t') {
			break;
		} else {
			erase_character(doc, cur);
		}
	}
	erase_character(doc, cur);
}

void increase_line_length(STRING *line, int increase) {
	char *new_array = (char *) malloc((line->length + increase) * sizeof(char));
	char *old_array = line->array;

	for (int i = 0; i < line->length; i++) {
		new_array[i] = old_array[i];
	}

	line->length += increase;
	line->array = new_array;
	free(old_array);
}

void insert_character(DOCUMENT *doc, CURSOR *cur, char ch) {
	STRING *line = get_line(doc, cur);
	increase_line_length(line, 1);

	char *array = line->array;

	for (int i = line->length - 1; i > cur->x + cur->horizontal_scroll; i--) {
		array[i] = array[i - 1];
	}

	array[cur->x + cur->horizontal_scroll] = ch;
}

void insert_new_line(DOCUMENT *doc, CURSOR *cur, STRING *line) {
	int y = cur->y + cur->vertical_scroll + 1;
	STRING **old_lines = doc->lines;
	STRING **new_lines = (STRING **) malloc((doc->length + 1) * sizeof(STRING *));

	if (y < doc->length) {
		int index = 0;
		for (int i = 0; i < doc->length; i++) {
			if (index == y) {
				new_lines[index++] = line; 
				i--;
			} else {
				new_lines[index++] = old_lines[i];
			}
		}
	} else {
		for (int i = 0; i < doc->length; i++) {
			new_lines[i] = old_lines[i];
		}
		new_lines[doc->length] = line;
	}

	doc->lines = new_lines;
	doc->length++;

	free(old_lines);
}

STRING * crop_line(DOCUMENT *doc, CURSOR *cur) {
	STRING *line = get_line(doc, cur);
	STRING *new_line = (STRING *) malloc(sizeof(STRING));
	new_line->length = line->length - (cur->x + cur->horizontal_scroll);
	new_line->array = (char *) malloc(new_line->length * sizeof(char));

	for (int i = 0; i < new_line->length; i++) {
		new_line->array[i] = line->array[i + cur->x + cur->horizontal_scroll];
	}

	line->length -= new_line->length;
	return new_line;
}

char s_equals(char *input, char *compare) {
	//int compare = strcmp(string1, string2);
	int index = 0;
	while (TRUE) {
		if (input[index] != compare[index]) {
			return FALSE;
		} else if (input[index] == 0) {
			return TRUE;
		}
		index++;
	}
}

/**
*** Command key combinations for ChEdit
*** Returns: 0 - No key combination found
***          1 - Key combination found but no document edit took place
***          2 - Key combination found and document edit took place
**/
int process_command(DOCUMENT *doc, CURSOR *cur, char *ch) {
	if (s_equals(ch, "\x1B[D")) {//LEFT
		decrement_x(cur, FALSE);
		return 1;
	} else if (s_equals(ch, "\x1B[C")) { //RIGHT
		increment_x(doc, cur, FALSE);
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
	} else if (s_equals(ch, "\x1B[5~")) {//PGUP
		cur->y = 0;
		cur->vertical_scroll = 0;
		cur->x = 0;
		cur->horizontal_scroll = 0;
		return 1;
	} else if (s_equals(ch, "\x1B[6~")) {//PGDN
		if (doc->length - 1 < cur->max_window_y) {
			cur->y = doc->length - 1;
		} else {
			cur->vertical_scroll = doc->length - cur->max_window_y - 1;
			cur->y = cur->max_window_y;
		}
		seek_line_end(doc, cur);
		return 1;
	} else if (s_equals(ch, "\x1B[3~")) {//DELETE
		delete_character(doc, cur);
		return 2;
	} else if (s_equals(ch, "\x1B\x7F") || s_equals(ch, "\x08")) {//CTRL+BACKSPACE
		erase_word(doc, cur);
		return 2;
	} else if (s_equals(ch, "\x7F\x00")) {//BACKSPACE
		erase_character(doc, cur);
		return 2;
	} else if (s_equals(ch, "\x1B[B\x1B[B")) {//SCROLL DOWN
		increment_y(doc, cur);
		return 1;
	} else if (s_equals(ch, "\x1B[A\x1B[A")) {//SCROLL UP
		decrement_y(doc, cur);
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

int process_text(DOCUMENT *doc, CURSOR *cur, char *ch, int length) {
	if (length != 0 && ch[0] != ERR && ch[0] != 0) {
		int command_result = process_command(doc, cur, ch);
		if (command_result == 1) {
			reset_cursor_highlight(cur);
			return 1;
		} else if (command_result == 2) {
			return 2;
		} else {
			if (ch[0] == '\t') { //TAB
				insert_character(doc, cur, '\t');
				increment_x(doc, cur, FALSE);
			} else if (ch[0] == '\n' || ch[0] == '\r') {//ENTER
				STRING *new_line = crop_line(doc, cur);
				insert_new_line(doc, cur, new_line);
				increment_y(doc, cur);
				cur->x = 0;
			} else if (ch[0] >= 32 && ch[0] <= 126) {//A standard printable character
				insert_character(doc, cur, ch[0]);
				increment_x(doc, cur, FALSE);
			} else {
				return 0;
			}
			return 2;
		}
	}
	return 0;
}

void write_file(char * directory, DOCUMENT *doc) {
	FILE *f;
	f = fopen(directory, "w");
	fprintf(f, "");
	fclose(f);
	f = fopen(directory, "a");

	for (int i = 0; i < doc->length; i++) {
		STRING *line = doc->lines[i];
		for (int x = 0; x < line->length; x++) {
			char ch = line->array[x];
			if (ch == 0) {
				break;
			}

			fprintf(f, "%c", ch);
		}
		if (i < doc->length - 1) {
			fprintf(f, "%c", '\n');
		}
	}

	fclose(f);
}

char * read_file(char *directory, int *length_of_file) {
	FILE *f;
	char *file_buffer;
	f = fopen(directory, "r");

	if (f != NULL) {
		fseek(f, 0, SEEK_END);
		*length_of_file = ftell(f);
		rewind(f);

		file_buffer = (char *) malloc(*length_of_file * sizeof(char) + 1);

		int index = 0;
		char ch = fgetc(f);
		while (ch != EOF) {
			file_buffer[index++] = ch;
			ch = fgetc(f);
		}
		file_buffer[index] = 0;
	} else {
		*length_of_file = 0;
		file_buffer = (char *) malloc(*length_of_file * sizeof(char));
	}

	return file_buffer;
}

DOCUMENT * convert_to_document(char *file_buffer, int file_length) {
	int number_of_lines = 1;

	for (int i = 0; i < file_length; i++) {
		char ch = file_buffer[i];
		if (ch == '\n') {
			number_of_lines++;
		}
	}

	STRING **lines = (STRING **) malloc(number_of_lines * sizeof(STRING *));

	DOCUMENT *doc = (DOCUMENT *) malloc(sizeof(DOCUMENT));
	doc->lines = lines;
	doc->length = number_of_lines;

	int file_buffer_index = 0;
	for (int i = 0; i < number_of_lines; i++) {
		int line_length = 0;
		for (int x = file_buffer_index; x < file_length; x++) {
			char ch = file_buffer[x];
			if (ch == '\n') {
				break;
			} else {
				line_length++;
			}
		}

		STRING *line = (STRING *) malloc(sizeof(STRING));
		char * line_char = (char *) malloc(line_length * sizeof(char));
		line->array = line_char;
		line->length = line_length;
		doc->lines[i] = line;

		for (int x = 0; x < line_length; x++) {
			line_char[x] = file_buffer[x + file_buffer_index];
		}
		file_buffer_index += line_length + 1;
	}

	return doc;
}

void draw_line_numbers(WINDOW *window, CURSOR *cur, int doc_length) {
	wclear(window);
	wstandout(window);
	for (int i = 0; i <= cur->max_window_y && i < doc_length; i++) {
		mvwprintw(window, i, 0, "%04d", cur->vertical_scroll + i + 1);
	}
	wstandend(window);
	wnoutrefresh(window);
}

void s_lower_case(STRING *string) {

}

WINDOW * input_window(DOCUMENT *doc, CURSOR *cur, STRING *input) {
	WINDOW *input_win = newwin(1, cur->max_window_x, cur->max_window_y + 1, STRING_NUMBER_WIDTH);
	for (int i = 0; i < input->length; i++) {
		input->array[i] = 0;
	}
	wclear(input_win);
	wstandout(input_win);
	mvwhline(input_win, 0, 0, ' ', cur->max_window_x);
	return input_win;
}

void goto_line(DOCUMENT *doc, CURSOR *cur, int line_number) {
	if (line_number > doc->length) {
		line_number = doc->length - 1;
	} else if (line_number < 1) {
		line_number = 1;
	}
	if (line_number < doc->length - cur->max_window_y) {
		cur->y = 0;
		cur->vertical_scroll = line_number - 1;
	} else {
		cur->vertical_scroll = doc->length - cur->max_window_y - 1;
		cur->y = line_number - cur->vertical_scroll;
	}
	reset_cursor_highlight(cur);
}

void find(DOCUMENT *doc, CURSOR *cur) {
	int index = 0;
	STRING *input = (STRING *) malloc(sizeof(STRING));
	input->array = (char *) malloc(50 * sizeof(char));
	input->length = FIND_STRING_LENGTH;
	WINDOW *find_win = input_window(doc, cur, input);
	mvwprintw(find_win, 0, 0, " Enter a string: ");
	wrefresh(find_win);
	while (TRUE) {
		char ch = getch();
		if (ch != '\n') {
			if (index <= 50 && ch != -1 && ch >= ' ' && ch <= '~') {
				mvwaddch(find_win, 0, 17 + index, ch);
				input->array[index++] = ch;
			} else if (ch == 8 || ch == 127) {
				if (index > 0) {
					mvwaddch(find_win, 0, 17 + --index, ' ');
					input->array[index] = 0;
				}
			}
			wrefresh(find_win);
		} else {
			break;
		}
	}
	wstandend(find_win);
	int line_pos = 0;
	int position = 0;
	int found;

	for (int y = 0; y < doc->length; y++) {
		line_pos = y;
		STRING *line = doc->lines[y];
		for (int x = 0; x < line->length; x++) {
			found = 1;
			position = x;
			for (int i = 0; i < index; i++) {
				if (line->array[x + i] != input->array[i]) {
					found = 0;
					break;
				}
			}
			if (found) {
				break;
			}
		} 
		if (found) {
			break;
		}
	}

	if (found) {
		goto_line(doc, cur, line_pos + 1);
	}

	reset_cursor_highlight(cur);
	free(input->array);
	free(input);
}

void show_goto_line(DOCUMENT *doc, CURSOR *cur) {
	int index = 0;
	STRING *input = (STRING *) malloc(sizeof(STRING));
	input->array = (char *) malloc(6 * sizeof(char));
	input->length = GOTO_STRING_LENGTH;
	WINDOW *line_win = input_window(doc, cur, input);
	mvwprintw(line_win, 0, 0, " GOTO: ");
	wrefresh(line_win);
	while (TRUE) {
		char ch = getch();
		if (ch != '\n') {
			if (index <= 5 && ch != -1 && ch >= '0' && ch <= '9') {
				mvwaddch(line_win, 0, 7 + index, ch);
				input->array[index++] = ch;
			} else if (ch == 8 || ch == 127) {
				if (index > 0) {
					mvwaddch(line_win, 0, 7 + --index, ' ');
					input->array[index] = 0;
				}
			}
			wrefresh(line_win);
		} else {
			break;
		}
	}
	wstandend(line_win);

	int line_number = atoi(input->array);
	goto_line(doc, cur, line_number);
	free(input->array);
	free(input);
}

void intHandler(int interr) {
	interrupt = interr;
}

void disable_interrupts() {
	signal(SIGINT, intHandler);
	signal(SIGALRM, intHandler);
	signal(SIGHUP, intHandler);
}

int main(int argc, char *argv[]) {
	char *savefile;
	char savepath[1024];
	char unsaved_changes = FALSE;
	char line_number_redraw = FALSE;
	char key_pressed = FALSE;
	char show_debug = FALSE;

	struct timespec delay;
	delay.tv_sec = 0;
	delay.tv_nsec = 16300000L;

	int max_x = 0, max_y = 0, file_length = 0;
	char ch = -1;
	char *chars = (char *) malloc((CHARACTER_INPUT_ARR_LENGTH + 1) * sizeof(char));
	int length = 1;
	chars[0] = -1;
	int ensure_display = 5;

	disable_interrupts();

	getcwd(savepath, 1024);

	if (argc != 2) {
		printf("Invalid arguments! Run: chedit <file to edit>\n");
		return 1;
	} else {
		savefile = argv[1];
	}

	char directory[1280];
	sprintf(directory, "%s/%s", savepath, savefile);

	char *file_buffer = read_file(directory, &file_length);

	curses_setup();

	getmaxyx(stdscr, max_y, max_x);
	WINDOW *root = newwin(max_y, max_x, 1, STRING_NUMBER_WIDTH);
	WINDOW *title_bar = newwin(1, max_x, 0, 0);
	WINDOW *line_numbers = newwin(max_y - 1, STRING_NUMBER_WIDTH, 1, 0);
	WINDOW *diag_win = newwin(DEBUG_STRINGS, DEBUG_COLUMNS, max_y - DEBUG_STRINGS, max_x - DEBUG_COLUMNS);

	clear();
	refresh();

	if (max_x < 1 || max_y < 2) {
		return 2;
	}

	CURSOR cur = {0, 0, 0, 0, 0, 0, 0, max_y - 2, max_x - 1 - STRING_NUMBER_WIDTH, 1, 0};

	DOCUMENT *doc = convert_to_document(file_buffer, file_length);

	draw_title_bar(title_bar, max_x, savepath, savefile, unsaved_changes);
	draw_text(root, doc, &cur);
	draw_line_numbers(line_numbers, &cur, doc->length);

	while (1) {
		char cursor_redraw = tick_cursor(&cur);
		if (key_pressed) {
			if ((chars[0] == 23 || s_equals(chars, "\x1Bs")) && unsaved_changes) { //CTRL+w
				write_file(directory, doc);
				unsaved_changes = FALSE;
				draw_title_bar(title_bar, max_x, savepath, savefile, unsaved_changes);
			} else if (chars[0] == 7) { //CTRL+g
				show_goto_line(doc, &cur);
				chars[0] = -1;
				line_number_redraw = TRUE;
			} else if (chars[0] == 4) {
				show_debug = !show_debug;
			} else if (s_equals(chars, "\x1B[24~") || s_equals(chars, "\x1B\x1B")) {
				break;
			} else if (s_equals(chars, "\x06")) {
				find(doc, &cur);
				chars[0] = -1;
				line_number_redraw = TRUE;
			}
		}

		int text_result = process_text(doc, &cur, chars, length);
		unsaved_changes = text_result == 2 || unsaved_changes;

		if (cursor_redraw || text_result > 0 || line_number_redraw) {
			draw_text(root, doc, &cur);
		}

		if (text_result > 0 || line_number_redraw) {
			draw_line_numbers(line_numbers, &cur, doc->length);
			draw_title_bar(title_bar, max_x, savepath, savefile, unsaved_changes);
		}

		if (show_debug) {
			draw_diag_win(diag_win, interrupt, cur.max_window_x, cur.max_window_y, cur.y, cur.x, chars);
		}
		doupdate();

		nanosleep(&delay, NULL);

		length = 0;
		ch = getch();
		key_pressed = FALSE;
		while (ch != -1) {
			key_pressed = TRUE;
			if (length < CHARACTER_INPUT_ARR_LENGTH) {
				chars[length++] = ch;
			}
			ch = getch();
		}
		if (key_pressed) {
			chars[length] = 0;
		}
	}

	endwin();

	return 0;
}
