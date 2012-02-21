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

void cmd_search (int argc, char *argv[])
{
	pad_t  *pad = e->cpad;
	line_t *l;
	regmatch_t matchptr[10];
	int flags;
	int r;
	int start_x, y, intab, line_len;
	const char *ptr;
	char buf[255];

	if(argc > 1) {
		regfree(&(pad->search));
	
		r = regcomp(&(pad->search), argv[1], REG_EXTENDED);
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
			// move to the correct place in the line but not past the end of the line
			ptr += (start_x < line_len) ? start_x : line_len;
		}

		/* only perform the search if we are not past the end of the line */
		if(start_x <= line_len) {
			if(start_x > 0)
				flags = REG_NOTBOL;
			else
				flags = 0;

			r = regexec(&(pad->search), ptr, 10, matchptr, flags);
		}

		if (!r) {
			pad->curs_x = get_curs_pos(matchptr[0].rm_so + start_x, l) - pad->offset_x;
			pad->curs_y = y - pad->offset_y;
			move_cursor_into_view(pad);
			output_message("");
		} else if(r != REG_NOMATCH) {
			regerror(r, &(pad->search), buf, 255);
			output_message(buf);
		}
		y++;
		l = l->next;
		start_x = 0;
	}
	if(r == REG_NOMATCH) {
		output_message("No match");
	}
}