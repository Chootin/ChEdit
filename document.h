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

/**
* Delete the line at cursor - cur position in the document - doc.
**/
void delete_line(DOCUMENT *doc, CURSOR *cur);

/**
* Remove the character at the current position of the cursor - cur in the document - doc.
* If the cursor is at the end of a line, append the next line to the current line.
* The cursor will stay in the same position in the document.
**/
void delete_character(DOCUMENT *doc, CURSOR *cur);

/**
* Remove the character at the position immediately to the left of the cursor - cur in the document - doc.
* If the cursor is at the start of a line, append that line to the previous line in the document.
* If the line has been appended, move the cursor to the end of the previous line before the append action.
* Otherwise, the cursor will stay in the same position in the document.
**/
void erase_character(DOCUMENT *doc, CURSOR *cur);

/**
* Remove a whole word from the document - doc behind the cursor - cur.
**/
void erase_word(DOCUMENT *doc, CURSOR *cur);

/**
* Add a character to the current line in document - doc at the current cursor position.
* Move all characters after the cursor on the current line 1 position to the right.
**/
void insert_character(DOCUMENT *doc, CURSOR *cur, char ch);

/**
* Add a new line at the position of cursor - cur position. If the cursor is not at the end of a line,
* crop the characters after the cursor on the current line and they are appended to the new line.
* The cursor will move to the start of the new line.
**/
void insert_new_line(DOCUMENT *doc, CURSOR *cur, STRING *line);

/**
* Get a line from the document - doc at the current position of the cursor - cur.
**/
STRING * get_line(DOCUMENT *doc, CURSOR *cur);

/**
* Create a new document from char array - file_buffer of length - file_length.
**/
DOCUMENT * text_to_document(char *file_buffer, int file_length);

#endif