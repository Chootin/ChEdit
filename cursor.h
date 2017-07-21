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

void goto_line(DOCUMENT *doc, CURSOR *cur, int line_number);
void seek_line_end(DOCUMENT *doc, CURSOR *cur);
void seek_line_start(DOCUMENT *doc, CURSOR *cur);
void increment_x(DOCUMENT *doc, CURSOR *cur, char shift);
void decrement_x(DOCUMENT *doc, CURSOR *cur, char shift);
void increment_y(DOCUMENT *doc, CURSOR *cur);
void decrement_y(DOCUMENT *doc, CURSOR *cur);
void increment_y_para(DOCUMENT *doc, CURSOR *cur);
void decrement_y_para(DOCUMENT *doc, CURSOR *cur);
void increment_x(DOCUMENT *doc, CURSOR *cur, char shift);
void decrement_x_word(DOCUMENT *doc, CURSOR *cur);
void increment_x_word(DOCUMENT *doc, CURSOR *cur);
void shift_x(CURSOR *cur, char enabled);
char get_cursor_char(DOCUMENT *doc, CURSOR *cur);
void goto_line(DOCUMENT *doc, CURSOR *cur, int line_number);
int get_tab_offset(DOCUMENT *doc, CURSOR *cur);

#endif
