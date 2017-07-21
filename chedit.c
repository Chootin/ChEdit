#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "define.h"
#include "chedit.h"

#define DEBUG_STRINGS (6)
#define DEBUG_COLUMNS (19)
#define CHARACTER_INPUT_ARR_LENGTH (6)
#define LINE_NUMBER_WIDTH (4)
#define CURSOR_SPEED (25)
#define FIND_STRING_LENGTH (50)
#define GOTO_STRING_LENGTH (10)

volatile int interrupt = 0;

void curses_setup() {
	initscr();
	noecho();
}

void draw_diag_win(WINDOW *diag_win, int interrupt, int max_y, int max_x, int cur_y, int cur_x, char *ch) {
	wclear(diag_win);
	mvwprintw(diag_win, 0, 0, "Debug");
	mvwprintw(diag_win, 1, 0, "Interrupt: %d", interrupt);
	mvwprintw(diag_win, 2, 0, "Width: %d", max_x);
	mvwprintw(diag_win, 3, 0, "Height: %d", max_y);
	mvwprintw(diag_win, 4, 0, "X: %d, Y: %d", cur_x, cur_y);
	//mvwprintw(diag_win, 5, 0, ch);
	mvwprintw(diag_win, 5, 0, "%d %d %d %d %d %d %d", ch[0], ch[1], ch[2], ch[3], ch[4], ch[5], ch[6]);
	wnoutrefresh(diag_win);
}

void draw_title_bar(WINDOW *window, int max_x, char *savepath, char *savefile, char unsaved_changes) {
	wclear(window);
	wstandout(window);
	mvwhline(window, 0, 0, ' ', max_x);
	if (unsaved_changes) {
		mvwprintw(window, 0, LINE_NUMBER_WIDTH, "ChEdit: %s/%s*", savepath, savefile);
	} else {
		mvwprintw(window, 0, LINE_NUMBER_WIDTH, "ChEdit: %s/%s", savepath, savefile);
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
	}
	wnoutrefresh(window);
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

void draw_line_numbers(WINDOW *window, CURSOR *cur, int doc_length) {
	wclear(window);
	wstandout(window);
	for (int i = 0; i <= cur->max_window_y && i < doc_length; i++) {
		mvwprintw(window, i, 0, "%04d", cur->vertical_scroll + i + 1);
	}
	wstandend(window);
	wnoutrefresh(window);
}

void input_window(CURSOR *cur, STRING *input, char *explain, int explain_size, char (*input_check)(char), char immediate) {
	WINDOW *input_win = newwin(1, cur->max_window_x, cur->max_window_y + 1, LINE_NUMBER_WIDTH);
	for (int i = 0; i < input->length; i++) {
		input->array[i] = 0;
	}
	wclear(input_win);
	wstandout(input_win);
	mvwhline(input_win, 0, 0, ' ', cur->max_window_x);
	mvwprintw(input_win, 0, 0, explain);
	wrefresh(input_win);
	int index = 0;

	while (true) {
		char ch = getch();
		/*if (getch() != -1) {//Filter out the command inputs
			while (getch() != -1) {}
			continue;
		}*/
		if (ch != '\n') {
			if (ch == 8 || ch == 127) {
				if (index > 0) {
					mvwaddch(input_win, 0, explain_size + --index, ' ');
					input->array[index] = 0;
				}
			} else if (index < input->length && ch != -1 && input_check(ch)) {
				mvwaddch(input_win, 0, explain_size + index, ch);
				input->array[index++] = ch;

				if (immediate && index == input->length) {
					break;
				}
			}
			wrefresh(input_win);
		} else {
			break;
		}
	}
	wstandend(input_win);
	input->length = index;
}

void find(DOCUMENT *doc, CURSOR *cur) {
	STRING *input = (STRING *) malloc(sizeof(STRING));
	input->array = (char *) malloc(FIND_STRING_LENGTH * sizeof(char));
	input->length = FIND_STRING_LENGTH;
	input_window(cur, input, " Enter a search term: ", 22, &is_printable, false);

	if (input->length != 0) {
		int line_pos = 0;
		int position = 0;
		char found = 0;
	
		for (int y = cur->y + cur->vertical_scroll + 1; y < doc->length; y++) {
			line_pos = y;
			STRING *line = doc->lines[y];
			for (int x = 0; x < line->length - input->length; x++) {
				found = 1;
				position = x;
				for (int i = 0; i < input->length; i++) {
					if (to_lowercase(line->array[x + i]) != to_lowercase(input->array[i])) {
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
	}

	free(input->array);
	free(input);
}

void show_goto_line(DOCUMENT *doc, CURSOR *cur) {
	STRING *input = (STRING *) malloc(sizeof(STRING));
	input->array = (char *) malloc(6 * sizeof(char));
	input->length = GOTO_STRING_LENGTH;
	input_window(cur, input, " GOTO: ", 7, &is_numeric, false);

	int line_number = atoi(input->array);
	goto_line(doc, cur, line_number);
	free(input->array);
	free(input);
}

char show_exit_warning(CURSOR *cur) {
	STRING *input = (STRING *) malloc(sizeof(STRING));
	input->array = (char *) malloc(sizeof(char));
	input->length = 1;
	input_window(cur, input, " Press ESC again to close, RETURN to cancel...", 46, &is_esc, true);

	char should_exit = input->array[0] == '\x1B';

	free(input->array);
	free(input);

	return should_exit;
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
	char unsaved_changes = false;
	char line_number_redraw = false;
	char key_pressed = false;
	char show_debug = false;

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
	WINDOW *text_area = newwin(max_y, max_x, 1, LINE_NUMBER_WIDTH);
	WINDOW *title_bar = newwin(1, max_x, 0, 0);
	WINDOW *line_numbers = newwin(max_y - 1, LINE_NUMBER_WIDTH, 1, 0);
	WINDOW *diag_win = newwin(DEBUG_STRINGS, DEBUG_COLUMNS, max_y - DEBUG_STRINGS, max_x - DEBUG_COLUMNS);

	clear();
	refresh();

	if (max_x < 1 || max_y < 2) {
		return 2;
	}

	CURSOR cur = {0, 0, 0, 0, max_y - 2, max_x - 1 - LINE_NUMBER_WIDTH};

	DOCUMENT *doc = text_to_document(file_buffer, file_length);

	draw_title_bar(title_bar, max_x, savepath, savefile, unsaved_changes);
	draw_text(text_area, doc, &cur);
	draw_line_numbers(line_numbers, &cur, doc->length);

	while (1) {
		int input_result = process_command(doc, &cur, chars);
		if (key_pressed) {
			if ((chars[0] == 23 || s_equals(chars, "\x1Bs")) && unsaved_changes) { //CTRL+w
				write_file(directory, doc);
				unsaved_changes = false;
				draw_title_bar(title_bar, max_x, savepath, savefile, unsaved_changes);
			} else if (chars[0] == 7) { //CTRL+g
				show_goto_line(doc, &cur);
				chars[0] = -1;
				line_number_redraw = true;
			} else if (chars[0] == 4) {
				show_debug = !show_debug;
			} else if (s_equals(chars, "\x1B")) {
				if (show_exit_warning(&cur)) {
					break;
				}
			} else if (s_equals(chars, "\x06")) {
				find(doc, &cur);
				chars[0] = -1;
				line_number_redraw = true;
			}
		}

		if (input_result == 0) {
			input_result = process_text(doc, &cur, chars, length);
		}
		unsaved_changes = input_result == 2 || unsaved_changes;

		if (input_result > 0 || line_number_redraw) {
			draw_text(text_area, doc, &cur);
		}

		if (input_result > 0 || line_number_redraw) {
			draw_line_numbers(line_numbers, &cur, doc->length);
			draw_title_bar(title_bar, max_x, savepath, savefile, unsaved_changes);
		}

		if (show_debug) {
			draw_diag_win(diag_win, interrupt, cur.max_window_y, cur.max_window_x, cur.y, cur.x, chars);
		}
		doupdate();
		move(cur.y + 1, cur.x + get_tab_offset(doc, &cur) + LINE_NUMBER_WIDTH);

		length = 0;
		nodelay(stdscr, false);
		ch = getch();
		nodelay(stdscr, true);
		while (ch != -1) {
			key_pressed = true;
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
