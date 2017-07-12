#ifndef chedit_h

typedef struct {
        char *array;
        int length;
} LINE;

typedef struct {
        LINE **lines;
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

#endif
