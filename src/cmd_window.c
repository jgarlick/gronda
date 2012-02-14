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
 *  cmd_navigation.c
 *  purpose : Window and pane movement commands
 *  authors : James Garlick
 */
#include <string.h>

#include "editor.h"

/*void wc_menu_handler (char *text, int index)
{
	if (index == 1)
	{
		cmd_pw (0, 0);
	}

	if (index != 3)
		sig_cleanexit ("%s %s\n", EDITOR_NAME, EDITOR_VERSION);
}*/

#define EXIT_TEXT "\n%s  Version %s\n", EDITOR_NAME, EDITOR_VERSION

void wc_prompt_callback(char *text) {
	if(*text && (*text == 'y' || *text == 'Y'))
		sig_cleanexit (EXIT_TEXT);

	pad_clear_prompt(e->cepad);
}

void cmd_wc (int argc, char *argv[])
{
//	menu_t *m;
	int force_quit = 0;
	
	if (argc > 1) {
		if (!strcmp(argv[1], "-y")) { /* force quit without saving */
			force_quit = 1;
		} else if (!strcmp(argv[1], "-f")) { /* save and quit */
			cmd_pw(0, 0);
		}
	}

	if ((e->cpad->flags & MODIFIED) && !force_quit)
	{
		/* TODO: use a pre-allocated menu */
/*		m = menu_alloc ("File modified; okay to quit?");

		menu_add_item (m, "Save and quit",        's', wc_menu_handler);
		menu_add_item (m, "Quit without saving",  'q', wc_menu_handler);
		menu_add_item (m, "Return to the editor", 'r', wc_menu_handler);

		display_do_menu (m);

		menu_free (m);*/
		pad_set_prompt(e->cpad, "File modified; okay to quit? ", wc_prompt_callback);
		e->occupied_window = COMMAND_WINDOW;
	}
	else
		sig_cleanexit (EXIT_TEXT);
}

void cmd_tdm (int argc, char *argv[])
{
	e->occupied_window = COMMAND_WINDOW_REQUEST;
}


