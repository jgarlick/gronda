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
 *  str.c
 *  purpose : Functions for manipulating dynamic strings
 *  authors : James Garlick
 *            Graeme Jefferis
 */
#include <stdarg.h>
#include <string.h>

#include "editor.h"

#define BLOCKSIZE 10

#define CALLOC( x ) ( char * ) calloc( 1, x )

#define REALLOC( ptr, size ) ( char * ) realloc( ptr, size )

string_t *string_alloc (char *format, ...)
{
	int     len;
	int     blocks;
	string_t *new;
	char   *str;

	va_list argp;

	va_start (argp, format);
	vasprintf (&str, format, argp);
	va_end (argp);

	len = strlen (str);

	blocks = (len / BLOCKSIZE) + 1;

	new = ALLOC (string_t);

	if (!new)
		return NULL;

	new->data = CALLOC (blocks * BLOCKSIZE);
	if (!new->data)
	{
		FREE (new);
		return NULL;
	}

	strcpy (new->data, str);

	new->allocated = blocks * BLOCKSIZE;

	free (str);
	return new;
}

void string_free (string_t * target)
{
	if (target)
	{   
		if (target->data)
			FREE (target->data);

		FREE (target);
	}
}

void string_append (string_t * target, char *format, ...)
{
	char    *new;
	int     required;
	va_list argp;

	if (target == NULL)
		return;

	va_start (argp, format);
	vasprintf (&new, format, argp);
	va_end (argp);

	required = (strlen (target->data) + strlen (new)) - target->allocated;

	if (required >= 0)
	{
		// round to the nearest block
		required = (required / BLOCKSIZE) + 1;
		required = required * BLOCKSIZE;

		target->data = REALLOC (target->data, target->allocated + required);
		target->allocated += required;
	}

	strcat (target->data, new);

	free (new);
}

void string_nappend (string_t * target, char *format, ...)
{
	char    *new;
	int     required;
	va_list argp;

	if (target == NULL)
		return;

	va_start (argp, format);
	vasprintf (&new, format, argp);
	va_end (argp);

	required = (strlen (target->data) + strlen (new)) - target->allocated;

	if (required >= 0)
	{
		// round to the nearest block
		required = (required / BLOCKSIZE) + 1;
		required = required * BLOCKSIZE;

		target->data = REALLOC (target->data, target->allocated + required);
		target->allocated += required;
	}

	strcat (target->data, new);

	free (new);
}

void string_insert (string_t * target, int pos, char *format, ...)
{
	char   *new;
	int     required;
	int     a;
	int     padding;
	int     oldlen;
	int     newlen;
	va_list argp;

	if (target == NULL)
		return;

	va_start (argp, format);
	vasprintf (&new, format, argp);
	va_end (argp);

	oldlen = strlen (target->data);
	newlen = strlen (new);

	padding = 0;
	if (pos > oldlen)
		padding = pos - oldlen;

	required = (oldlen + newlen + padding) - target->allocated;

	if (required >= 0)
	{
		// round to the nearest block
		required = (required / BLOCKSIZE) + 1;
		required = required * BLOCKSIZE;

		target->data = REALLOC (target->data, target->allocated + required);
		target->allocated += required;
	}

	if (pos <= oldlen)
	{
		for (a = oldlen; a >= pos; a--)
			*(target->data + a + newlen) = *(target->data + a);
	}
	else
	{
		for (a = oldlen; a < pos; a++)
			*(target->data + a) = ' ';
		*(target->data + pos + newlen) = 0;
	}

	memcpy (target->data + pos, new, newlen);

	free (new);
}

void string_overwrite (string_t * target, int pos, char *format, ...)
{
	char    *new;
	int     required;
	int     i;
	int     oldlen;
	int     newlen;
	va_list argp;

	if (target == NULL)
		return;

	va_start (argp, format);
	vasprintf (&new, format, argp);
	va_end (argp);

	oldlen = strlen (target->data);
	newlen = strlen (new);

	required = pos + newlen - (target->allocated);

	if (required >= 0)
	{
		// round to the nearest block
		required = (required / BLOCKSIZE) + 1;
		required = required * BLOCKSIZE;

		target->data = REALLOC (target->data, target->allocated + required);
		target->allocated += required;
	}

	if (pos > oldlen)
	{
		for (i = oldlen; i < pos; i++)
			*(target->data + i) = ' ';
	}
	if (pos + newlen > oldlen)
		*(target->data + pos + newlen) = 0;

	memcpy (target->data + pos, new, newlen);

	free (new);
}

void string_remove (string_t * target, int start, int len)
{
	int     a;
	int     oldlen;

	if (target == NULL)
		return;

	oldlen = strlen (target->data);

	if (start > oldlen || len < 1) return;

	if (start + len > oldlen)
		string_truncate (target, start);
	else if (start < oldlen)
		for (a = start + len; a < oldlen + 1; a++)
			*(target->data + a - len) = *(target->data + a);
}

void string_truncate (string_t * target, int len)
{
	if (target == NULL)
		return;

	*(target->data + len) = 0;
}

string_t *string_dup (string_t * source)
{
	if (source == NULL)
		return NULL;

	return string_alloc ("%s", source->data);
}

int string_length(string_t *str) {
	return (str->data ? strlen(str->data) : 0);
}

void string_debug (string_t * s)
{
	if (!s)
		debug ("NULL STRING");
	else
		debug ("str \'%s\'\nlen %d\nallocated %d\n", s->data,
			   strlen (s->data), s->allocated);
}
