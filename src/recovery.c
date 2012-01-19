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
 *  recovery.c
 *  purpose : crash file and error handling
 *  authors : James Garlick
 */
#include <signal.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <errno.h>

#include "editor.h"

char    executing_action[255];
sigjmp_buf env;

char    crash_file_name[255] = { 0 };

static void crash_file_handler (char *text, int index)
{
	char *args[2];
	int  len;

	if (index == 2) /* ignoring crash file */
	{
		/* restore the original file name */
		len = strlen (crash_file_name);

		if (len > 4 && !strcmp (crash_file_name + (len - 4), ".CRA"))
			crash_file_name[len - 4] = '\0';
	}

	args[0] = "ce";
	args[1] = crash_file_name;
	cmd_ce (2, args);
 
	if (index == 1) /* using crash file */
	{
		/* remove the crash file */
		unlink (crash_file_name);

		/* remove ".CRA" frome the filename */
		len = strlen (e->cpad->filename);

		if (len > 4 && !strcmp (e->cpad->filename + (len - 4), ".CRA"))
			e->cpad->filename[len - 4] = '\0';

		/* set pad to modified */
		e->cpad->flags |= MODIFIED;
	}

	crash_file_name[0] = '\0';
}

int crash_file_check (char *fname)
{
	struct stat st_buf;
	FILE        *f;
	menu_t      *m;

	if (crash_file_name[0] != '\0')
		return 0;

	sprintf (crash_file_name, "%s.CRA", fname);

	stat (crash_file_name, &st_buf);
	if (S_ISDIR (st_buf.st_mode))
		return 0;

	f = fopen (crash_file_name, "r");
	if (!f)
		return 0;

	fclose (f);

	m = menu_alloc ("A crash file exists for file:\n%s", fname);
	menu_add_item (m, "Use crash file",    'u', crash_file_handler);
	menu_add_item (m, "Ignore crash file", 'i', crash_file_handler);

	display_do_menu (m);

	menu_free (m);

	return 1;
}

/* attempt to write a crash file */
static void crash_file_write ()
{
	char    fname[255];
	FILE    *f;
	int     a;
	line_t  *mover;

	if (e && e->cpad 
		&& e->cpad->line_head 
		&& e->cpad->filename
		&& (e->cpad->line_count > 0)
		&& (e->cpad->flags & MODIFIED))
	{
		sprintf (fname, "%s.CRA", e->cpad->filename);

		f = fopen (fname, "w");
		if (f)
		{
			mover = e->cpad->line_head->next;

			for (a = 0; a < e->cpad->line_count; a++)
			{
				if (mover && mover != e->cpad->line_head)
				{
					if (mover->str != NULL)
						fputs (mover->str->data, f);

					fputc ('\n', f);

					mover = mover->next;
				}
			}

			fclose (f);

			printf ("\n** Crash file written! **\n");
		}
		else
			printf ("\n** Error opening crash file! **\n");
	}
}


void set_executing (char *format, ...)
{
	va_list argp;

	va_start (argp, format);
	vsnprintf (executing_action, 254, format, argp);
	va_end (argp);

	executing_action[254] = 0;

}

void done_executing ()
{
	executing_action[0] = 0;
}

void handle_interrupt_action (char *text, int index)
{
	switch (index)
	{
	case 1:
		display_finish_menu ();
		siglongjmp (env, 1);
		break;
	case 2:
		display_finish_menu ();
		cmd_wc (0, 0);
		break;
	}
}

void handle_interrupt (int dummy)
{
	menu_t  *m;

	if (*executing_action)
	{
		m = menu_alloc ("Software Interrupt received\nin command \'%s\'",
						executing_action);

		menu_add_item (m, "(A)bort execution of command", 'a',
					   handle_interrupt_action);
		menu_add_item (m, "(Q)uit", 'q', handle_interrupt_action);

		display_do_menu (m);

		menu_free (m);
	}
	else
		sig_cleanexit ("ctrl-c - %s (v%s)\n", EDITOR_NAME, EDITOR_VERSION);
}

void handle_error (int signal)
{
	char    s[100];

	switch (signal)
	{
	case SIGFPE:
		strcpy (s, "Floating point exception");
		break;
	case SIGILL:
		strcpy (s, "Illegal instruction exception");
		break;
	case SIGSEGV:
		strcpy (s, "Segmentation violation");
		break;
	case SIGBUS:
		strcpy (s, "Bus error");
		break;
	default:
		sprintf (s, "Unhandled signal (%d)", signal);
	}

	display_close ();

	crash_file_write ();

	printf ("%s received in core program (%s)\n",
		 s, (*executing_action) ? executing_action : "no command");

	exit (1);
}
