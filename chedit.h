#ifndef chedit_h

typedef struct {
        char *array;
        int length;
} STRING;

typedef struct {
        STRING **lines;
        int length;
} DOCUMENT;

typedef struct {
        int y;
        int x;
        int select_y;
        int select_x;
        int selection;
        int vertical_scroll;
        int horizontal_scroll;
        int max_window_y;
        int max_window_x;
        int highlight;
        int highlight_tick;
} CURSOR;

void increment_x(DOCUMENT *doc, CURSOR *cur, char shift);
void goto_line(DOCUMENT *doc, CURSOR *cur, int line_number);

#endif
