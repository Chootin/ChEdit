#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

int main(int argc, char *argv[]) {
	initscr();
	noecho();
	int *ch = (int *) malloc(10 * sizeof(int));
	int index = 0;
	while (1) {
		index = -1;
		do {
			ch[++index] = getch();
		} while (ch[index] != -1 && ch[index] != 0);
		
		
		if (ch[0] != -1 && ch[1] != 0) {
			clear();
			mvprintw(0, 0, "%d %d %d %d %d %d %d %d %d %d\n", ch[0], ch[1], ch[2], ch[3], ch[4], ch[5], 
			ch[6], ch[7], ch[8], ch[9]);
			refresh();
		}
	}
	return 0;	
}	
