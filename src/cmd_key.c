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
 *  cmd_key.c
 *  purpose : Key commands
 *  authors : James Garlick
 */
#include "editor.h"

#include <string.h>

void cmd_kd (int argc, char *argv[])
{
	if (strcmp (argv[argc - 1], "ke"))
	{
		output_message_c (argv[0], "Missing ke");

		return;
	}

	/* TODO - concatenate remaining args up to ke */

	KEY_define (argv[1], argv[2]);
}


