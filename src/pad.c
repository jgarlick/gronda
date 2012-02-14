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
	string_t    *buf;
	char        tmp[1024];
	char        *c;
	char   *args[2];

	f = fopen (filename, "r");
	if (!f && (errno != ENOENT))
	{
		output_message_c (filename, "%s", strerror(errno));
		return;
	}

	pad->filename = strdup (filename);
	e->redraw |= (DIRTY_ALL | TITLE | STATS);

	if (!f)
		return;

	buf = string_alloc ("");

	while (!feof (f))
	{
		if (fgets (tmp, 1024, f) == NULL)
			continue;

		c = tmp;
		while (*c != '\n' && *c)
			c++;

		if (*c == '\n')
		{
			*c = 0;
			string_append (buf, "%s", tmp);

			args[0] = "es";
			args[1] = buf->data;
			cmd_es (2, args);

			args[0] = "ad";
			args[1] = "-s";
			cmd_ad (2, args);
			cmd_tl (0, NULL);

			string_truncate (buf, 0);
		}
		else
			string_append (buf, "%s", tmp);

	}

	if (strlen (buf->data) > 0)
	{
		args[0] = "es";
		args[1] = buf->data;
		cmd_es (2, args);
	}

	string_free (buf);

	pad->curs_x   = 1;
	pad->curs_y   = 1;
	pad->offset_x = 0;
	pad->offset_y = 0;
	pad->flags    = 0;

	/* set the read/write / read only mode */
	if (access (pad->filename, W_OK) != -1)
		pad->flags |= FILE_WRITE;
	else
		pad->flags &= ~FILE_WRITE;
}