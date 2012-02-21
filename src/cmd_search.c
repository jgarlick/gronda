/*  Gronda Text Editor
 *  Copyright (C) 2002-2012 James Garlick and Graeme Jefferis
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of version 2 of the GNU General Public License as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  cmd_search.c
 *  purpose : Search and replace commands
 *  authors : James Garlick
 */
#include <string.h>

#include "editor.h"

#define SEARCH_FORWARDS  0
#define SEARCH_BACKWARDS 1

void search(pad_t *pad, char *new_search, int direction) {
	line_t *l;
	regmatch_t matchptr[10];
	int flags;
	int r, match_start;
	int start_x, y, intab, line_len;
	const char *ptr;
	char buf[255];

	if(new_search) {
		regfree(&(pad->search));
	
		r = regcomp(&(pad->search), new_search, REG_EXTENDED);
		if(r){
			output_message ("Could not compile regex");
			return; 
		}
	}

	r = REG_NOMATCH;
	y = pad_pos_y(pad);
	start_x = pad_pos_x(pad) + 1;

	l = LINE_get_line_at (pad, y);
	while(l && l != pad->line_head && r == REG_NOMATCH) {
		ptr      = line_data(l);
		line_len = strlen(ptr);

		if(start_x > 0) { // first line of the search
			// convert start_x from cursor position to string position
			start_x = get_string_pos(start_x, ptr, &intab);
		}

		if(direction == SEARCH_FORWARDS) {
			/* only perform the search if we are not past the end of the line */
			if(start_x <= line_len) {
				if (start_x > 0) {
					// move to the correct place in the line but not past the end of the line
					ptr += (start_x < line_len) ? start_x : line_len;
				}
				flags = (start_x > 0) ? REG_NOTBOL : 0;
				r = regexec(&(pad->search), ptr, 10, matchptr, flags);
				match_start = matchptr[0].rm_so + start_x;
			}
		} else { /* SEARCH_BACKWARDS */
			match_start = 0;
			flags = (start_x < line_len) ? REG_NOTEOL : 0;

			/* keep matching until... */
			while((!regexec(&(pad->search), ptr, 10, matchptr, flags))    // there are no more matches
					&& ((match_start + matchptr[0].rm_so + 1) < start_x)  // or we are past the cursor position
					&& (match_start <= line_len)) {                       // or we are past the end of the string
				match_start += matchptr[0].rm_so + 1;
				ptr         += matchptr[0].rm_so + 1;
				flags |= REG_NOTBOL;
			}
			if(match_start > 0 && match_start < start_x) {
				/* set up the match data at the last matched location */
				match_start--;
				if (match_start > line_len) match_start = line_len;
				r = regexec(&(pad->search), line_data(l) + match_start, 10, matchptr, 0);
			} else {
				r = REG_NOMATCH;
			}
			
		}

		if (!r) {
			pad->curs_x = get_curs_pos(match_start, l) - pad->offset_x;
			pad->curs_y = y - pad->offset_y;
			move_cursor_into_view(pad);
			output_message("");
		} else if(r != REG_NOMATCH) {
			regerror(r, &(pad->search), buf, 255);
			output_message(buf);
		}
		if (direction == SEARCH_FORWARDS) {
			y++;
			l = l->next;
			start_x = 0;
		} else {
			y--;
			l = l->prev;
			start_x = 2147483647;
		}
	}
	if(r == REG_NOMATCH) {
		output_message("No match");
		display_beep();
	}
}

void cmd_search (int argc, char *argv[]) {
	pad_t  *pad = e->cpad;
	search(pad, argc > 1 ? argv[1] : NULL, SEARCH_FORWARDS);
}

void cmd_search_backward (int argc, char *argv[]) {
	pad_t  *pad = e->cpad;
	search(pad, argc > 1 ? argv[1] : NULL, SEARCH_BACKWARDS);
}