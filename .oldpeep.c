/*
**	peep: text file reader
**	written by stephanos pavlou in Nov 2021 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <curses.h>

/* main loop for reading from file */
int read_loop(FILE *fp);

int main(int argc, char **argv) {
	/* validate process arguments and open input file */
	if (argc != 2) {
		fprintf(stderr, "Incorrect usage.\n");
		exit(1);
	} else if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0
		|| strcmp(argv[1], "-h") == 0) {
	  	printf("peep version 0.1, Nov 06 2021\n"
			   "Author: Stephanos Pavlou\n\n");
		exit(0);
	}
	
	const char *pathname = argv[1];
	FILE *fp = fopen(pathname, "r");
	if (fp == NULL) {
		fprintf(stderr, "Failed to open file: %s\n", pathname);
		exit(1);
	}
	
	/* init ncurses */
	initscr();

	if (read_loop(fp) != 1) {
		fprintf(stderr, "Failed to read from file: %s\n", fp);
		exit(1);
	}

	/* close process */
	endwin();
	fclose(fp);

	exit(0);
}

int read_loop(FILE *fp) {
	/* general vars */
	int lncount = 0, curline = 1, offset = 0;
	char displaybuf[LINES*COLS];
	int *cinline = malloc(sizeof(int));
	char input = 'a';

	/* count lines in file and count chars in line */
	int c = 0, l = 0, charcount = 0;
	while ((c = getc(fp)) != EOF) {
		charcount++;
		if (c == '\n') {
			lncount++;
			cinline[l] = charcount;
			l++;
			cinline = reallocarray(cinline, l + 1, sizeof(int));
			charcount = 0;
		}
	}

	/* rewind file pointer to start of file */
	rewind(fp);

	/* display loop */
	int count = 0, i = 0; c = 0;
	while (1) {
		i = 0, c = 0, count = 0;

		while (count < LINES) {
			displaybuf[i] = (char)getc(fp);
			c++;
			if (displaybuf[i] == '\n' || c == COLS) {
				count++;
				c = 0;
			}
			i++;
		}
		clear();

		printw(displaybuf);
		refresh();

		/* get input */
		input = (char)getch();
		if (input == 'j') {
			/* going down */
			if (curline + LINES <= lncount) {
				offset += cinline[curline - 1];
				curline++;
			}
		} else if (input == 'k') {
			/* going up */
			if (curline - 1 > 0) {
				offset -= cinline[curline - 2];
				curline--;
			}
		} else if (input == 'q') {
			/* exit process */
			break;
		}
	
		fseek(fp, offset, SEEK_SET);	
	}
	return 1;
}
