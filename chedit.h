#ifndef chedit_h
#define chedit_h

#include <ncurses.h>
#include "document.h"
#include "cursor.h"
#include "input.h"

typedef struct DOCUMENT DOCUMENT;
typedef struct CURSOR CURSOR;
typedef struct STRING STRING;

/**
* Setup curses for ChEdit.
**/
void curses_setup();

/**
* Draw the diagnostics window.
**/
void draw_diag_win(WINDOW *diag_win, int interrupt, int max_y, int max_x, int cur_y, int cur_x, char *ch);

/**
* Draw the title bar.
**/
void draw_title_bar(WINDOW *window, int max_x, char *savepath, char *savefile, char unsaved_changes);

/**
* Get the display length of a line in ChEdit.
**/
int get_line_display_length(STRING *line);

/**
* Show a warning. Exit ChEdit without saving if the user presses ESC. Return to ChEdit if the user presses RETURN.
**/
char show_exit_warning(CURSOR *cur);

/**
* Move the cursor cur to the next line containing the text the user enters.
**/
void find(DOCUMENT *doc, CURSOR *cur);

/**
* Draw the line numbers.
**/
void draw_line_numbers(WINDOW *window, CURSOR *cursor, int doc_length);

/**
* Draw the text display.
**/
void draw_text(WINDOW *window, DOCUMENT *doc, CURSOR *cur);

#endif