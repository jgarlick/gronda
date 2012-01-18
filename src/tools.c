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
 *  tools.c
 *  purpose : various utility functions
 *  authors : James Garlick
 */
#include <stdio.h>
#include <stdarg.h>
#include <termios.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "editor.h"

void    yy_scan_string (char *);
int     yylex (void);

void debug (char *str, ...)
{
	va_list argp;

	if (!e || e->flags & DEBUG)
	{
		va_start (argp, str);
		fprintf (stderr, "[stderr] ");
		vfprintf (stderr, str, argp);
		fprintf (stderr, "\n");
		va_end (argp);
	}
}

void parse (const char *format, ...)
{
	char   *str;
	va_list argp;
	line_t *line;

	if (e->occupied_window == COMMAND_WINDOW) {
		e->cpad = e->input_pad;
	} else {
		e->cpad = e->cepad;
	}

	va_start (argp, format);
	vasprintf (&str, format, argp);
	va_end (argp);

	yy_scan_string (str);
	yylex ();
	free (str);

	if (e->occupied_window == COMMAND_WINDOW_EXECUTE) {
		e->occupied_window = EDIT_WINDOW;
		line = e->input_pad->line_head->prev->prev;
		if (line != e->input_pad->line_head && line->str && line->str->data)
			parse(line->str->data);
	}
}

void output_message (char *format, ...)
{
	char *buf;

	va_list argp;

	va_start (argp, format);
	vasprintf (&buf, format, argp);
	va_end (argp);

	LINE_append(e->output_pad, buf);
	e->redraw |= OUTPUT;

	free(buf);
}

#ifdef HPUX

/* 
Memory allocating vsprintf (from FreeBSD)
*/

#ifdef __STDC__
#define PTR void *
#else
#define PTR char *
#endif

unsigned long strtoul ();
//char *malloc ();

static int
int_vasprintf (result, format, args)
	 char  **result;
	 const char *format;
	 va_list *args;
{
	const char *p = format;
	/* Add one to make sure that it is never zero, which might cause malloc
	   to return NULL.  */
	int     total_width = strlen (format) + 1;
	va_list ap;

	memcpy ((PTR) & ap, (PTR) args, sizeof (va_list));

	while (*p != '\0')
	{
		if (*p++ == '%')
		{
			while (strchr ("-+ #0", *p))
				++p;
			if (*p == '*')
			{
				++p;
				total_width += abs (va_arg (ap, int));
			}
			else
				total_width += strtoul ((char *) p, (char **) &p, 10);
			if (*p == '.')
			{
				++p;
				if (*p == '*')
				{
					++p;
					total_width += abs (va_arg (ap, int));
				}
				else
					total_width += strtoul ((char *) p, (char **) &p, 10);
			}
			while (strchr ("hlL", *p))
				++p;
			/* Should be big enough for any format specifier except %s.  */
			total_width += 30;
			switch (*p)
			{
			case 'd':
			case 'i':
			case 'o':
			case 'u':
			case 'x':
			case 'X':
			case 'c':
				(void) va_arg (ap, int);
				break;
			case 'f':
			case 'e':
			case 'E':
			case 'g':
			case 'G':
				(void) va_arg (ap, double);
				break;
			case 's':
				total_width += strlen (va_arg (ap, char *));
				break;
			case 'p':
			case 'n':
				(void) va_arg (ap, char *);
				break;
			}
		}
	}
	*result = (char *) malloc (total_width);
	if (*result != NULL)
		return vsprintf (*result, format, *args);
	else
		return 0;
}

int
vasprintf (result, format, args)
	 char  **result;
	 const char *format;
	 va_list args;
{
	return int_vasprintf (result, format, &args);
}
#endif

int parse_commandfile (char *path)
{
	FILE   *f;
	char    str[4096];
	char   *c;
	int     lines;

	f = fopen (path, "r");
	if (!f)
		return -1;

	lines = 0;
	while (fgets (str, 4096, f))
	{
		c = str;
		while (*c != '\n' && *c)
			c++;
		*c = 0;

		c = str;
		while (*c == ' ')
			c++;

		if (*c && *c != '#')
		{
			parse ("%s", c);
			lines++;
		}
	}

	fclose (f);

	return lines;
}

void create_dir (char *name)
{
	struct stat buf;

	if (stat (name, &buf) == -1)
	{
		if (errno == ENOENT)
		{
			mkdir (name, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
		}
		else
		{
			perror (name);
			exit (1);
		}
	}
}

void create_local_config ()
{
	char  temp[256];
    char *home;

	home = getenv ("HOME");

	if (home == NULL)
		return;


	sprintf (temp, "%s/.gronda", home);
	create_dir (temp);

	sprintf (temp, "%s/.gronda/buffer", home);
	create_dir (temp);
}

/* get actual string position on a line in characters,   */
/* from cursor co-ordinate and pad offset, accounting    */
/* for the posibility of tabs                            */
/* if position x is inside a tab, intab is set to the    */
/* number of characters offset from the start of the tab */
int get_string_pos (int x, char *str, int *intab)
{
	int  a;
	char *c;
	int  correction;

	x--;

	a = 0;
	c = str;
	correction = 0;

	*intab = 0;

	while (a < x && *c)
	{
		if (*c == '\t')
		{
			(*intab)++;
			if (*intab == 4) /* replace with next tab stop */
			{
				*intab = 0;
				correction += 3;
				c++;
			}
		}
		else
			c++;

		a++;
	}

	return (x - correction) - *intab;
}

/* get the horizontal cursor position for the given string */
/* position on a line, accounting for tabs etc             */
/* TODO: why doesn't this use get_string_pos? */
int get_curs_pos (int x, line_t *l)
{
	int  len;
	char *c;
	int  a;

	if (l == NULL || l->str == NULL)
		return x;

	c   = l->str->data;
	a   = 0;
	len = 0;

	while (a < x)
	{
		if (*c == '\t')
			len += 4; /* this is temporary, replace with tabstops */
		else
			len++;

		a++;
		if (*c)
			c++;
	}

	return len + 1;
}

/* attempt to set the cursor position to the given y, x position */
/* if we end up in the middle of a wide character such as a tab, */
/* adjust the cursor position either left or right depending on  */
/* the value of 'adjust'                                         */
void cursor_set_pos (pad_t *p, int curs_y, int curs_x, int adjust)
{
	line_t *l;
	char   *c;
	int    a;
	int    width;
	int    offset;
	int    x;

	l = LINE_get_line_at (p, p->offset_y + curs_y);

	p->curs_y = curs_y;
	p->curs_x = curs_x;

	if (l != NULL && l->str != NULL)
	{
		c = l->str->data;
		x = p->offset_x + curs_x;

		a = 0;
		while (*c && a < x - 1)
		{
			if (*c == '\t')
			{
				width = 4;
				a += width; /* TODO: tab stops */
			}
			else
				a++;

			if (a < x) c++;
		}

		if (*c)
		{
			offset = x - a - 1;
			if (offset != 0)
			{
				offset += width;
				if (adjust == ADJUST_LEFT)
					p->curs_x -= offset;
				else /* adjust == ADJUST_RIGHT */
					p->curs_x += (width - offset);
			}
		}
	}

}

void move_cursor_into_view(pad_t *pad) {
	if (pad->curs_y < 1) {
		pad->offset_y += (pad->curs_y - 1);
		pad->curs_y = 1;
	} else if (pad->curs_y > pad->height) {
		pad->offset_y += (pad->curs_y - pad->height);
		pad->curs_y = pad->height;
	}

	if (pad->curs_x < 1) {
		pad->offset_x += (pad->curs_x - 1);
		pad->curs_x = 1;
	} else if (pad->curs_x > pad->width) {
		pad->offset_x += (pad->curs_x - pad->width);
		pad->curs_x = pad->width;
	}
}

/* takes the text for a line and converts it into output suitable for the */
/* viewport by converting tabs into the appropriate number of spaces      */
/* in_str - the text for the line from the pad x offset onwards           */
/* intab  - how far we are already inside a tab at this x offset          */
/* width  - the width of the viewport                                     */
/* out_str - pre-allocated string for the output                          */
void get_string_for_viewport(char *in_str, int intab, int width, char *out_str) {
	char *ptr;
	int j;
	
	ptr = out_str;
	if (intab > 0)
		intab = 4 - intab;
		
	for (j = 0; j < width; j++) {
		if (!intab && *in_str == '\t') {
			intab = 4;
			in_str++;
		}
		if (intab) {
			*ptr = ' ';
			ptr++;
			intab--;
		} else if (*in_str) {
			*ptr = *in_str;
			ptr++;
			in_str++;
		}
	}
	*ptr = '\0';
}
