#include <stdio.h>
#include <termios.h>
#include <string.h>

int getkey() {
        int character;
        struct termios orig_term_attr;
        struct termios new_term_attr;

        /* set the terminal to raw mode */
        tcgetattr(fileno(stdin), &orig_term_attr);
        memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
        new_term_attr.c_lflag &= ~(ECHO|ICANON);
        new_term_attr.c_cc[VTIME] = 0;
        new_term_attr.c_cc[VMIN] = 0;
        tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

        /* read a character from the stdin stream without blocking */
        /*   returns EOF (-1) if no character is available */
        character = fgetc(stdin);

        /* restore the original terminal attributes */
        tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

        return character;
}

int main(int argc, char *argv[]) {
	int ch;
	while (1) {
		do {
			ch = getkey();
		} while (ch == -1);
		
		printf("%d        %c\n", ch ,ch);
	}
	return 0;	
}	
