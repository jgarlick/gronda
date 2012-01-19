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
 *  cmd_system.c
 *  purpose : System commands
 *  authors : James Garlick
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#include "editor.h"

void cmd_bang (int argc, char *argv[])
{
	char  *args[10];
	char  buf[1024];
	char  *home;
	char  *shell;
	int   r;
	pid_t pid;
	pid_t status;
	char  *cmd;
	int   a;
	int   message = 0;
	int   copy    = 0;
	FILE  *f;

	cmd = NULL;
	for (a = 1; a < argc; a++)
	{
		if (argv[a][0] != '-')
			cmd = argv[a];
		else if (strstr (argv[a], "-m") == argv[a])
			message   = 1;
		else if (strstr (argv[a], "-c") == argv[a])
			copy = 1;
	}

	if (argc < 2)
	{
		output_message ("(!) Missing command argument");
		return;
	}

	args[1] = "-f";
	args[2] = "BangIn";

	if (copy)
	{
		args[0] = "xc";
		cmd_xp (3, args);
	}
	else
	{
		args[0] = "xd";
		cmd_xd (3, args);
	}

	home = getenv ("HOME");
	if (home == NULL) home = "";
	sprintf (buf
		, "%1$s <%2$s/.gronda/buffer/BangIn >%2$s/.gronda/buffer/BangOut 2>%2$s/.gronda/buffer/BangErr"
		, cmd, home);

	pid = fork ();
	if (pid == -1)
		return; /* TODO - report error */

	if (pid == 0)
	{
		shell = getenv ("SHELL");
		if (shell == NULL) shell = "/bin/sh";

		args[0] = shell;
		args[1] = "-c";
		args[2] = buf;
		args[3] = 0;

		execv (shell, args);

		exit (0);
	}
	
	r = waitpid (pid, &status, 0);

	if (r == -1)
		return; /* TODO - report error */

	if (!copy && !message)
	{
		args[0] = "xp";
		args[1] = "-f";
		args[2] = "BangOut";

		cmd_xp (3, args);
	}

	if (message)
	{
		sprintf (buf, "%s/.gronda/buffer/BangOut", home);

		f = fopen (buf, "r");
		if (f == NULL)
			return;

		args[0] = "msg";
		while (fgets (buf, 1024, f)) /* TODO - strip out newlines */
		{
			args[1] = buf;
			cmd_msg (2, args);
		}
		fclose (f);
	}
}

