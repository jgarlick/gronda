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
 *  main.c
 *  purpose : main program loop 
 *            editor initialisation and finalisation
 *  authors : James Garlick
 */
#include <sys/time.h>
#include <string.h>

#include "../include/editor.h"

void
resize ()
{
	pad_t  *mover;

	for (mover = e->pad_head; mover; mover = mover->next)
	{
		display_max_pad_dim (&(mover->width), &(mover->height));

		if (mover->curs_x > mover->width)
			mover->curs_x = mover->width;

		if (mover->curs_y > mover->height)
			mover->curs_y = mover->height;
	}

	if (e->dm)
		free (e->dm);

	e->dm = (int *) calloc ((e->pad_head->width + 1) * (e->pad_head->height + 1), 4);
	if (!e->dm)
	{
		debug ("Out of memory");
		/* TODO */
	}

	e->redraw = DIRTY_ALL | TITLE | STATS | COMMAND | OUTPUT;
	redraw ();
}


int
main (int argc, char **argv)
{
	struct itimerval value;
	char   *s;
	char   *home;
	char    tmp[256];
	int     lines;
	int     a;
	char   *args[2];
	keydef_t *keydef;

	sig_init ();

	editor_init ();

	create_local_config ();

	display_init ();

	add_base_commands ();

	/* handle command line args before plugin load */
	for (a = 1; a < argc; a++)
	{
		s = argv[a];

		if (*s == '-')
		{
			s++;
			if (*s == 'd' || !strcasecmp (s, "-debug"))
				e->flags |= DEBUG;
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

	/* handle command line args after plugin load */
	for (a = 1; a < argc; a++)
	{
		s = argv[a];

		if (*s == '-')
		{
			s++;
			if (*s == 'v' || !strcasecmp (s, "-version"))
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

			cmd_ce (2, args);
		}
	}

	e->redraw |= OUTPUT;
	redraw ();

	/* start timer */
	setitimer (ITIMER_REAL, &value, NULL);

	/* main loop */
	while (1)
	{
		/* blocks until event or timer tick */
		display_nextevent ();

		if (e->key[0] != 0) /* && key != 410)*/
		{
			debug ("KEY %s", e->key);

			if (e->occupied_window == COMMAND_WINDOW)
			{
				if (!strcmp(e->key, "esc")) {
					e->occupied_window = EDIT_WINDOW;
					redraw ();
				} else if (!strcmp(e->key, "enter")) {
					if (*(e->input->buffer))
					{
						debug ("To Parser: %s", e->input->buffer);
						parse ("%s", e->input->buffer);
						memset (e->input->buffer, 0, BUFFER_SIZE);
						e->input->curs_x = 0;
						e->input->offset = 0;
						e->redraw |= COMMAND;

						redraw ();
					}
					else
					{
						e->occupied_window = EDIT_WINDOW;
						redraw ();
					}
				} else if (!strcmp(e->key, "bs")) { /* backspace */
					if (e->input->curs_x)
					{
						e->input->curs_x--;
						e->input->buffer[e->input->curs_x + e->input->offset] =
							0;
						e->redraw |= COMMAND;
						redraw ();
					}
				} else if (!strcmp(e->key, "squote")) {
					e->input->buffer[e->input->curs_x + e->input->offset] =
						'\'';
					e->input->curs_x++;
					e->redraw |= COMMAND;
					redraw ();
				} else if (!strcmp(e->key, "dquote")) {
					e->input->buffer[e->input->curs_x + e->input->offset] =
						'\"';
					e->input->curs_x++;
					e->redraw |= COMMAND;
					redraw ();
				} else if (strlen(e->key) == 1 && *e->key >= 32 && *e->key <= 126) {
					e->input->buffer[e->input->curs_x + e->input->offset] =
						(char) *e->key;
					e->input->curs_x++;
					e->redraw |= COMMAND;
					redraw ();
				}
			}
			else if (e->occupied_window == EDIT_WINDOW)
			{
				keydef = KEY_find (e->key);

				if (keydef)
				{
					debug ("Found keydef for %s : '%s'", e->key, keydef->def);
					parse ("%s", keydef->def);

					redraw ();
				}
				else
					debug ("No keydef defined for (%s)\n", e->key);
			}
		}
	}

	return 0;
}
