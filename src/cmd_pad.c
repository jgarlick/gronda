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
 *  cmd_pad.c
 *  purpose : Pad manipulation commands
 *  authors : Graeme Jefferis
 *            James Garlick
 */
#include <signal.h>
#include <stdlib.h>

#include "editor.h"

void cmd_position(int argc, char *argv[]) {
	output_message("[%d,%d]", pad_pos_x(e->cpad), pad_pos_y(e->cpad));
}

void cmd_ph (int argc, char *argv[])
{
	int    offset = 1;
	pad_t  *pad = e->cpad;
	int    adjust;

	if (argc > 1)
		offset = atoi (argv[1]);

	pad->offset_x += offset;

	if (pad->offset_x < 0)
	{
		offset -= pad->offset_x;
		pad->offset_x = 0;
		debug ("(ph) Left margin (ph)");
	}

	pad->curs_x -= offset;
	if (pad->curs_x < 1)
		pad->curs_x = 1;
	else if (pad->curs_x > pad->width)
		pad->curs_x = pad->width;

	if (offset < 0)
		adjust = ADJUST_LEFT;
	else
		adjust = ADJUST_RIGHT;

	/* adjust the cursor if we have been placed in the middle of a tab */
	cursor_set_pos (e->cpad, e->cpad->curs_y, e->cpad->curs_x, adjust);

	e->redraw |= DIRTY_ALL;
}

void cmd_pv (int argc, char *argv[])
{
	int     offset = 1;
	pad_t  *pad = e->cpad;

	if (argc > 1)
		offset = atoi (argv[1]);

	pad->offset_y += offset;

	if (pad->offset_y < 0)
	{
		offset -= pad->offset_y;
		pad->offset_y = 0;
		display_beep ();
	}
	else if (pad->offset_y >= pad->line_count)
	{
		pad->offset_y = pad->line_count - 1;
		display_beep ();
	}

	pad->curs_y -= offset;
	if (pad->curs_y < 1)
		pad->curs_y = 1;
	else if (pad->curs_y > pad->height)
		pad->curs_y = pad->height;

	/* adjust the cursor if we have been placed in the middle of a tab */
	cursor_set_pos (e->cpad, e->cpad->curs_y, e->cpad->curs_x, ADJUST_LEFT);

	e->redraw |= DIRTY_ALL;
}

void cmd_pt (int argc, char *argv[])
{
	pad_t  *pad = e->cpad;
	int     pad_y;

	pad_y = pad_pos_y(pad);
	pad->offset_y = 0;

	if (pad_y < pad->height)
		pad->curs_y = pad_y;

	e->redraw |= DIRTY_ALL;
}

void cmd_pb (int argc, char *argv[])
{
	pad_t  *pad = e->cpad;
	int     pad_y;

	pad_y = pad_pos_y(pad);
	pad->offset_y = pad->line_count - pad->height;
	if(pad->offset_y < 0) pad->offset_y = 0;

	if (pad_y > pad->offset_y)
		pad->curs_y = pad_y - pad->offset_y;

	e->redraw |= DIRTY_ALL;
}

void cmd_pp (int argc, char *argv[])
{
	int     offset;
	pad_t  *pad = e->cpad;

	if (argc == 0)
		offset = pad->height;
	else
		offset = (pad->height) * atof (argv[1]);

	if (offset != 0)
	{
		debug ("pp %s\n", argv[1]);
		pad->offset_y += offset;

		if (pad->offset_y < 0)
		{
			offset -= (pad->offset_y);
			offset = abs (offset);
			pad->offset_y = 0;
			output_message ("(pp) Top of file (pp)");
		}

		if ((pad->offset_y) > (pad->line_count))
		{
			offset -= ((pad->offset_y) - (pad->line_count - 1));
			pad->offset_y = (pad->line_count) - 1;
			output_message ("(pp) Bottom of file (pp)");
		}
	}
}

