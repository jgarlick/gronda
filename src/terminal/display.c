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
 *  display.c
 *  purpose : Terminal based editor front-end using curses
 *  authors : James Garlick
 */
#include <curses.h>
#include <term.h>

#include <sys/ioctl.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>

#include "../include/editor.h"
#include "include/display.h"

struct ttysize dimensions;
int lineno;
int showtabs;

void resize_handler ()
{
	debug ("ORIGINAL: %dx%d", dimensions.ts_cols, dimensions.ts_lines);
	if (ioctl (0, TIOCGSIZE, &dimensions) < 0)
	{
		dimensions.ts_lines = 24;
		dimensions.ts_cols = 80;
	}
	debug ("RESIZE: %dx%d", dimensions.ts_cols, dimensions.ts_lines);

//	resizeterm (dimensions.ts_lines, dimensions.ts_cols);
	resize ();
}

void next_event ()
{
	int keycode;
	int shift;
	int ctrl;
	char buffer[80];
	
	shift = 0;
	ctrl  = 0;

	keycode = getch ();

	debug("KEYCODE %d", keycode);

	/* shift + function keys */
	if (keycode >= 277 && keycode <= 284) {
		shift = 1;
		keycode -= 12;
	}

	/* ctrl + letter */
	if (keycode < 26 && keycode != 8 && keycode != 9 && keycode != 13) {
		ctrl = 1;
		keycode += 96;
	}

	switch (keycode)
	{
	case KEY_RESIZE:
		resize_handler ();
		e->key[0] = 0;
		break;
	case KEY_UP:
		strcpy(e->key, "up");
		break;
	case KEY_DOWN:
		strcpy(e->key, "down");
		break;
	case KEY_LEFT:
		strcpy(e->key, "left");
		break;
	case KEY_RIGHT:
		strcpy(e->key, "right");
		break;

	case KEY_IC:
		strcpy(e->key, "ins");
		break;
	case KEY_DC:
		strcpy(e->key, "del");
		break;
	case KEY_BACKSPACE:
		strcpy(e->key, "bs");
		break;
	case KEY_HOME:
		strcpy(e->key, "home");
		break;
	case KEY_END:
		strcpy(e->key, "end");
		break;
	case KEY_PPAGE:
		strcpy(e->key, "pgup");
		break;
	case KEY_NPAGE:
		strcpy(e->key, "pgdown");
		break;
	case KEY_F (1):
		strcpy(e->key, "F1");
		break;
	case KEY_F (2):
		strcpy(e->key, "F2");
		break;
	case KEY_F (3):
		strcpy(e->key, "F3");
		break;
	case KEY_F (4):
		strcpy(e->key, "F4");
		break;
	case KEY_F (5):
		strcpy(e->key, "F5");
		break;
	case KEY_F (6):
		strcpy(e->key, "F6");
		break;
	case KEY_F (7):
		strcpy(e->key, "F7");
		break;
	case KEY_F (8):
		strcpy(e->key, "F8");
		break;
	case '\'':
		strcpy(e->key, "squote");
		break;
	case '\"':
		strcpy(e->key, "dquote");
		break;

		/* TODO - make these terminal independant */
	case 27:
		strcpy(e->key, "esc");
		break;
	case 13:
		strcpy(e->key, "enter");
		break;
	case 9:
		strcpy(e->key, "tab");
		break;
	case 8:
	case 127:
		strcpy(e->key, "bs");
		break;
	default:
		sprintf(e->key, "%c", keycode);	
	}
	
	if (shift) {
		strcpy(buffer, e->key);
		sprintf(e->key, "%sS", buffer);
	}
	if (ctrl) {
		strcpy(buffer, e->key);
		sprintf(e->key, "^%s", buffer);
	}
}


void display_max_pad_dim (int *width, int *height)
{
	if (lineno)
		*width = dimensions.ts_cols - 6;
	else
		*width = dimensions.ts_cols;

	*height = dimensions.ts_lines - 2;
}

void cmd_delay (int argc, char *argv[])
{
	int     delay = atoi (argv[1]);

	ESCDELAY = delay;

	output_message_c (argv[0], "Esc delay changed to %d ms", delay);
	debug ("Delay changed to %s milliseconds", argv[1]);
}

void cmd_lineno (int argc, char *argv[])
{
	if (argc > 1)
	{
		if (strcmp (argv[1], "-on") == 0)
		{
			lineno = 1;
		}
		else if (strcmp (argv[1], "-off") == 0)
		{
			lineno = 0;
		}
	}
	else
		lineno = !lineno;

	resize ();
}

void cmd_showtabs (int argc, char *argv[])
{
	if (argc > 1)
	{
		if (strcmp (argv[1], "-on") == 0)
		{
			showtabs = 1;
		}
		else if (strcmp (argv[1], "-off") == 0)
		{
			showtabs = 0;
		}
	}
	else
		showtabs = !showtabs;

	redraw ();
}

void display_init ()
{
	initscr ();
	raw ();
	noecho ();

	nonl ();
	intrflush (stdscr, FALSE);
	keypad (stdscr, TRUE);

	start_color ();
	use_default_colors ();

	lineno   = 1;
	showtabs = 0;

	resize_handler ();

	if (getenv ("ESCDELAY") == NULL)
		ESCDELAY = 50;

	add_command ("delay",    cmd_delay);
	add_command ("lineno",   cmd_lineno);
	add_command ("showtabs", cmd_showtabs);

	init_pair (1, COLOR_CYAN,    -1);
	init_pair (2, COLOR_RED,     -1);
	init_pair (3, COLOR_MAGENTA, -1);
	init_pair (4, COLOR_GREEN,   -1);
	init_pair (5, COLOR_WHITE, COLOR_GREEN);

}

void display_fini ()
{
	nocbreak ();
	endwin ();
}
          
void display_beep ()
{
	beep ();
};

void display_redraw_title ()
{
	debug ("display_redraw_title");

	attron (A_REVERSE);

	mvprintw (0, 0, "%-*.*s", dimensions.ts_cols, dimensions.ts_cols, " ");

	if (e->cpad->filename)
		mvprintw (0, 0, " %s", e->cpad->filename);
	else
		mvprintw (0, 0, " (new file)");

	attroff (A_REVERSE);

}

void display_redraw_stats ()
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

void display_redraw_command ()
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

void display_redraw_output ()
{
	int     width;

	width = (dimensions.ts_cols / 2) - 2;

	attron (A_REVERSE);
	if (e->messages)
		mvprintw (dimensions.ts_lines - 1, dimensions.ts_cols / 2, "| %-*.*s",
				  width, width, e->messages->message);
	else
		mvprintw (dimensions.ts_lines - 1, dimensions.ts_cols / 2, "| %-*.*s",
				  width, width, "");
	attroff (A_REVERSE);
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

void display_redraw_line (int y, line_t *l)
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

void display_redraw_curs ()
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

void display_do_menu (menu_t * menu)
{
	int     title_count, item_count, count;
	menu_item_t *mi;
	int     width, len, index, done;
	int     a;
	int     x, y;
	char   *s, *t;
	int     selected, old_selected;

	debug ("starting menu...");

	if (menu == NULL || menu->item_head == NULL)
		return;

	width = 10;

	/* find the number of title lines and expand width if necessary */

	len = 0;
	title_count = 0;

	s = menu->title;
	if (*s)
		title_count++;

	while (*s)
	{
		if (*s == '\n')
		{
			debug ("TITLE len %d", len);
			if (len > width)
				width = len;
			title_count++;
			len = 0;
		}
		else
			len++;

		s++;
	}
	if (len > width)
		width = len;

	/* find the number of items and expand width if necessary */

	item_count = 0;
	mi = menu->item_head;

	while (mi)
	{
		item_count++;
		len = strlen (mi->text);
		if (len > width)
			width = len;

		mi = mi->next;
	}

	/* increase width to account for border */
	width += 4;

	/* total number of lines for the menu */
	count = 3 + title_count + item_count;

	/* starting coords */
	x = (dimensions.ts_cols / 2) - (width / 2);
	y = (dimensions.ts_lines / 2) - (count / 2);

	/* draw menu */

	/* top line */
	mvaddch (y, x, ACS_ULCORNER);
	for (a = 2; a < width; a++)
		addch (ACS_HLINE);
	addch (ACS_URCORNER);

	/* title */
	t = s = menu->title;
	for (a = 0; a < title_count; a++)
	{
		while (*s && *s != '\n')
			s++;

		*s++ = 0;

		mvaddch (y + a + 1, x, ACS_VLINE);
		printw (" %-*.*s ", width - 4, width - 4, t);
		addch (ACS_VLINE);

		t = s;
	}

	/* empty line */
	mvaddch (y + title_count + 1, x, ACS_LTEE);
	for (a = 2; a < width; a++)
		addch (ACS_HLINE);
	addch (ACS_RTEE);

	/* items */
	mi = menu->item_head;
	for (a = 0; a < item_count; a++)
	{
		mvaddch (y + title_count + 2 + a, x, ACS_VLINE);
		if (a == 0)
			attron (A_REVERSE);
		printw (" %-*.*s ", width - 4, width - 4, mi->text);
		if (a == 0)
			attroff (A_REVERSE);
		addch (ACS_VLINE);

		mi = mi->next;
	}

	/* bottom line */
	mvaddch (y + count - 1, x, ACS_LLCORNER);
	for (a = 2; a < width; a++)
		addch (ACS_HLINE);
	addch (ACS_LRCORNER);

	curs_set (0);

	refresh ();

	old_selected = selected = 1;
	done = 0;
	while (done == 0)
	{
		next_event ();

		if (!strcmp(e->key, "esc")) {
			done = 1;
		} else if (!strcmp(e->key, "up")) {
			if (selected > 1)
				selected--;
		} else if (!strcmp(e->key, "down")) {
			if (selected < item_count)
				selected++;
		} else if (!strcmp(e->key, "enter")) {
			done = selected;
		}

		/* change selection */

		if (selected != old_selected)
		{
			mi = menu->item_head;
			for (a = 0; a < item_count; a++)
			{
				if (a == (selected - 1))
				{
					attron (A_REVERSE);
					mvprintw (y + title_count + 2 + a, x + 1,
							  " %-*.*s ", width - 4, width - 4, mi->text);
					attroff (A_REVERSE);
				}
				else if (a == (old_selected - 1))
				{
					mvprintw (y + title_count + 2 + a, x + 1,
							  " %-*.*s ", width - 4, width - 4, mi->text);
				}

				mi = mi->next;
			}
			old_selected = selected;
		}

		/* search for hotkey */

		mi = menu->item_head;
		index = 0;
		while (mi)
		{
			index++;
			if ((char) *(e->key) == mi->quick_key || index == done)
			{
				(mi->handler) (mi->text, index);
				done = 1;
				break;
			}

			mi = mi->next;
		}

	}
	e->redraw |= DIRTY_ALL;
	curs_set (1);
	debug ("Finished menu");
}

/* this is called if a menu items function handler crashes
   while executing
 */
void display_finish_menu ()
{
	e->redraw |= DIRTY_ALL;
	curs_set (1);
	debug ("Finished menu");
}
