/*  Gronda/Ce
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

#include <sys/types.h>
#include <regex.h>

#include "editor.h"

void cmd_search (int argc, char *argv[])
{
	pad_t  *pad = e->cpad;
	line_t *l;
	regex_t regex;
	regmatch_t matchptr[10];
	int r;
	int start_x, y, intab;
	char *ptr;
	
	debug("searching for %s", argv[1]);
	
	r = regcomp(&regex, argv[1], REG_EXTENDED);
	if(r){
		output_message ("Could not compile regex");
		return; 
	}
	r = REG_NOMATCH;
	y = pad_pos_y(pad);
	start_x = pad_pos_x(pad) + 1;
//	debug("start_x = %d", start_x);
	l = LINE_get_line_at (pad, y);
	while(l && l != pad->line_head && r == REG_NOMATCH) {
		if(l->str && l->str->data) {
			ptr = l->str->data;
			if(start_x > 0) { // first line, move to the correct place in the line
				start_x = get_string_pos(start_x, l->str->data, &intab);
				ptr += start_x;
//				debug("searching from %s", ptr);
			}
//			debug("match string = %s", ptr);
			r = regexec(&regex, ptr, 10, matchptr, 0);
		}
		if (!r) {
			cursor_set_pos (pad, y, matchptr[0].rm_so + start_x + 1, ADJUST_RIGHT);
			move_cursor_into_view(e->cpad);
		} else if(r != REG_NOMATCH) {
			output_message("error");
		}
		y++;
		l = l->next;
		start_x = 0;
	}
	if(r == REG_NOMATCH) {
		output_message("No match");
	}
	regfree(&regex);
}