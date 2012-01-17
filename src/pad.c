/*  Gronda/Ce
 *  Copyright (C) 2001-2003 James Garlick and Graeme Jefferis
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