#ifndef __COMMAND_H
#define __COMMAND_H

typedef struct command_s
{
	char   *name;
	void    (*func) ();

	struct command_s *next;
}
command_t;

extern command_t *command_ptrs[27];
void    execute_command (int, char **);
void    add_command (const char *, void (*)());
void    remove_command (char *);

#endif
