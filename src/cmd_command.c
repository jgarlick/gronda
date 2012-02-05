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
 *  cmd_command.c
 *  purpose : Commands used for implementing other commands
 *  authors : James Garlick
 */

#include "editor.h"

void cmd_msg (int argc, char *argv[])
{
	if (argc > 1)
		output_message (argv[1]);
}

/* testing */
void cloop (int argc, char *argv[])
{
	while (1);
}
