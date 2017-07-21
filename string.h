#ifndef string_h
#define string_h

typedef struct STRING STRING;

struct STRING {
        char *array;
        int length;
};

void increase_string_length(STRING *string, int increase);
STRING * crop_string(STRING *string, int n);
char s_equals(char *input, char *compare);
char to_lowercase(char ch);

#endif
