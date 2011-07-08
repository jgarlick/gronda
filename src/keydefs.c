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
 *  keydefs.c
 *  purpose : storage and retreival of key definitions
 *  authors : James Garlick
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "editor.h"

keydef_t *keydef_ptrs[27] = { NULL };

void
KEY_init ()
{
	char    k[10];
	char    es[80];
	int     a;

	/* default key bindings */

	for (a = 32; a < 127; a++)
	{
		sprintf (k, "%c", (char) a);

		if (a == 92)
			sprintf (es, "er 92");
		else
			sprintf (es, "es\'%c\'", (char) a);

		KEY_define (k, es);
	}

	KEY_define ("squote", "es\"\'\"");
	KEY_define ("dquote", "es\'\"\'");

	KEY_define ("esc", "tdm");
	KEY_define ("enter", "en");
	KEY_define ("tab", "er 9");

	KEY_define ("up", "au");
	KEY_define ("down", "ad");
	KEY_define ("left", "al");
	KEY_define ("right", "ar");

}

int 
get_keydef_index (char *a)
{
	while (*a && !isalpha(*a))
		a++;
	
	return isalpha (*a) ? tolower (*a) - 'a' : 26;
}

void
KEY_define (char *key_name, char *def)
{
	keydef_t *new;
	int     a;

	new = KEY_find (key_name);

	if (new)
	{
		debug ("Keydef overridden: %s\n", key_name);
		free(new->def);
		new->def = strdup(def);

		return;
	}

	new = ALLOC (keydef_t);
	new->name = strdup (key_name);
	new->def = strdup(def);

	a = get_keydef_index (key_name);

	new->next = keydef_ptrs[a];
	keydef_ptrs[a] = new;

	debug ("Keydef added: %s\n", key_name);
}

keydef_t   *
KEY_find (char *key_name)
{
	keydef_t *mover;
	int     a;

	a = get_keydef_index (key_name);

	mover = keydef_ptrs[a];

	while (mover)
	{
		if (!strcmp (key_name, mover->name))
			return mover;

		mover = mover->next;
	}

	return NULL;
}
