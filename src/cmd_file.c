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
 *  cmd_file.c
 *  purpose : File handling commands
 *  authors : James Garlick
 */
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>

#include "editor.h"

/* create edit */
void cmd_ce (int argc, char *argv[])
{
	struct stat st_buf;
	pad_t  *pad;
	int r;

	if (argc == 1) {
		output_message_c (argv[0], "Missing File Name");
		return;
	}

	r = stat (argv[1], &st_buf);
	if (r != 0 && errno != ENOENT) {
		output_message("%s : %s", argv[1], strerror(errno));
		return;
	}
	if (S_ISDIR (st_buf.st_mode))
	{
		output_message ("%s : is a directory", argv[1]);
		return;
	}

	pad = pad_add();

	if (!crash_file_check (pad, argv[1])) {
		pad_read_file(pad, argv[1]);
	}

}

/* pad write */
void cmd_pw (int argc, char *argv[])
{
	FILE   *f;
	line_t *mover;

	if (!(e->cpad->flags & MODIFIED))
		return;

	f = fopen (e->cpad->filename, "w");
	if (!f)
	{
		debug ("(pw) Could not write to file (pw)");
		return;
	}

	mover = e->cpad->line_head->next;

	while (mover != e->cpad->line_head)
	{
		if (mover->str != NULL)
			fputs (mover->str->data, f);

		mover = mover->next;

		if (mover != e->cpad->line_head)
			fputc ('\n', f);
	}

	fclose (f);

	e->cpad->flags &= ~(MODIFIED);
	e->redraw |= STATS;
}

/* pad name */
void cmd_pn (int argc, char *argv[])
{
	if (argc < 2)
	{
		debug ("(pn) Missing argument (pn)");
		return;
	}

	if (e->cpad->filename)
		free (e->cpad->filename);

	e->cpad->filename = strdup (argv[1]);
	e->cpad->flags |= MODIFIED;

	e->redraw |= (TITLE|STATS);
}

/* show working directory in output window */
/* TODO change directory with an argument to this */
void cmd_wd (int argc, char *argv[])
{
	output_message (getenv ("PWD"));
}

/* read only */
void cmd_ro (int argc, char *argv[])
{
	pad_t  *p = e->cpad;

	if (argc > 1) {
		if (!strcasecmp (argv[1], "on"))
			p->flags |= FILE_WRITE;
		else if (!strcasecmp (argv[1], "off"))
			p->flags &= ~FILE_WRITE;
	} else {
		p->flags ^= FILE_WRITE;
	}
	
	if ((p->flags & FILE_WRITE) && p->filename != NULL && access(p->filename, W_OK) == -1 && errno != ENOENT) {
		p->flags &= ~FILE_WRITE;
		output_message_c (argv[0], "Permission denied");
	}

	e->redraw |= STATS;
}


