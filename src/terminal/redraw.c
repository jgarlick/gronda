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
 *  redraw.c
 *  purpose : Setting up display modifiers and high-level redraw
 *            Basic implementation of syntax highlighting for testing purposes
 *  authors : James Garlick
 */
#include <curses.h>
#include <term.h>

#include <string.h>
#include <ctype.h>

#include "../include/editor.h"
#include "include/terminal.h"

void syntax_highlight_test ();

void redraw ()
{
	pad_t  *p = e->cpad;
	int     a;
	line_t *l;

	int start_y;
	int start_x;
	int end_y;
	int end_x;
	int len;
	int end;

	if (p->echo)
	{
		get_region (p->echo, &start_y, &start_x, &end_y, &end_x);

		start_y -= p->offset_y + 1;
		start_x -= p->offset_x + 1;
		end_y   -= p->offset_y + 1;
		end_x   -= p->offset_x + 1;
	}

	/* redraw title */
	if (e->redraw & TITLE)
		redraw_title ();

	/* redraw stats */
	if (e->redraw & STATS)
		redraw_stats ();

	/* redraw output window */
	if (e->redraw & OUTPUT)
		redraw_output ();

	/* redraw command window */
	if (e->redraw & COMMAND)
		redraw_command ();

	/* redraw lines */
	l = LINE_get_line_at (p, p->offset_y + 1);
	if (l == NULL)
		l = p->line_head;

	dm_clear_all ();
	/* syntax_highlight_test (); */

	for (a = 0; a < p->height; a++)
	{
		if (l != p->line_head)
		{
			if (l->str != NULL)
				len = get_curs_pos (strlen (l->str->data), l) - p->offset_x;
			else
				len = 1;
		}
		else
			len = 1;

		/* region highlight */
		if (p->echo == REGION_LINEAR)
		{
			if (a == start_y)
			{
				if (!(start_y == end_y && start_x == end_x))
				{
					if (start_x < len)
					{
						dm_set (a, start_x, INVERT_ON);
	
						if (start_y == end_y)
						{
							end = (len < end_x) ? len : end_x;
							dm_set (a, end, INVERT_OFF);
						}
						else
							dm_set (a, len, INVERT_OFF);
					}
				}
			} 
			else if (a > start_y && a < end_y)
			{
				dm_set (a, 0, INVERT_ON);
				dm_set (a, len, INVERT_OFF);

			}
			else if (a == end_y)
			{
				if (end_x != 0)
				{
					dm_set (a, 0, INVERT_ON);
	
					end = (len < end_x) ? len : end_x;
					dm_set (a, end, INVERT_OFF);
				}
			}
		}
		else if (p->echo == REGION_RECT)
		{
			if (!(start_x == end_x) && a >= start_y && a <= end_y)
			{
				dm_set (a, start_x, INVERT_ON);
				dm_set (a, end_x,   INVERT_OFF);
			}
		}

		redraw_line (a, l);

		if (l != p->line_head)
			l = l->next;
	}

	e->redraw = 0;
	memset (p->line_redraw, p->height, CLEAN);

	redraw_curs ();
}

void redraw_title ()
{
	attron (A_REVERSE);

	mvprintw (0, 0, "%-*.*s", dimensions.ts_cols, dimensions.ts_cols, " ");

	if (e->cpad->filename)
		mvprintw (0, 0, " %s", e->cpad->filename);
	else
		mvprintw (0, 0, " (new file)");

	attroff (A_REVERSE);

}

void redraw_stats ()
{
	if (!(e->cpad->flags & FILE_WRITE))
		mvprintw (0, dimensions.ts_cols - 4, "R");
	else if (e->flags & INSERT)
		mvprintw (0, dimensions.ts_cols - 4, "I");
	else
		mvprintw (0, dimensions.ts_cols - 4, "O");

	if (e->cpad->flags & MODIFIED)
		mvprintw (0, dimensions.ts_cols - 2, "M");
	else
	{
		attron (A_REVERSE);
		mvprintw (0, dimensions.ts_cols - 2, " ");
		attroff (A_REVERSE);
	}
}

void redraw_command ()
{
	int     width;

	if (e->occupied_window == COMMAND_WINDOW)
		curs_set (1);

	width = (dimensions.ts_cols / 2) - 9;

	attron (A_REVERSE);
	mvprintw (dimensions.ts_lines - 1, 0, "Command: %-*.*s", width, width,
			  e->input->buffer);
	attroff (A_REVERSE);
}

void redraw_output ()
{
	int     width;
	line_t  *line;

	width = (dimensions.ts_cols / 2) - 2;
	line  = e->output_pad->line_head->prev;

	attron (A_REVERSE);
	if (line->str && line->str->data)
		mvprintw (dimensions.ts_lines - 1, dimensions.ts_cols / 2, "| %-*.*s",
				  width, width, line->str->data);
	else
		mvprintw (dimensions.ts_lines - 1, dimensions.ts_cols / 2, "| %-*.*s",
				  width, width, "");
	attroff (A_REVERSE);
}

void redraw_line (int y, line_t *l)
{
	int     x;
	int     c_dm;				/* current display modifier */
	int     colour, invert;
	int     intab;
	int     offset;

	char   *ptr;
	char   *str;

	char   line_num[10];

	intab  = 0;

	if (l == e->cpad->line_head)
	{
		if (lineno)
			str = "";
		else
			str = "~";
	}
	else if (l == NULL || l->str == NULL)
		str = "";
	else
	{
		offset = get_string_pos (e->cpad->offset_x + 1, l->str->data, &intab);
		if ((size_t)offset < strlen (l->str->data))
			str = l->str->data + offset;
		else
			str = "";
	}

	move (y + 1, 0);

	if (lineno)
	{
		if (l == e->cpad->line_head)
			sprintf (line_num, "%5s", "");
		else
			sprintf (line_num, "%5d", e->cpad->offset_y + y + 1);

		addstr (line_num);
		addch (ACS_VLINE);
	}

	x      = 0;
	ptr    = str;
	colour = 0;
	invert = 0;

	while (x < e->cpad->width)
	{
		if (e->cpad->echo && e->occupied_window == EDIT_WINDOW)
		{
			curs_set (0);
			if (y == e->cpad->curs_y - 1 && x == e->cpad->curs_x - 1)
				attron (A_UNDERLINE);

			if (y == e->cpad->curs_y - 1 && x == e->cpad->curs_x)
				attroff (A_UNDERLINE);
		}
		else
			curs_set (1);

		c_dm = *(e->dm + (y * (e->cpad->width + 1)) + x);
		if (c_dm != 0)
			apply_dm (c_dm, &colour, &invert);

		if (*ptr)
		{
			if (*ptr == '\t')
			{
				intab++;

				if (showtabs)
				{
					if (intab == 1)
						addch (ACS_ULCORNER);
					else if (intab == 4)
						addch (ACS_URCORNER);
					else
						addch (ACS_HLINE);
				}
				else
					addch (' ');

				/* TODO: replace 4 here with next tab stop */
				if (intab == 4)
				{
					ptr++;
					intab = 0;
				}
			}
			else if (isgraph (*ptr) || *ptr == ' ')
			{
				addch (*ptr);
				ptr++;
			}
			else
			{
				addch (ACS_DIAMOND);
				ptr++;
			}
		}
		else
			addch (' ');

		x++;
	}
	c_dm = *(e->dm + (y * (e->cpad->width + 1)) + x);
	if (c_dm != 0)
		apply_dm (c_dm, &colour, &invert);

/*	if (x < e->cpad->width)
		clrtoeol ();*/
	if (invert)
		attroff (A_REVERSE);
/*		attroff (COLOR_PAIR(5));*/
	if (colour)
		attroff (COLOR_PAIR (colour));
	if (y == e->cpad->curs_y - 1 && x == e->cpad->curs_x)
		attroff (A_UNDERLINE);

}

void redraw_curs ()
{    
	int offset = 0;

	if (e->occupied_window == EDIT_WINDOW)
	{
		if (lineno)
			offset = 6;

		move (e->cpad->curs_y, e->cpad->curs_x + offset - 1);
	}
	else
		move (dimensions.ts_lines - 1, 9 + strlen (e->input->buffer));
}

void apply_dm (int c_dm, int *colour, int *invert)
{
	if (c_dm & INVERT_ON)
	{
		attron (A_REVERSE);
/*		attron (COLOR_PAIR(5));*/
		*invert = 1;
	}
	else if (c_dm & INVERT_OFF)
	{
		attroff (A_REVERSE);
/*		attroff (COLOR_PAIR(5));*/
		*invert = 0;
	}
	if (c_dm & SYNTAX0)
	{
		attroff (COLOR_PAIR (*colour));
		*colour = 0;
	}
	else if (c_dm & SYNTAX1)
	{
		attron (COLOR_PAIR (1));
		*colour = 1;
	}
	else if (c_dm & SYNTAX2)
	{
		attron (COLOR_PAIR (2));
		*colour = 2;
	}
	else if (c_dm & SYNTAX3)
	{
		attron (COLOR_PAIR (3));
		*colour = 3;
	}
	else if (c_dm & SYNTAX4)
	{
		attron (COLOR_PAIR (4));
		*colour = 4;
	}
}

void dm_set (int row, int col, int mod)
{
	if (row < 0) row = 0;
	if (row > e->pad_head->height) row = e->pad_head->height;
	if (col < 0) col = 0;
	if (col > e->pad_head->width) col = e->pad_head->width;

	*(e->dm + (row * (e->pad_head->width + 1)) + col) |= mod;
}

void dm_clear_all ()
{
	memset (e->dm, 0, (e->cpad->height + 1) * (e->cpad->width + 1) * 4);
}

/*
Types        1
Keywords     2
Literals     3
Operands     4
Precompiler  5
Comment      6
*/

char   *c_s1[] =
	{ "auto", "const", "enum", "extern", "register", "static", "struct",
	"typedef", "union", "volatile", "char", "double", "float", "int", "long",
	"short", "signed", "unsigned", "void", 0
};

char   *c_s2[] =
	{ "break", "case", "continue", "default", "do", "else", "for", "goto",
	"if", "return", "sizeof", "switch", "while", 0
};

static void highlight_words (char *line, int line_no, char **search, int highlight)
{
	pad_t  *p = e->cpad;
	char   *ptr;

	while (*search)
	{
		ptr = strstr (line, *search);

		while (ptr && ptr < line + p->width)
		{
			dm_set (line_no, (int) (ptr - line), highlight);
			dm_set (line_no, (int) (ptr - line) + strlen (*search), SYNTAX0);

			ptr = strstr (ptr + 1, *search);
		}
		search++;
	}
}

void syntax_highlight_test ()
{
	pad_t  *p = e->cpad;
	int     a;
	line_t *l;
	char   *s;

	/* syntax highlighting test */
	l = LINE_get_line_at (p, p->offset_y + 1);
	if (l == NULL)
		l = p->line_head;

	for (a = 0; a < p->height; a++)
	{
		if (l != p->line_head)
		{
			if (l->str == NULL || strlen (l->str->data) < (size_t)p->offset_x)
				continue;

			s = l->str->data;
			s += p->offset_x;

			highlight_words (s, a, c_s1, SYNTAX1);
			highlight_words (s, a, c_s2, SYNTAX2);

			l = l->next;
		}
	}
}
