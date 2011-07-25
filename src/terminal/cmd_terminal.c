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
 *  cmd_terminal.c
 *  purpose : Terminal client specific editor commands
 *  authors : James Garlick
 */
#include <curses.h>
#include <term.h>

#include <string.h>
#include <ctype.h>

#include "../include/editor.h"
#include "include/terminal.h"

void cmd_delay (int argc, char *argv[])
{
	int     delay = atoi (argv[1]);

	ESCDELAY = delay;

	output_message_c (argv[0], "Esc delay changed to %d ms", delay);
	debug ("Delay changed to %s milliseconds", argv[1]);
}

void cmd_lineno (int argc, char *argv[])
{
	if (argc > 1)
	{
		if (strcmp (argv[1], "-on") == 0)
		{
			lineno = 1;
		}
		else if (strcmp (argv[1], "-off") == 0)
		{
			lineno = 0;
		}
	}
	else
		lineno = !lineno;

	resize ();
}

void cmd_showtabs (int argc, char *argv[])
{
	if (argc > 1)
	{
		if (strcmp (argv[1], "-on") == 0)
		{
			showtabs = 1;
		}
		else if (strcmp (argv[1], "-off") == 0)
		{
			showtabs = 0;
		}
	}
	else
		showtabs = !showtabs;
}

