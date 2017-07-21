#ifndef document_h
#define document_h

#include "string.h"
#include "cursor.h"
#include <stdlib.h>

typedef struct DOCUMENT DOCUMENT;
typedef struct CURSOR CURSOR;
typedef struct STRING STRING;

struct DOCUMENT {
        STRING **lines;
        int length;
};

void delete_line(DOCUMENT *doc, CURSOR *cur);
void append_line(STRING *line, STRING *append);
void delete_character(DOCUMENT *doc, CURSOR *cur);
void erase_character(DOCUMENT *doc, CURSOR *cur);
void erase_word(DOCUMENT *doc, CURSOR *cur);
void insert_character(DOCUMENT *doc, CURSOR *cur, char ch);
void insert_new_line(DOCUMENT *doc, CURSOR *cur, STRING *line);
STRING * get_line(DOCUMENT *doc, CURSOR *cur);
DOCUMENT * text_to_document(char *file_buffer, int file_length);

#endif
