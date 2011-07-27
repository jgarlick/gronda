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
 *  cmd_cursor.c
 *  purpose : Cursor movement commands
 *  authors : James Garlick
 */
#include "editor.h"

#include <string.h>

void cmd_au (int argc, char *argv[])
{
	int scroll = 0;
	char *args[2];

	if (argc > 1 && strcmp (argv[1], "-s") == 0)
		scroll = 1;

	if (!scroll) /* no scrolling */
	{	
		if (e->cpad->curs_y > 1)
			cursor_set_pos (e->cpad, e->cpad->curs_y - 1, e->cpad->curs_x, ADJUST_LEFT);
	}
	else /* with scrolling */
	{
		if (e->cpad->offset_y > 0 || e->cpad->curs_y > 1)
		{
			cursor_set_pos (e->cpad, e->cpad->curs_y - 1, e->cpad->curs_x, ADJUST_LEFT);

			if (e->cpad->curs_y < 1)
			{
				args[0] = "pv";
				args[1] = "-1";
				cmd_pv (2, args);
			}

		}
		else
			display_beep ();
	}
}

void cmd_ad (int argc, char *argv[])
{
	int scroll = 0;
	char *args[2];

	if (argc > 1 && strcmp (argv[1], "-s") == 0)
		scroll = 1;

	if (!scroll) /* no scrolling */
	{
		if (e->cpad->curs_y < e->cpad->height)
			cursor_set_pos (e->cpad, e->cpad->curs_y + 1, e->cpad->curs_x, ADJUST_LEFT);
	}
	else /* with scrolling */
	{
		/* the pad can only scroll up to the point where the last */
		/* line of the file is the first visible line             */
		if (e->cpad->offset_y < e->cpad->line_count - 1 || e->cpad->curs_y < e->cpad->height)
		{
			cursor_set_pos (e->cpad, e->cpad->curs_y + 1, e->cpad->curs_x, ADJUST_LEFT);

			if (e->cpad->curs_y > e->cpad->height)
			{
				args[0] = "pv";
				args[1] = "1";
				cmd_pv (2, args);
			}
		}
		else
			display_beep ();
	}
}

void cmd_al (int argc, char *argv[])
{
	pad_t *p  = e->cpad;
	int  wrap = 0;

	if (argc > 1 && strcmp (argv[1], "-w") == 0)
		wrap = 1;

	if (p->curs_x == 1)
	{
		if (wrap && p->curs_y > 1)
		{
			cmd_au (0, NULL);
			cmd_tr (0, NULL);
		}
		return;
	}

	cursor_set_pos (p, p->curs_y, p->curs_x - 1, ADJUST_LEFT);
}

void cmd_ar (int argc, char *argv[])
{
	pad_t  *p   = e->cpad;
	int    wrap = 0;
	line_t *l;
	int    len;

	if (argc > 1 && strcmp (argv[1], "-w") == 0)
		wrap = 1;

	if (wrap)
	{
		l = LINE_get_line_at (p, p->curs_y + p->offset_y);
		len = line_length (l);

		if (p->curs_x + p->offset_x > len && p->curs_y != p->height)
		{
			cmd_ad (0, NULL);
			cmd_tl (0, NULL);
			return;
		}
	}

	if (p->curs_x < p->width)
		cursor_set_pos (p, p->curs_y, p->curs_x + 1, ADJUST_RIGHT);
}

void cmd_tt (int argc, char *argv[])
{
	cursor_set_pos (e->cpad, 1, e->cpad->curs_x, ADJUST_LEFT);
}

void cmd_tb (int argc, char *argv[])
{
	cursor_set_pos (e->cpad, e->cpad->height, e->cpad->curs_x, ADJUST_LEFT);
}

void cmd_tl (int argc, char *argv[])
{
	pad_t  *p = e->cpad;

	p->curs_x = 1;

	if (p->offset_x > 0)
	{
		p->offset_x = 0;
		e->redraw |= DIRTY_ALL;
	}
}

void cmd_tr (int argc, char *argv[])
{
	pad_t  *p = e->cpad;
	line_t *l;
	int     len;

	l = LINE_get_line_at (p, p->curs_y + p->offset_y);

	len = line_length (l);

	if (len + 2 > p->width)
	{
		p->offset_x = (len + 2) - p->width;
		p->curs_x = p->width - 1;
	}
	else
	{
		p->offset_x = 0;
		p->curs_x = len + 1;
	}

	e->redraw |= DIRTY_ALL;
}

