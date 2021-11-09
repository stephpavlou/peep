/*
**	peep version 0.1 
**	Author: Stephanos Pavlou
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <curses.h>

/* runs main loop accepting input from user */
int input_loop(FILE *fp, const char *pathname);
/* fills display buffer from file */
int fill_buffer(FILE *fp, int startOffset, int lncount, char *displaybuf);
/* prints buffer and escapes special characters */
int print_buffer(char *displaybuf, const char *pathname, int lineNum, int lncount);

int main(int argc, char **argv) {
	/* validate process args and open file */
	if(argc != 2) {
		fprintf(stderr, "You got to specify a file to peep, okay?\n");
		exit(1);
	} else if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
		printf("peep (a character file reader), version 0.1\n"
				"Author: Stephanos Pavlou\n\n");
		exit(0); 
	} 
	
	/* verify that file type is supported */
	// TODO

	const char *pathname = argv[1];
	FILE *fp = fopen(pathname, "r");
	if(fp == NULL) {
		fprintf(stderr, "Failed to open file: %s. Does it even exist?\n", pathname);
		exit(1);
	}
	
	/* we're ready to read ;) */
	/* init ncurses */
	initscr();
	cbreak();
	noecho();

	if(input_loop(fp, pathname) != 1) {
		fprintf(stderr, "Failure while reading from file.\n");
		exit(1);
	}

	/* cleanup and close */
	endwin();
	fclose(fp);

	exit(0);
}

int input_loop(FILE *fp, const char *pathname) {

	/* support resizing of term window */
	int curLines = LINES;
	int curCols = COLS;

	/* different size depending on character encoding;
	will assume for now ASCII */
	char *displaybuf = calloc(curLines*curCols, sizeof(char));
	int startOffset = 0, lineNum = 0;

	/* helpful data for moving up or down in file,
	informs offset */
	int *charInLine = malloc(sizeof(int));
	int charCount = 0, l = 0, curLine = 1;

	/* count lines in file */
	unsigned long lncount = 0;
	int c = 0;
	while((c = getc(fp)) != EOF) {
		charCount++;
		if(c == '\n') {
			lncount++;
			charInLine[l] = charCount;
			l++;
			charInLine = reallocarray(charInLine, l + 1, sizeof(int));
			charCount = 0;
		}
	}
	rewind(fp);

	int input = 0;
	/* main loop */
	while(1) {
		/* clear buffer */		
		for(int j = 0; j < curLines*curCols; j++) {
			displaybuf[j] = 0;
		}

		/* read file contents between scope and store
		in displaybuf */
		if(is_term_resized(curLines, curCols)) {
			curLines = LINES;
			curCols = COLS;
			free(displaybuf);
			displaybuf = calloc(curLines*curCols, sizeof(char));
			clear();			
		}
	
		if(fill_buffer(fp, startOffset, lncount, displaybuf) != 1) {
			return -1;
		}
		if(print_buffer(displaybuf, pathname, curLine, lncount) != 1) {
			return -1;
		}
		
		input = getch();
		if(input == 106) {
			/* going down */
			/* check if any lines from file require two lines to print */
			int priv = 1;
			/* DANGER: this is just bound to cause an issue but wth */
			for(int j = curLine; j < lncount; j++) {
				if(charInLine[j] > COLS) {
					priv++;
				}
			}

			if(curLine + (LINES - 1) <= lncount + priv) {
				startOffset += charInLine[curLine - 1];
				curLine++;
			}
		}
		else if(input == 107) {
			/* going up */
			if(curLine - 1 > 0) {
				startOffset -= charInLine[curLine - 2];
				curLine--;
			}
		}
		else if(input == 113) {
			/* quit process */
			break;
		}

		fseek(fp, startOffset, SEEK_SET);
	}
	
	free(displaybuf);

	return 1;
}

int fill_buffer(FILE *fp, int startOffset, int lncount, char *displaybuf) {
	int curLine = 0, i = 0, charCount = 0, check = 0;
	while(curLine < LINES + 1) {
		check = getc(fp);
		displaybuf[i] = (char)check;
		charCount++;
		if(check == '\n' || charCount == COLS) {
			curLine++;
			charCount = 0;
		}
		else if(check == EOF) {
			break;
		}
		i++;
	}
	
	return 1;
}

int print_buffer(char *displaybuf, const char *pathname, int lineNum, int lncount) {
	move(0, 0);
	printw(displaybuf);
	
	/* add footer */
	char footer[COLS];
	int outof = lineNum + (LINES - 3);
	if (outof > lncount) {
		outof = lncount;
	}
	snprintf(footer, COLS, " File: %s  Lines %d-%d of %d  peep 0.1 (press q to quit)", pathname, lineNum, outof, lncount);
	move(LINES - 1, 0);
	attron(A_STANDOUT);
	printw(footer);
	attroff(A_STANDOUT);

	refresh();
	return 1;
}
