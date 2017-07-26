#ifndef string_h
#define string_h

typedef struct STRING STRING;

struct STRING {
        char *array;
        int length;
};

/**
* Increase the length of the string's char array by increase amount.
**/
void increase_string_length(STRING *string, int increase);

/**
* Remove characters on and after the nth place from string and return the new string.
**/
STRING * crop_string(STRING *string, int n);

/**
* Append the content of string - append to the string - line. Append will be deleted.
**/
void append_string(STRING *line, STRING *append);

/**
* Returns true if the input and compare arrays are equal.
**/
char s_equals(char *input, char *compare);

/**
* If the char is from A to Z, return the corresponding a to z character. Otherwise return the character as is. 
**/
char to_lowercase(char ch);

/**
* Returns true if a string is of length 0 or only contains ' ' or '\t'.
**/
char is_empty(STRING *string);

#endif
