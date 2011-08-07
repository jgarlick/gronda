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
 *  buffer.c
 *  purpose : Paste buffer support functions
 *  authors : James Garlick
 */
#include <string.h>

#include "editor.h"

/* get the absolute start and end of the highlighted region */
void get_region (int shape, int *start_y, int *start_x, int *end_y, int *end_x)
{
	pad_t  *p = e->cpad;
	line_t *l;
	int acurs_y; /* absolute cursor position */
	int acurs_x; 

	acurs_y = p->curs_y + p->offset_y;
	acurs_x = p->curs_x + p->offset_x;

	/* default region if there is no mark */
	if (p->mark_y == 0 || p->mark_x == 0)
	{
		*start_y = acurs_y;
		*start_x = acurs_x;

		l = LINE_get_line_at (p, acurs_y);
		if (l == NULL)
			l = p->line_head;

		*end_y = *start_y;
//		*end_x = strlen (l->str->data) + 1;
		*end_x = get_curs_pos (strlen (l->str->data), l) + 1;
	}
	else if (shape == REGION_LINEAR)
	{
		if (acurs_y > p->mark_y || (acurs_y == p->mark_y && acurs_x > p->mark_x))
		{
			*start_y = p->mark_y;
			*start_x = p->mark_x;

			*end_y = acurs_y;
			*end_x = acurs_x;
		}
		else
		{
			*start_y = acurs_y;
			*start_x = acurs_x;

			*end_y = p->mark_y;
			*end_x = p->mark_x;
		}

		/* apply corrections for end of pad */
		if (*start_y > p->line_count)
		{
			*start_y = *start_x = *end_y = *end_x = 0;
		}
		else if (*end_y > p->line_count)
		{
			*end_y = p->line_count + 1;
			*end_x = 1;
		}

	}
	else
	{
		if (acurs_y < p->mark_y)
		{
			*start_y = acurs_y;
			*end_y   = p->mark_y;
		}
		else
		{
			*start_y = p->mark_y;
			*end_y   = acurs_y;
		}

		if (acurs_x < p->mark_x)
		{
			*start_x = acurs_x;
			*end_x   = p->mark_x;
		}
		else
		{
			*start_x = p->mark_x;
			*end_x   = acurs_x;
		}
	}
}

/* Search for a named paste buffer. */
/* If the buffer does not exist then try to create it. */
buffer_t *buffer_find (char *name)
{
	buffer_t *mover;

	mover = e->buffer_head;

	while (mover && strcmp (name, mover->name))
		mover = mover->next;

	if (!mover)
	{
		mover = ALLOC (buffer_t);
		if (mover)
		{
			strcpy (mover->name, name);
			mover->str     = string_alloc ("");
			mover->next    = e->buffer_head;
			e->buffer_head = mover;
		}
	}

	return mover;
}

buffer_t *buffer_create (char *name)
{
	buffer_t *buf;

	buf = buffer_find (name);

	if (!buf)
		debug ("buffer_add: Could not find buffer!");

	return buf;
}

void buffer_cutcopy (buffer_t *buf, int cut, int shape, int start_y, int start_x, int end_y, int end_x)
{
	static char temp[4096];
	int     a, b;
	int     diff;
	line_t *l;
	line_t *next;
	line_t *top = NULL;
	char   *s;
	pad_t  *p = e->cpad;
	int     len, copied_len;
	int     start;
	int     intab;

	l = LINE_get_line_at (p, start_y);
	if (l == NULL)
		l = p->line_head;

	for (a = start_y; a <= end_y && l != p->line_head; a++)
	{
		next = l->next;

		if (l->str == NULL)
		{
			s = "";
		}
		else
		{
			s = l->str->data;
		}

		if (shape & REGION_LINEAR)
		{
			len = strlen (s);
			if (a == start_y)
			{
				start = get_string_pos (start_x, s, &intab);

				s += (start_x - 1 > len) ? len : start_x - 1;
			}
			else
				start = 0;

			if (a == end_y)
			{
				start = (a == start_y) ? start_x : 1;

				strcpy (temp, s);
				temp[end_x - start] = '\0';

				s = temp;
			}
			
			/* if we are not on the last line, 
			   or the cursor is past the end of the line and it starts before the end of the line */
			if (a != end_y || (end_x > (len + 1) && !(a == start_y && start > (len + 1))))
			{
				string_append (buf->str, "%s\n", s);

				if (cut)
				{
					if (a != start_y)
						LINE_remove (p, l);
					else
						string_remove (l->str, start_x - 1, len); 						
				}
			}
			else
			{
				string_append (buf->str, "%s", s);

				if (cut)
				{
					string_remove (l->str, start - 1, end_x - start);

					if (start_y != end_y && top)
					{
						if (l->str)
							string_append (top->str, l->str->data);

						LINE_remove (p, l);
					}
				}
			}

			if (a == start_y) top = l;
		}
		else /* REGION_RECT */
		{
			len = end_x - start_x;
			if ((size_t)start_x <= strlen (s))
			{
				s += start_x - 1;
				strncpy (temp, s, len);
			}
			temp[len] = '\0';

			copied_len = strlen (temp);
			diff       = len - copied_len;

			for (b = 0; b < diff; b++)
				temp[copied_len + b] = ' ';

			if (a != end_y)
				string_append (buf->str, "%s\n", temp);
			else
				string_append (buf->str, "%s", temp);

			if (cut) string_remove (l->str, start_x - 1, len); 
		}

		l = next;
	}
}

void buffer_save (buffer_t *buf, char *file_name)
{
	char full_path[512];
	FILE *f;

	if (*file_name != '/')
		sprintf (full_path, "%s/.gronda/buffer/%s", 
		                    getenv ("HOME"), file_name);
	else
		strcpy (full_path, file_name);

	f = fopen (full_path, "w");

	if (f == NULL)
	{
		output_message ("Error saving buffer to file!");
		return;
	}

	fwrite (buf->str->data, 1, strlen (buf->str->data), f);
	fclose (f);
}

buffer_t *buffer_load (char *file_name)
{
	char *full_path;
	char line[512];
	FILE *f;
	buffer_t *buf;

	/* use line first to store the full path name */
	full_path = line;

	if (*file_name != '/')
		sprintf (full_path, "%s/.gronda/buffer/%s", 
		                    getenv ("HOME"), file_name);
	else
		strcpy (full_path, file_name);

	f = fopen (full_path, "r");
	if (f == NULL)
	{
		output_message ("Error loading buffer %s", file_name);
		return NULL;
	}

	buf = buffer_create (file_name);

	if (buf == NULL)
	{
		output_message ("Error creating buffer");
		return NULL;
	}

	if (*(buf->str->data))
		string_truncate (buf->str, 0);

	/* line is now used to store data from the file stream */ 
	while (fgets (line, 512, f))
		string_append (buf->str, line);

	fclose (f);

	return buf;
}

