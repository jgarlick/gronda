#ifndef __KEYDEFS_H
#define __KEYDEFS_H

struct keydef_s
{
	char   *name;				/* the keyname */
	char   *def;				/* the commands to be executed */
	struct keydef_s *next;
};
typedef struct keydef_s keydef_t;

extern keydef_t *keydef_ptrs[27];

extern void KEY_init ();

extern void KEY_define (char *, char *);

extern keydef_t *KEY_find (char *);

#endif
