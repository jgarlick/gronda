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
 *  cmd_buffer.c
 *  purpose : Paste buffer commands
 *  authors : James Garlick
 */

#include <string.h>

#include "editor.h"

void cmd_dr (int argc, char *argv[])
{
	e->cpad->mark_y = e->cpad->curs_y + e->cpad->offset_y;
	e->cpad->mark_x = e->cpad->curs_x + e->cpad->offset_x;
}

void cmd_echo (int argc, char *argv[])
{
	int echo;

	if (argc > 1 && argv[1][1] == 'r')
		echo = REGION_RECT;
	else	
		echo = REGION_LINEAR;

	if (e->cpad->mark_x && e->cpad->mark_y)
		e->cpad->echo = echo;
}


void cmd_xcxdxa (int argc, char *argv[], int cut)
{
	int start_y, start_x, end_y, end_x;
	buffer_t *buf;
	int a;
	char *buf_name;

	int shape  = REGION_LINEAR;
	int append = 0;
	char *file_name = "";

	buf_name = "primary";
	for (a = 1; a < argc; a++)
	{
		if (argv[a][0] != '-')
			buf_name = argv[a];
		else if (strstr (argv[a], "-r") == argv[a])
			shape = REGION_RECT;
		else if (strstr (argv[a], "-a") == argv[a])
			append = 1;
		else if (strstr (argv[a], "-f") == argv[a] && a + 1 < argc)
			file_name = argv[a + 1];
	}

	get_region (shape, &start_y, &start_x, &end_y, &end_x);

	buf = buffer_create (buf_name);

	if (buf == NULL)
	{
		output_message_c (argv[0], "Error creating buffer");
		return;
	}

	if (!append) string_truncate (buf->str, 0);

	buffer_cutcopy (buf, cut, shape, start_y, start_x, end_y, end_x);

	if (*file_name)
		buffer_save (buf, file_name);

	debug ("|%s|", buf->str->data);

	e->cpad->echo   = 0;
	e->cpad->mark_y = 0;
	e->cpad->mark_x = 0;

	if (cut)
	{
		e->cpad->curs_y = start_y - e->cpad->offset_y;
		e->cpad->curs_x = start_x - e->cpad->offset_x;
	}
	
	display_copy(buf);
}

void cmd_xc (int argc, char *argv[])
{
	cmd_xcxdxa (argc, argv, 0);
}

void cmd_xd (int argc, char *argv[])
{
	cmd_xcxdxa (argc, argv, 1);
}

void cmd_xa (int argc, char *argv[])
{
	output_message ("Unimplemented");
}

void cmd_xp (int argc, char *argv[])
{
	char     *buf_name;
	int      curs_y, curs_x;
	buffer_t *mover;
	pad_t    *p = e->cpad;
	char     *args[2];
	char     *start, *end;
	int      newline;
	int      shape;
	int      force_overstrike;
	int      flags;
	int      a;

	char     *file_name = "";

	buf_name         = "default";
	force_overstrike = 0;
	shape = REGION_LINEAR;

	for (a = 1; a < argc; a++)
	{
		if (argv[a][0] != '-')
			buf_name = argv[a];
		else if (strstr (argv[a], "-r") == argv[a])
			shape = REGION_RECT;
		else if (strstr (argv[a], "-a") == argv[a])
			force_overstrike = 1;
		else if (strstr (argv[a], "-f") == argv[a] && a + 1 < argc)
			file_name = argv[a + 1];
	}

	/* force overstrike only works for rectangular paste */
	if (force_overstrike && !(shape & REGION_RECT))
		force_overstrike = 0;

	if (*file_name)
	{
		mover = buffer_load (file_name);
		if (!mover)
			return;
	}
	else
	{
		mover = e->buffer_head;

		while (mover && strcmp (buf_name, mover->name))
			mover = mover->next;

		if (!mover)
		{
			output_message_c ("xp", "Cannot open paste buffer %s", buf_name);
			return;
		}
	}

	if (mover->str && mover->str->data && strlen (mover->str->data) > 0)
	{
		curs_y = p->curs_y;
		curs_x = p->curs_x;

		if (force_overstrike)
		{
			flags = e->flags;
			e->flags &= (~INSERT);
		}

		start = end = mover->str->data;
		newline = 0;

		while (*start)
		{
			while (*end && *end != '\n')
				end++;

			if (*end == '\n')
			{
				*end    = '\0';
				newline = 1;
			}

			args[0] = "es";
			args[1] = start;
			cmd_es (2, args);

			if (newline)
			{
				*end++ = '\n';
				newline = 0;
				if (shape & REGION_LINEAR)
				{
					args[0] = "en";
					cmd_en (1, args);
				}
				else /* REGION_RECT */
				{
					p->curs_y++;
					p->curs_x = curs_x;
				}
			}
			start = end;
		}

		if (force_overstrike)
			e->flags = flags;

		p->curs_y = curs_y;
		p->curs_x = curs_x;
	}
}

