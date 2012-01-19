#ifndef __STR_H
#define __STR_H

typedef struct string_s
{
	int     allocated;
	char   *data;
}
string_t;

string_t *string_alloc (char *, ...);

void    string_free (string_t * target);
void    string_append (string_t * target, char *format, ...);
void    string_insert (string_t * target, int pos, char *format, ...);
void    string_overwrite (string_t * target, int pos, char *format, ...);
void    string_remove (string_t * target, int start, int len);
void    string_truncate (string_t * target, int len);
string_t *string_dup (string_t * source);
int     string_length(string_t *);

void    string_debug (string_t *);

#endif
