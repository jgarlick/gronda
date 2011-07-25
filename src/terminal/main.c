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
#include <curses.h>
#include <term.h>

#include <sys/time.h>
#include <string.h>

#include "../include/editor.h"
#include "include/terminal.h"

void get_viewport_size (int *width, int *height)
{
	if (lineno)
		*width = dimensions.ts_cols - 6;
	else
		*width = dimensions.ts_cols;

	*height = dimensions.ts_lines - 2;
}


void
resize ()
{
	pad_t  *mover;
	int width, height;
	
	get_viewport_size(&width, &height);

	for (mover = e->pad_head; mover; mover = mover->next)
		pad_set_viewport_size(mover, width, height);

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
	keydef_t *keydef;

	editor_setup(argc, argv);
	display_init ();
	redraw();

	/* start timer */
	setitimer (ITIMER_REAL, &value, NULL);

	/* main loop */
	while (1)
	{
		/* blocks until event or timer tick */
		next_event ();

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
