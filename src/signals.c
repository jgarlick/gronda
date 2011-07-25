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
 *  signal.c
 *  purpose : signal handling including error recovery
 *            and timer tick
 *  authors : James Garlick
 */
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/time.h>

#include "editor.h"

void sig_init ()
{
	struct itimerval value;
	struct sigaction s;

	s.sa_flags = 0;

	s.sa_handler = sig_handler;

	sigaction (SIGABRT, &s, NULL);
	sigaction (SIGFPE, &s, NULL);
	sigaction (SIGILL, &s, NULL);
	sigaction (SIGQUIT, &s, NULL);
	sigaction (SIGSEGV, &s, NULL);
	sigaction (SIGBUS, &s, NULL);
	sigaction (SIGTERM, &s, NULL);

	s.sa_flags = 0;

	s.sa_handler = handle_interrupt;
	sigaction (SIGINT, &s, NULL);

	s.sa_handler = sig_timer_click;
	sigaction (SIGALRM, &s, NULL);

	value.it_interval.tv_sec = 1;
	value.it_interval.tv_usec = 0;
	value.it_value.tv_sec = 1;
	value.it_value.tv_usec = 0;
}

void sig_handler (int signal)
{
	switch (signal)
	{
	case SIGABRT:
		sig_die ("Abort received. Shutting down.\n");
		break;
	case SIGFPE:
		debug ("Floating point exception (SIGFPE) received\n");
		handle_error (signal);
		break;
	case SIGILL:
		debug ("Illegal Instruction exception (SIGILL) received\n");
		handle_error (signal);
		break;
	case SIGQUIT:
		sig_cleanexit ("Quit character received. Shutting down.\n");
		break;
	case SIGSEGV:
		debug ("Segmentation violation (SIGSEV) received\n");
		handle_error (signal);
		break;
	case SIGBUS:
		debug ("Bus error (SIGBUS) received\n");
		handle_error (signal);
		break;
	case SIGTERM:
		sig_cleanexit
			("Software termination signal received. Shutting down.\n");
		break;
	}
}

void sig_cleanexit (char *str, ...)
{
	char   *out;

	va_list argp;

	display_close ();

	editor_fini ();

	va_start (argp, str);
	vasprintf (&out, str, argp);
	va_end (argp);

	printf ("%s", out);

	free (out);

	exit (0);
}

void sig_die (char *str, ...)
{
	char   *out;

	va_list argp;

	display_close ();

	va_start (argp, str);
	vasprintf (&out, str, argp);
	va_end (argp);

	printf ("%s", out);

	free (out);

	exit (1);
}

void sig_timer_click (int dummy)
{
	// does nothing at the moment
}
