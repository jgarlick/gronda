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
 *  pad.c
 *  authors : James Garlick
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "editor.h"

pad_t *pad_new()
{
	pad_t *new;
	
	new = ALLOC (pad_t);
	if (new == NULL)
	{
		debug ("(pad.c) Unable to allocate memory for pad");
		exit (1);
	}
	
	new->line_head = LINE_new_lines ();
	new->curs_x = 1;
	new->curs_y = 1;
	new->flags |= FILE_WRITE;

	/* initial values for width and height must be non-zero */
	new->width = 10;
	new->height = 10;
	
	new->prompt = string_alloc("Command: ");
	
	return new;
}

pad_t *pad_add() {
	pad_t *cpad, *new;

	cpad = e->cepad;
	new  = pad_new();
	if(cpad) {
		new->next = cpad->next;
		cpad->next = new;
		
		new->width  = cpad->width;
		new->height = cpad->height;
	}
	e->cpad = e->cepad = new;
	if (e->pad_head == NULL) e->pad_head = new;

	return new;
}

void pad_set_viewport_size(pad_t *pad, int width, int height)
{
	pad->width  = width;
	pad->height = height;
	
	/* make sure cursor is within the viewport */
	if (pad->curs_x > pad->width)
		pad->curs_x = pad->width;

	if (pad->curs_y > pad->height)
		pad->curs_y = pad->height;
}

/* get the character at the given co-ordinates in a pad, */
/* accounting for tabs */
char pad_get_char_at (pad_t *p, int x, int y)
{
	line_t *l;
	char   *c;
	char   r = 0;
	int    a;
	int    offset_l;
	int    offset_r;
	int    width;

	l = LINE_get_line_at (p, y);
	offset_r = 0;

	if (l != NULL && l->str != NULL)
	{
		c = l->str->data;

		a = 0;
		while (*c && a < x - 1)
		{
			if (*c == '\t')
			{
				width = 4;
				a += width; /* TODO: tab stops */
			}
			else
				a++;

			if (a < x) c++;
		}

		r = *c;
	}

	offset_l = x - a - 1;

	if (offset_l != 0)
	{
		offset_l += width;
		offset_r = width - offset_l;
	}

	return r;
}

void pad_modified (pad_t *pad)
{
	if (!(pad->flags & MODIFIED))
	{
		pad->flags |= MODIFIED;
		e->redraw |= STATS;
	}
}

int pad_pos_x(pad_t *pad) {
	return pad->curs_x + pad->offset_x;
}

int pad_pos_y(pad_t *pad) {
	return pad->curs_y + pad->offset_y;
}

/* move the cursor to an absolute position in the pad and scroll into view if needed */
void pad_goto(pad_t *pad, int row, int col, int adjust) {
	cursor_set_pos(pad, row - pad->offset_y, col - pad->offset_x, adjust);
	move_cursor_into_view(pad);
}

void pad_set_prompt(pad_t *pad, char *str, void (*callback) ()) {
	string_truncate(pad->prompt, 0);
	string_append(pad->prompt, str);
	pad->prompt_callback = callback;
}

void pad_clear_prompt(pad_t *pad) {
	string_truncate(pad->prompt, 0);
	string_append(pad->prompt, "Command: ");
	pad->prompt_callback = NULL;
}

void pad_read_file(pad_t *pad, char *filename) {
	FILE        *f;
	char        tmp[1024];
	char        *c;
	char        val;
	line_t      *line;

	f = fopen (filename, "r");
	if (!f && (errno != ENOENT))
	{
		output_message_c (filename, "%s", strerror(errno));
		return;
	}

	pad->filename = strdup (filename);
	e->redraw |= (DIRTY_ALL | TITLE | STATS);

	/* if the file does not exist just return at this stage */
	if (!f)	return;

	line = NULL;

	while (!feof (f))
	{
		if (fgets (tmp, 1024, f) == NULL)
			continue;

		c = tmp;
		while (*c != '\n' && *c)
			c++;

		val = *c;
		if (val == '\n') *c = 0;

		if(line == NULL) {
			line = LINE_append(pad, tmp);
		} else {
			string_append(line->str, "%s", tmp);
		}

		if (val == '\n') line = NULL;
	}

	pad->flags    = 0;

	/* set the write flag if the file can be written */
	if (access (pad->filename, W_OK) != -1)
		pad->flags |= FILE_WRITE;
}

/*** UNDO ACTIONS **/

void pad_grow (pad_t *pad, int ypos)
{
	while (pad->line_count < ypos)
	{
		/* TODO: LINE_append should really return an error status,
				 and it should be checked here */
		LINE_append (pad, "");
	}
}

#define PADSTEP (8)

/* insert a string at the current cursor position */
void pad_insert_string(pad_t *pad, const char *str) {
	int     xpos;
	int     ypos;
	line_t *cline;
	int     intab;
	
	ypos = pad_pos_y(pad);

	if (ypos > pad->line_count)
		pad_grow (pad, ypos);

	cline = LINE_get_line_at (pad, ypos);
	if (cline->str == NULL)
		cline->str = string_alloc ("");

	xpos = get_string_pos (pad_pos_x(pad), line_data(cline), &intab);

	if (e->flags & INSERT) {
		string_insert (cline->str, xpos, "%s", str);
//		printf("INSERT STRING :%s:\n", inserted_str);
	} else
		string_overwrite (cline->str, xpos, "%s", str);

/*	string_debug (cline->str);*/

	/* horizontal cursor movement */
	pad->curs_x = get_curs_pos (xpos + strlen (str), cline) - pad->offset_x;

	while (pad->curs_x > pad->width)
	{
		pad->offset_x += PADSTEP;
		pad->curs_x   -= PADSTEP;
	}

	pad_modified (pad);	
}