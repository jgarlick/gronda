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
 *  editor.c
 *  authors : James Garlick
 */
#include <string.h>

#include "editor.h"

editor_t *e;

void editor_init ()
{
	e = ALLOC (editor_t);

	if (e == NULL)
	{
		debug ("(main.c) Unable to allocate memory for editor");
		exit (1);
	}

	e->occupied_window = EDIT_WINDOW;

	/* new pad */
	e->pad_head = pad_new();
	e->cpad = e->pad_head;

	e->input = ALLOC (input_t);
	if (e->input == NULL)
	{
		debug ("(main.c) Unable to allocate memory for e->input");
		exit (1);
	}

	e->input->offset = 0;
	e->input->curs_x = 0;
	e->flags |= INSERT;
}

void editor_setup (int argc, char **argv)
{
	char   *s;
	char   *home;
	char    tmp[256];
	int     lines;
	int     a;
	char   *args[2];
	
	sig_init ();
	editor_init ();
	KEY_init ();
	create_local_config ();
	add_base_commands ();

	args[1] = 0;

	/* handle command line args */
	for (a = 1; a < argc; a++)
	{
		s = argv[a];

		if (*s == '-')
		{
			s++;
			if (*s == 'd' || !strcasecmp (s, "-debug"))
			{
				e->flags |= DEBUG;
			}
			else if (*s == 'v' || !strcasecmp (s, "-version"))
			{
				sig_cleanexit ("%s (v%s)\nhttp://gronda.sourceforge.net\n\n",
							   EDITOR_NAME, EDITOR_VERSION);
			}
			else if (*s == 'h' || !strcasecmp (s, "-help"))
			{
				sig_cleanexit
					("Usage: ge [--debug] [--version] [--help] [filename]\n");
			}
		}
		else
		{
			args[0] = "ce";
			args[1] = s;
		}
	}

	/* read a config file */
	home = getenv ("HOME");

	if (home != NULL)
	{
		sprintf (tmp, "%s/.gronda/startup", home);

		lines = parse_commandfile (tmp);
	}

	if (lines == -1)
	{
		lines = parse_commandfile ("grondarc");

		if (lines == -1)
			sig_cleanexit
				("Can't load configuration file grondarc\nThis is essential to the functioning of %s.\nAborting.\n",
				 EDITOR_NAME);
	}

	output_message ("Config file: %d lines processed", lines);
	
	if (args[1] != 0) {
		cmd_ce (2, args);
	}
}

void
editor_fini ()
{
	/* TODO */
	FREE (e);
}
