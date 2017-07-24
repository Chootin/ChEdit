#ifndef cursor_h
#define cursor_h
#include "document.h"
#include "string.h"

typedef struct DOCUMENT DOCUMENT;
typedef struct CURSOR CURSOR;
typedef struct STRING STRING;

struct CURSOR {
        int y;
        int x;
        int vertical_scroll;
        int horizontal_scroll;
        int max_window_y;
        int max_window_x;
};

/**
* Set the cursor - cur on line number - line_number in document doc.
**/
void goto_line(DOCUMENT *doc, CURSOR *cur, int line_number);

/**
* Set the position of cursor - cur to the end of the current line in document doc.
**/
void seek_line_end(DOCUMENT *doc, CURSOR *cur);

/**
* Set the position of cursor - cur to the first non-whitespace character of the current line in document doc.
**/
void seek_line_start(DOCUMENT *doc, CURSOR *cur);

/**
* Move the cursor - cur one position to the right in the current line of document doc.
* If shift is true, instead adjust the selection to include an additional column to the right.
**/
void increment_x(DOCUMENT *doc, CURSOR *cur, char shift);

/**
* Move the cursor - cur one position to the left in the current line of document doc.
* If shift is true, instead adjust the selection to include an additional column to the left.
**/
void decrement_x(DOCUMENT *doc, CURSOR *cur, char shift);

/**
* Move the cursor - cur one line down in the document - doc.
**/
void increment_y(DOCUMENT *doc, CURSOR *cur);

/**
* Move the cursor - cur one line up in the document - doc.
**/
void decrement_y(DOCUMENT *doc, CURSOR *cur);

/**
* Move the cursor - cur down the document - doc until it reaches the next line which 
* contains characters other than whitespace.
**/
void increment_y_para(DOCUMENT *doc, CURSOR *cur);

/**
* Move the cursor up the document until it reaches the previous line which 
* contains characters other than whitespace.
**/
void decrement_y_para(DOCUMENT *doc, CURSOR *cur);

/**
* Move the cursor - cur left until it reaches the start of the previous word in the current line of document - doc.
**/
void decrement_x_word(DOCUMENT *doc, CURSOR *cur);

/**
* Move the cursor - cur right until it reaches the start of the previous word in the cureent line of document - doc.
**/
void increment_x_word(DOCUMENT *doc, CURSOR *cur);

/**
* Get the character at the position of cursor - cur in the document - doc.
**/
char get_cursor_char(DOCUMENT *doc, CURSOR *cur);

/**
* Get the position of the cursor - cur on the text display window given the tabs in the line of document - doc.
**/
int get_tab_offset(DOCUMENT *doc, CURSOR *cur);

#endif
