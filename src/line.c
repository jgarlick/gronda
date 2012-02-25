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
 *  line.c
 *  purpose : Functions for line manipulation
 *  authors : James Garlick
 */

#include "editor.h"

#include <string.h>

/* initialise a new line list */
line_t *LINE_new_lines ()
{
	line_t *dummy;

	dummy = ALLOC (line_t);
	if (!dummy)
		return NULL;

	dummy->next = dummy;
	dummy->prev = dummy;

	dummy->str = NULL;

	return dummy;
}

void LINE_free_lines (line_t * head)
{
	line_t *mover = head, *next;

	while (mover->next != head)
	{
		next = mover->next;

		string_free (mover->str);
		FREE (mover);

		mover = next;
	}

	FREE (head);
}

/* Append a line to a line list  */
void LINE_append (pad_t * p, char *str)
{
	line_t *head;
	line_t *new;

	head = p->line_head;

	new = ALLOC (line_t);
	if (!new)
	{
		debug ("LINE_append: out of memory\n");
		return;
	}

	new->next = head;
	new->prev = head->prev;

	if (strlen (str))
		new->str = string_alloc ("%s", str);
	else
		new->str = NULL;

	head->prev = new;
	(new->prev)->next = new;

	p->line_count++;
}

/* return the number of lines in the list */
static int line_count (pad_t * p)
{
	line_t *head = p->line_head;

	line_t *mover = head->next;
	int     count = 0;

	while (mover != head)
	{
		count++;
		mover = mover->next;
	}

	return count;
}

/* insert an empty line at the given index */
void LINE_insert (pad_t * p, int index, char *str)
{
	int     count;
	line_t *mover, *new, *head;

	head = p->line_head;

	count = line_count (p);

	while (index > count)
	{
		LINE_append (p, "");
		count++;
	}

	mover = head;

	while (index--)
		mover = mover->next;

	new = ALLOC (line_t);
	if (!new)
	{
		debug ("LINE_append: out of memory\n");
		return;
	}

	new->next = mover->next;
	new->prev = mover;
	if (strlen (str))
		new->str = string_alloc ("%s", str);
	else
		new->str = NULL;

	mover->next = new;
	new->next->prev = new;

	p->line_count++;
}

void LINE_remove (pad_t * p, line_t * node)
{
	line_t *prev, *next;

	if (node == NULL || node->next == node)
		return;

	prev = node->prev;

	next = node->next;

	if (prev)
		prev->next = next;

	if (next)
		next->prev = prev;

	if (node->str)
		string_free (node->str);

	FREE (node);

	p->line_count--;
}

line_t *LINE_get_line_at (pad_t * p, int index)
{
	line_t *head = p->line_head;
	line_t *mover = head->next;

	while (mover != head && --index)
		mover = mover->next;

	if (mover == head)
		mover = NULL;

	return mover;
}

/* returns the visible length of a line in characters, */
/* expanding wide characters such as hard tab          */
int line_length (line_t *l)
{
	int  len = 0;
	char *c;

	if (l != NULL && l->str != NULL)
	{
		c = l->str->data;

		while (*c)
		{
			if (*c == '\t')
				len += 4; /* TODO: tab stops */
			else
				len++;

			c++;
		}
	}

	return len;
}

/* string data for the line is read only */
const char *line_data(line_t *l) {
	static const char *empty = "";
	
	return (l->str && l->str->data) ? l->str->data : empty;
}


