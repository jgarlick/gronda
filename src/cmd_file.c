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

#define CE_BUF_SIZE (1024)


/* create edit */
void cmd_ce (int argc, char *argv[])
{
	FILE        *f;
	char        tmp[CE_BUF_SIZE];
	string_t    *buf;
	char        *c;
	struct stat st_buf;

	char   *args[2];

	if (argc == 1)
	{
		output_message_c (argv[0], "Missing File Name");
		return;
	}

	if (crash_file_check (argv[1]))
		return;

	stat (argv[1], &st_buf);
	if (S_ISDIR (st_buf.st_mode))
	{
		output_message ("%s : is a directory", argv[1]);
		return;
	}

	f = fopen (argv[1], "r");
	if (!f && (errno != ENOENT))
	{
		output_message_c (argv[0], "%s", strerror(errno));
		return;
	}

	e->cpad->filename = strdup (argv[1]);
	e->redraw |= (DIRTY_ALL | TITLE | STATS);

	if (!f)
	{
		e->cpad->flags |= FILE_WRITE;
		return;
	}

	e->cpad->curs_x   = 1;
	e->cpad->curs_y   = 1;
	e->cpad->flags |= FILE_WRITE;

	buf = string_alloc ("");

	while (!feof (f))
	{
		if (fgets (tmp, CE_BUF_SIZE, f) == NULL)
			continue;

		c = tmp;
		while (*c != '\n' && *c)
			c++;

		if (*c == '\n')
		{
			*c = 0;
			string_append (buf, "%s", tmp);

			args[0] = "es";
			args[1] = buf->data;
			cmd_es (2, args);

			args[0] = "ad";
			args[1] = "-s";
			cmd_ad (2, args);
			cmd_tl (0, NULL);

			string_truncate (buf, 0);
		}
		else
			string_append (buf, "%s", tmp);

	}

	if (strlen (buf->data) > 0)
	{
		args[0] = "es";
		args[1] = buf->data;
		cmd_es (2, args);
	}

	string_free (buf);

	fclose (f);

	e->cpad->curs_x   = 1;
	e->cpad->curs_y   = 1;
	e->cpad->offset_y = 0;
	e->cpad->flags    = 0;
	e->cpad->flags    = 0;

	/* set the read/write / read only mode */
	if (access (e->cpad->filename, W_OK) != -1)
		e->cpad->flags |= FILE_WRITE;
	else
		e->cpad->flags &= ~FILE_WRITE;
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

/* print working directory */
void cmd_pwd (int argc, char *argv[])
{
	output_message (getenv ("PWD"));
}

/* read only */
void cmd_ro (int argc, char *argv[])
{
	pad_t  *p = e->cpad;
	int     r;

	if (argc == 1)
	{
		p->flags ^= FILE_WRITE;
	}
	else
	{
		if (!strcasecmp (argv[1], "on"))
		{
			p->flags |= FILE_WRITE;
		}
		else if (!strcasecmp (argv[1], "off"))
		{
			p->flags &= ~FILE_WRITE;
		}
	}
	if (p->flags & FILE_WRITE)
	{
		if (p->filename != NULL)
		{
			r = access (p->filename, W_OK);

			if (r == -1 && errno != ENOENT)
			{
				p->flags &= ~FILE_WRITE;
				output_message_c (argv[0], "Permission denied");
			}
		}
	}

	e->redraw |= STATS;
}


