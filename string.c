#include "string.h"
#include <stdlib.h>
#include "constants.h"

void increase_string_length(STRING *string, int increase) {
	char *new_array = (char *) malloc((string->length + increase) * sizeof(char));
	char *old_array = string->array;

	for (int i = 0; i < string->length; i++) {
		new_array[i] = old_array[i];
	}

	string->length += increase;
	string->array = new_array;
	free(old_array);
}

//was cur->x + cur->horizontal_scroll
STRING * crop_string(STRING *string, int n) {
	STRING *new_string = (STRING *) malloc(sizeof(STRING));
	new_string->length = string->length - n;
	new_string->array = (char *) malloc(new_string->length * sizeof(char));

	for (int i = 0; i < new_string->length; i++) {
		new_string->array[i] = string->array[i + n];
	}

	string->length -= new_string->length;
	return new_string;
}

char s_equals(char *input, char *compare) {
	//int compare = strcmp(string1, string2);
	int index = 0;
	while (true) {
		if (input[index] != compare[index]) {
			return false;
		} else if (input[index] == 0) {
			return true;
		}
		index++;
	}
}

char to_lowercase(char ch) {
	if (ch >= 'A' && ch <= 'Z') {
		return ch + 32; 
	}
	return ch;
}

char is_empty(STRING *string) {
	for (int i = 0; i < string->length; i++) {
		char ch = string->array[i];
		if (ch != ' ' && ch != '\t' && ch != 0) {
			return false;
		}
	}
	return true;
}