#ifndef input_h
#define input_h

#include "document.h"
#include "cursor.h"

typedef struct DOCUMENT DOCUMENT;
typedef struct CURSOR CURSOR;

/**
* Perform commands on document - doc with cursor - cur with char array - ch containing the user input.
**/
int process_command(DOCUMENT *doc, CURSOR *cur, char *ch);

/**
* Add text to document - doc on cursor - cur with char array - ch containing the user input.
**/
int process_text(DOCUMENT *doc, CURSOR *cur, char *ch);

/**
* Returns true if ch is between A to Z or a to z inclusive.
**/
char is_alpha(char ch);

/**
* Returns true if ch is between 0 and 9.
**/
char is_numeric(char ch);

/**
* Returns true if the character is between A to Z, a to z and 0 - 9.
**/
char is_printable(char ch);

/**
* Returns true if the character is ESC
**/
char is_esc(char ch);

#endif