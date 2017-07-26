#include "document.h"
#include <stdlib.h>
#include "constants.h"

void delete_line(DOCUMENT *doc, CURSOR *cur) {
	int y = cur->y-- + cur->vertical_scroll;
	doc->length--;

	free(doc->lines[y]->array);
	free(doc->lines[y]);
	
	for (int i = y; i < doc->length; i++) {
		doc->lines[i] = doc->lines[i + 1];
	}
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
		append_string(line, next_line);
		cur->y++;
		delete_line(doc, cur);
	}
}

void erase_character(DOCUMENT *doc, CURSOR *cur) {
	if (cur->x + cur->horizontal_scroll == 0) {
		if (cur->y + cur->vertical_scroll > 0) {
			STRING *line = get_line(doc, cur);
			STRING *last_line = doc->lines[cur->y + cur->vertical_scroll - 1];
			int new_cur_x = last_line->length;
			if (line->length > 0) {
				append_string(last_line, line);
			}
			delete_line(doc, cur);
			if (new_cur_x > cur->max_window_x) {
				cur->x = cur->max_window_x;
				cur->horizontal_scroll = new_cur_x - cur->x;
			} else {
				cur->x = new_cur_x;
			}
		}
	} else {
		decrement_x(doc, cur, false);
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

void insert_character(DOCUMENT *doc, CURSOR *cur, char ch) {
	STRING *line = get_line(doc, cur);
	increase_string_length(line, 1);

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

DOCUMENT * text_to_document(char *file_buffer, int file_length) {
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

STRING * get_line(DOCUMENT *doc, CURSOR *cur) {
	return doc->lines[cur->y + cur->vertical_scroll];
}

