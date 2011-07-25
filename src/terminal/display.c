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
#include "include/terminal.h"

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
