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
 *  command.c
 *  purpose : add / remove / find / execute commands
 *  authors : James Garlick
 */
#include <ctype.h>
#include <string.h>

#include "editor.h"

command_t *command_ptrs[27] = { NULL };

void add_base_commands ()
{
	/* buffer */
	add_command ("dr",   cmd_dr);
	add_command ("echo", cmd_echo);
	add_command ("xc",   cmd_xc);
	add_command ("xd",   cmd_xd);
	add_command ("xp",   cmd_xp);

	/* cursor */
	add_command ("au",   cmd_au);
	add_command ("ad",   cmd_ad);
	add_command ("al",   cmd_al);
	add_command ("ar",   cmd_ar);

	add_command ("tt",   cmd_tt);
	add_command ("tb",   cmd_tb);
	add_command ("tl",   cmd_tl);
	add_command ("tr",   cmd_tr);
	
	add_command ("[", cmd_goto);

	/* file */
	add_command ("ce",   cmd_ce);
	add_command ("pw",   cmd_pw);
	add_command ("pn",   cmd_pn);
	add_command ("pwd",  cmd_pwd);
	add_command ("ro",   cmd_ro);

	/* pad */
	add_command ("1",    cmd_goto_line);
	add_command ("2",    cmd_goto_line);
	add_command ("3",    cmd_goto_line);
	add_command ("4",    cmd_goto_line);
	add_command ("5",    cmd_goto_line);
	add_command ("6",    cmd_goto_line);
	add_command ("7",    cmd_goto_line);
	add_command ("8",    cmd_goto_line);
	add_command ("9",    cmd_goto_line);
	add_command ("=",    cmd_position);
	add_command ("ph",   cmd_ph);
	add_command ("pp",   cmd_pp);
	add_command ("pt",   cmd_pt);
	add_command ("pb",   cmd_pb);
	add_command ("pv",   cmd_pv);
	add_command ("tn",   cmd_tn);

	/* text */
	add_command ("es",   cmd_es);
	add_command ("er",   cmd_er);
	add_command ("en",   cmd_en);
	add_command ("ee",   cmd_ee);
	add_command ("ed",   cmd_ed);
	add_command ("ei",   cmd_ei);

	/* search */
	add_command("/", cmd_search);
//	add_command("\\", cmd_search_backward);

	/* system */
	add_command ("!",    cmd_bang);

	/* navigation */
	add_command ("tdm", cmd_tdm);
	add_command ("wc",  cmd_wc);

	/* key */
	add_command ("kd",  cmd_kd);

	/* command */
	add_command ("msg", cmd_msg);
	add_command ("cloop", cloop); /* testing */

}

int get_index (char a)
{
	return isalpha (a) ? tolower (a) - 'a' : 26;
}

command_t *find_command (const char *name)
{
	command_t *mover;
	int a;

	a = get_index (*name);
	mover = command_ptrs[a];

	while (mover) {
		if (a == 26) {
			if (*name == *(mover->name))
				return mover;
		} else {
			if (!strcasecmp (name, mover->name))
				return mover;
		}
		mover = mover->next;
	}

	return NULL;
}

void execute_command (int argc, char *argv[])
{
	command_t *c;

	if (sigsetjmp (env, 1))
	{
		done_executing ();
		output_message ("Aborted execution of command");
		return;
	}

	if (strlen (argv[0]) == 0)
		return;

	c = find_command (*argv);

	if (c)
	{
		set_executing ("%s command", c->name);
		(c->func) (argc, argv);
		done_executing ();

		if (e->occupied_window == COMMAND_WINDOW_REQUEST)
			e->occupied_window = COMMAND_WINDOW;
	}
	else
		output_message_c (*argv, "Unknown command");
}

void add_command (const char *name, void (*func) ())
{
	command_t *new;
	int     a;

	new = find_command (name);

	if (new)
	{
		new->func = func;
		return;
	}

	new = ALLOC (command_t);
	new->name = strdup (name);
	new->func = func;

	a = get_index (*name);

	new->next = command_ptrs[a];
	command_ptrs[a] = new;
}

void remove_command (char *name)
{
	command_t *mover, *prev;
	int     a;

	a = get_index (*name);

	mover = command_ptrs[a];
	prev = NULL;

	while (mover)
	{
		if (!strcasecmp (name, mover->name))
		{
			if (prev == NULL)
				command_ptrs[a] = mover->next;
			else
				prev->next = mover->next;

			free (mover->name);
			FREE (mover);

			debug ("Command removed: %s\n", name);

			return;
		}

		prev = mover;
		mover = mover->next;
	}

	debug ("Tried to remove unknown command: %s\n", name);
}
