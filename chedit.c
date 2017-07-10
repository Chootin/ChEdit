#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define DEBUG_LINES (5)
#define DEBUG_COLUMNS (12)
#define TAB_WIDTH (4)
#define CHARACTER_INPUT_ARR_LENGTH (6)
#define LINE_NUMBER_WIDTH (3)

struct line {
	char *array;
	int length;
};

struct document {
	struct line **lines;
	int length;
};

struct cursor {
	int y;
	int x;
	int vertical_scroll;
	int horizontal_scroll;
	int max_window_y;
	int max_window_x;
	int highlight;
	int highlight_tick;
};

void curses_setup() {
	initscr();
	noecho();
	curs_set(0);
	nodelay(stdscr, true);
}

void draw_diag_win(WINDOW *diag_win, int max_y, int max_x, int cur_y, int cur_x, char ch) {
	mvwprintw(diag_win, 0, 0, "Debug");
	mvwprintw(diag_win, 1, 0, "Width: %d", max_x);
	mvwprintw(diag_win, 2, 0, "Height: %d", max_y);
	mvwprintw(diag_win, 3, 0, "X: %d, Y: %d", cur_x, cur_y);
	mvwprintw(diag_win, 4, 0, "ch: %d", ch);
}

void draw_title_bar(WINDOW *window, int max_x) {
	wstandout(window);
	mvwhline(window, 0, 0, ' ', max_x);
	mvwprintw(window, 0, max_x / 2 - 3, "ChEdit");
	wstandend(window);
}

void tick_cursor(struct cursor *cur) {
	if (cur->highlight_tick++ == 25) {
		cur->highlight_tick = 0;
		if (cur->highlight) {
			cur->highlight = 0;
		} else {
			cur->highlight = 1;
		}
	}
}

char cursor_active_here(struct cursor *cur, int y, int x) {
	return cur->highlight && cur->y == y && cur->x + cur->horizontal_scroll == x;
}

struct line * get_line(struct document *doc, struct cursor *cur) {
	return doc->lines[cur->y + cur->vertical_scroll];
}

int get_tab_offset(struct document *doc, struct cursor *cur) {
	struct line *line = get_line(doc, cur);
	int tab_offset = 0;
	for (int x = cur->horizontal_scroll; x < cur->horizontal_scroll + cur->max_window_x; x++) {
		if (line->array[x] == '\t') {
			tab_offset += TAB_WIDTH - 1;
		}
	}
	return tab_offset;
}

void draw_text(WINDOW *window, struct document *doc, struct cursor *cur) {
	struct line **lines = doc->lines;

	for (int y = 0; y < doc->length - cur->vertical_scroll; y++) {
		int draw_x = 0;
		int start_x = 0;
		int tab_offset = 0;
		struct line *line = lines[y + cur->vertical_scroll];
		if (y == cur->y) {
			start_x = cur->horizontal_scroll;
		}
		for (int x = start_x; x < line->length; x++) {
			char ch = line->array[x];
			if (cursor_active_here(cur, y, x)) {
				wstandout(window);
			}
			if (ch == '\t') {
				for (int i = 0; i < TAB_WIDTH; i++) {
					mvwaddch(window, y, draw_x + tab_offset, ' ');
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
}

void reset_cursor_highlight(struct cursor *cur) {
	cur->highlight = 1;
	cur->highlight_tick = 0;
}

void seek_line_end(struct document *doc, struct cursor *cur) {
	struct line *line = get_line(doc, cur);

	if (cur->x + cur->horizontal_scroll > line->length) {
		if (line->length > cur->max_window_x) {
			cur->horizontal_scroll = line->length - cur->max_window_x;
			cur->x = line->length - cur->horizontal_scroll;
		} else {
			cur->x = line->length;
		}
		if (cur->x < 0) {
			cur->x = 0;
		}
	}
}

void decrement_y(struct document *doc, struct cursor *cur) {
	if (cur->y > 0) {
		cur->y--;
	} else if (cur->vertical_scroll > 0) {
		cur->vertical_scroll--;
	}

	seek_line_end(doc, cur);

	reset_cursor_highlight(cur);
}

void increment_y(struct document *doc, struct cursor *cur) {
	if (cur->y + 1 < doc->length) {
		if (cur->y < cur->max_window_y) {
			cur->y++;
		} else {
			if (cur->vertical_scroll < doc->length - cur->max_window_y - 1) {
				cur->vertical_scroll++;
			}
		}
	}

	seek_line_end(doc, cur);

	reset_cursor_highlight(cur);
}

void decrement_y_para(struct document *doc, struct cursor *cur) {
	decrement_y(doc, cur);
	for (int i = cur->y + cur->vertical_scroll - 1; i >= 0; i--) {
		if (doc->lines[i]->length > 0) {
			decrement_y(doc, cur);
		} else {
			break;
		}
	}
}

void increment_y_para(struct document *doc, struct cursor *cur) {
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

void decrement_x(struct cursor *cur) {
	if (cur->x > 0) {
		cur->x--;
	} else if (cur->horizontal_scroll > 0) {
		cur->horizontal_scroll--;
	}

	reset_cursor_highlight(cur);
}

void increment_x(struct document *doc, struct cursor *cur) {
	struct line *line = get_line(doc, cur);

	if (cur->x + cur->horizontal_scroll < line->length) {
		if (cur->x + get_tab_offset(doc, cur) >= cur->max_window_x) {
			cur->horizontal_scroll++;
		} else {
			cur->x++;
		}
	}

	reset_cursor_highlight(cur);
}

char get_cursor_char(struct document *doc, struct cursor *cur) {
	struct line *line = get_line(doc, cur);
	return line->array[cur->x + cur->horizontal_scroll];
}

void decrement_x_word(struct document *doc, struct cursor *cur) {
	decrement_x(cur);
	while (cur->x > 0 || cur->horizontal_scroll > 0) {
		decrement_x(cur);
		char ch = get_cursor_char(doc, cur);
		if (ch == ' ' || ch == '\t') {
			increment_x(doc, cur);
			break;
		}
	}
}

void increment_x_word(struct document *doc, struct cursor *cur) {
	struct line *line = get_line(doc, cur);
	while (cur->x + cur->horizontal_scroll < line->length) {
		increment_x(doc, cur);
		char ch = get_cursor_char(doc, cur);
		if (ch == ' ' || ch == '\t') {
			increment_x(doc, cur);
			break;
		}
	}
}

void delete_line(struct document *doc, struct cursor *cur) {
	int y = cur->y-- + cur->vertical_scroll;
	doc->length--;

	free(doc->lines[y]->array);
	free(doc->lines[y]);
	
	for (int i = y; i < doc->length; i++) {
		doc->lines[i] = doc->lines[i + 1];
	}
}

void append_line(struct line *line, struct line *append) {
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

void delete_character(struct document *doc, struct cursor *cur) {
	struct line *line = get_line(doc, cur);
	if (line->length > 0 && cur->x + cur->horizontal_scroll < line->length) {
		char *array = line->array; 
		for (int i = cur->x + cur->horizontal_scroll; i < line->length; i++) {
			array[i] = array[i + 1];
		}
		line->length--;
	} else if (cur->y + cur->vertical_scroll < doc->length - 1) {
		struct line *line = get_line(doc, cur);
		struct line *next_line = doc->lines[cur->y + cur->vertical_scroll + 1];
		append_line(line, next_line);
		cur->y++;
		delete_line(doc, cur);
	}
	reset_cursor_highlight(cur);
}

void erase_character(struct document *doc, struct cursor *cur) {
	if (cur->x + cur->horizontal_scroll == 0) {
		if (cur->y + cur->vertical_scroll > 0) {
			struct line *line = get_line(doc, cur);
			struct line *last_line = doc->lines[cur->y + cur->vertical_scroll - 1];
			int new_cur_x = last_line->length;
			if (line->length > 0) {
				append_line(last_line, line);
			}
			delete_line(doc, cur);
			cur->x = new_cur_x;
			reset_cursor_highlight(cur);
		}
	} else {
		decrement_x(cur);
		delete_character(doc, cur);
	}
}

void erase_word(struct document *doc, struct cursor *cur) {
	struct line *line = get_line(doc, cur);
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

void increase_line_length(struct line *line, int increase) {
	char *new_array = (char *) malloc((line->length + increase) * sizeof(char));
	char *old_array = line->array;

	for (int i = 0; i < line->length; i++) {
		new_array[i] = old_array[i];
	}

	line->length += increase;
	line->array = new_array;
	free(old_array);
}

void insert_character(struct document *doc, struct cursor *cur, char ch) {
	struct line *line = get_line(doc, cur);
	increase_line_length(line, 1);

	char *array = line->array;

	for (int i = line->length - 1; i > cur->x + cur->horizontal_scroll; i--) {
		array[i] = array[i - 1];
	}

	array[cur->x + cur->horizontal_scroll] = ch;
}

void insert_new_line(struct document *doc, struct cursor *cur, struct line *line) {
	int y = cur->y + cur->vertical_scroll + 1;
	struct line **old_lines = doc->lines;
	struct line **new_lines = (struct line **) malloc((doc->length + 1) * sizeof(struct line *));

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

struct line * crop_line(struct document *doc, struct cursor *cur) {
	struct line *line = get_line(doc, cur);
	struct line *new_line = (struct line *) malloc(sizeof(struct line));
	new_line->length = line->length - cur->x;
	new_line->array = (char *) malloc(new_line->length * sizeof(char));

	for (int i = 0; i < new_line->length; i++) {
		new_line->array[i] = line->array[i + cur->x];
	}

	line->length -= new_line->length;
	return new_line;
}

void process_text(struct document *doc, struct cursor *cur, char *ch, int length) {
	if (length == 0 || ch[0] == ERR || ch[0] == 0) {
	} else if (ch[0] == 27) {
		if (ch[1] == 91 || ch[1] == 79) {
			if (ch[2] == 51) {
				delete_character(doc, cur);	
			} else if (ch[2] == 53) {
				cur->y = 0;
				cur->vertical_scroll = 0;
				seek_line_end(doc, cur);
			} else if (ch[2] == 54) {
				if (doc->length - 1 < cur->max_window_y) {
					cur->y = doc->length - 1;
				} else {
					cur->vertical_scroll = doc->length - cur->max_window_y - 1;
					cur->y = cur->max_window_y;
				}
				seek_line_end(doc, cur);
			} else if (ch[2] == 72) {
				cur->x = 0;
				cur->horizontal_scroll = 0;
			} else if (ch[2] == 70) {
				struct line *line = get_line(doc, cur);
				if (line->length < cur->max_window_x) {
					cur->x = line->length;
				} else {
					cur->horizontal_scroll = line->length - cur->max_window_x;
					cur->x = line->length - cur->horizontal_scroll;
				}
			} else if (ch[2] == 49) {
				switch (ch[5]) {
					case 68:
						decrement_x_word(doc, cur);
						break;
					case 67:
						increment_x_word(doc, cur);
						break;
					case 65:
						decrement_y_para(doc, cur);
						break;
					case 66:
						increment_y_para(doc, cur);
						break;
				}
			} else {
				switch (ch[2]) {
					case 'A': //UP
						decrement_y(doc, cur);
						break;
					case 'B': //DOWN
						increment_y(doc, cur);
						break;
					case 'C': //RIGHT
						increment_x(doc, cur);
						break;
					case 'D': //LEFT
						decrement_x(cur);
						break;
				}
			}
			reset_cursor_highlight(cur);
		} else if (ch[1] == 127) {
			erase_word(doc, cur);
		}
	} else {
		if (ch[0] == 9) { //Tab key
			insert_character(doc, cur, '\t');
			increment_x(doc, cur);
		} else if (ch[0] == 8 || ch[0] == 127) { //Backspace
			erase_character(doc, cur);
		} else if (ch[0] == '\n' || ch[0] == '\r') {
			struct line *new_line = crop_line(doc, cur);
			insert_new_line(doc, cur, new_line);

			increment_y(doc, cur);
			cur->x = 0;
		} else if (ch[0] >= 32 && ch[0] <= 126) {
			insert_character(doc, cur, ch[0]);
			increment_x(doc, cur);
		}
	}
}

void write_file(char * directory, struct document *doc) {
	FILE *f;
	f = fopen(directory, "w");
	fprintf(f, "");
	fclose(f);
	f = fopen(directory, "a");

	for (int i = 0; i < doc->length; i++) {
		struct line *line = doc->lines[i];
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
		file_buffer = (char *) malloc(*length_of_file *	 sizeof(char));
	}

	return file_buffer;
}

struct document * convert_to_document(char *file_buffer, int file_length) {
	int number_of_lines = 1;

	for (int i = 0; i < file_length; i++) {
		char ch = file_buffer[i];
		if (ch == '\n') {
			number_of_lines++;
		}
	}

	struct line **lines = (struct line **) malloc(number_of_lines * sizeof(struct line *));

	struct document *doc = (struct document *) malloc(sizeof(struct document));
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

		struct line *line = (struct line *) malloc(sizeof(struct line));
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

void draw_line_numbers(WINDOW *window, struct cursor *cur) {
	wstandout(window);
	for (int i = 0; i <= cur->max_window_y; i++) {
		mvwprintw(window, i, 0, "%03d", cur->vertical_scroll + i + 1);
	}
	wstandend(window);
}

int main(int argc, char *argv[]) {
	char *savefile;
	char savepath[1024];

	struct timespec delay;
	delay.tv_sec = 0;
	//delay.tv_nsec = 16300000L;
	delay.tv_nsec = 32600000L;

	int max_x = 0, max_y = 0, file_length = 0;
	char ch = -1;
	char *chars = (char *) malloc(CHARACTER_INPUT_ARR_LENGTH * sizeof(char));
	int length = 0;

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
	WINDOW *root = newwin(max_y, max_x, 1, LINE_NUMBER_WIDTH);
	WINDOW *title_bar = newwin(1, max_x, 0, 0);
	WINDOW *line_numbers = newwin(max_y - 1, LINE_NUMBER_WIDTH, 1, 0);
	WINDOW *diag_win = newwin(DEBUG_LINES, DEBUG_COLUMNS, max_y - DEBUG_LINES, max_x - DEBUG_COLUMNS);

	if (max_x < 1 || max_y < 2) {
		return 2;
	}

	struct cursor cur = {0, 0, 0, 0, max_y - 2, max_x - 1 - LINE_NUMBER_WIDTH, 1, 0};

	struct document *doc = convert_to_document(file_buffer, file_length);

	while (1) {
		if (chars[0] == 23) { //CTRL+w
			break;
		}
		process_text(doc, &cur, chars, length);		

		wclear(root);
		draw_text(root, doc, &cur);

		tick_cursor(&cur);

		wnoutrefresh(root);

		wclear(title_bar);
		draw_title_bar(title_bar, max_x);
		wnoutrefresh(title_bar);

		wclear(line_numbers);
		draw_line_numbers(line_numbers, &cur);
		wnoutrefresh(line_numbers);

		wclear(diag_win);
		draw_diag_win(diag_win, cur.max_window_x, cur.max_window_y, cur.y, cur.x, chars[0]);
		wnoutrefresh(diag_win);
		doupdate();

		length = 0;

		ch = getch();
		while (ch != -1 && length < CHARACTER_INPUT_ARR_LENGTH) {
			chars[length++] = ch;
			ch = getch();
		}
		nanosleep(&delay, NULL);
	}

	endwin();
	printf("Writing output to %s\n", directory);

	write_file(directory, doc);

	return 0;
}
