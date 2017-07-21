#ifndef chedit_h
#define chedit_h

#include <ncurses.h>
#include "document.h"
#include "cursor.h"
#include "input.h"

typedef struct DOCUMENT DOCUMENT;
typedef struct CURSOR CURSOR;
typedef struct STRING STRING;

void curses_setup();
void draw_diag_win(WINDOW *diag_win, int interrupt, int max_y, int max_x, int cur_y, int cur_x, char *ch);
void draw_title_bar(WINDOW *window, int max_x, char *savepath, char *savefile, char unsaved_changes);
int get_line_display_length(STRING *line);
void draw_text(WINDOW *window, DOCUMENT *doc, CURSOR *cur);

#endif
