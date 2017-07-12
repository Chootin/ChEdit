#ifndef chedit_h

struct line {
        char *array;
        int length;
};

struct document {
        struct line **lines;
        int length;
};

struct cursor {
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
};

void increment_x(struct document *doc, struct cursor *cur, char shift);

#endif
