%{
	#include <stdarg.h>
	#include <stdio.h>
	#include <string.h>
	#include "editor.h"

	string_t *buf;
	char def_name[64];
	char *ptr;
	char *ptr2;

	char quote_delimiter;

	char *vargs[32];
	int i = 0;
	
%}

ws         [[:blank:]]+
optws		{ws}?
digit		[0-9]
number		[+-]?{digit}*"."?{digit}+
uint		{digit}+

punct		[\\\-[\]\.\,/<>?:|{}!@#$%^&*()_`~]
word		([[:alpha:]]|{punct})([[:alnum:]]|{punct})*

delimiter	[;]
quote		[\'\"]

range		^(\{{optws}{uint}?{optws}\,{optws}{uint}?{optws}\})

%x STRING
%x DEF

%%

kd{ws}{word}{ws} { 
	BEGIN DEF;
	
	/* get the def name */
	ptr = yytext + 2;
	while(*ptr == ' ') ptr++;
	ptr2 = def_name;
	while(*ptr != ' ') {
		*ptr2++ = *ptr++;
	}
	*ptr2 = '\0';
	
	buf = string_alloc("");
}
<DEF>{ws}ke {
	vargs[0] = "kd";
	vargs[1] = def_name;
	vargs[2] = buf->data;
	vargs[3] = "ke";
	execute_command(4, vargs);
	
	BEGIN 0;
	def_name[0] = '\0';
	string_free(buf);
}
<DEF>. {
	string_append(buf, "%c", *yytext);
}
<DEF><<EOF>> {
	BEGIN 0;
	string_free(buf);
}

{quote}		{
			BEGIN STRING;
			buf = string_alloc("");
			quote_delimiter = *yytext;
		}
<STRING>\\n	{
			string_append(buf, "\n");
		}
<STRING>\\t	{
			string_append(buf, "\t");
		}
<STRING>\\\'	{
			string_append(buf, "\'");
		}
<STRING>\\\"	{
			string_append(buf, "\"");
		}
<STRING>{quote}	{
			if (*yytext == quote_delimiter)
			{
				BEGIN 0; /* Back into normal parse mode. */
				debug(" [lex] Found string %s! Adding to vargs.", buf->data);
				if (i < 32)
					vargs[i++] = strdup(buf->data);
				else
					debug(" [lex] Out of tokens.");
				string_free(buf);
			}
			else
				string_append(buf, "%c", *yytext);
		}
<STRING><<EOF>>	{	debug(" [lex] Quotes don't match.");
			BEGIN 0;
			string_free(buf);
		}
<STRING>\n	{	debug(" [lex] Quotes don't match.");
			BEGIN 0;
			string_free(buf);
		}
<STRING>.	{
			string_append(buf, "%c", *yytext);
		}

{ws}		;

{word}	{	
			if (i < 32) vargs[i++] = strdup(yytext);
		}

{number} {	
			if (i < 32) vargs[i++] = strdup(yytext);
		}

<<EOF>>		{
			int j;
			if (i != 0)
			{
				execute_command(i, vargs);

				for (j = 0; j < i; j++)
					free(vargs[j]);
			}

			i = 0;
			yyterminate();
		}
{delimiter}	{
			int j;
			if (i != 0)
			{
				execute_command(i, vargs);

				for (j = 0; j < i; j++)
					free(vargs[j]);
			}
			i = 0;
		}
.		{
			debug (" [lex] *** Unexpected punctuation! Fix this! ***\n");
		}

%%

int yywrap(void)
{
	return 1;
}
