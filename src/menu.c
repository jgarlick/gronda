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
 *  menu.c
 *  purpose : Functions to allow dynamic menu generation
 *  authors : James Garlick
 */
#include <string.h>
#include <stdarg.h>

#include "editor.h"

menu_t *
menu_alloc (char *format, ...)
{
	menu_t *new;
	char   *str;
	va_list argp;

	new = ALLOC (menu_t);
	if (new == NULL)
	{
		output_message ("Out of memory: menu_alloc");
		return NULL;
	}

	va_start (argp, format);
	vasprintf (&str, format, argp);
	va_end (argp);

	new->title = str;

	return new;
}

void
menu_free (menu_t * m)
{
	menu_item_t *mi, *next;

	if (m == NULL)
		return;

	if (m->title)
		free (m->title);

	mi = m->item_head;

	while (mi)
	{
		next = mi->next;

		if (mi->text)
			free (mi->text);

		FREE (mi);

		mi = next;
	}

	FREE (m);
}

void
menu_add_item (menu_t * m, char *text, char quick_key, void (*handler) ())
{
	menu_item_t *mi, *new;

	if (m == NULL)
		return;

	new = ALLOC (menu_item_t);
	if (new == NULL)
	{
		output_message ("Out of memory: menu_add_item");
		return;
	}

	new->text = strdup (text);
	new->quick_key = quick_key;
	new->handler = handler;

	if (m->item_head == NULL)
	{
		m->item_head = new;
	}
	else
	{
		mi = m->item_head;

		while (mi->next != NULL)
			mi = mi->next;

		mi->next = new;
	}


}
