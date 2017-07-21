#ifndef input_h
#define input_h

#include "document.h"
#include "cursor.h"

typedef struct DOCUMENT DOCUMENT;
typedef struct CURSOR CURSOR;

int process_command(DOCUMENT *doc, CURSOR *cur, char *ch);
int process_text(DOCUMENT *doc, CURSOR *cur, char *ch, int length);
char is_alpha(char ch);
char is_numeric(char ch);
char is_printable(char ch);
char is_esc(char ch);

#endif
