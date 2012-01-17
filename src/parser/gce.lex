%{
	#include <stdarg.h>
	#include <stdio.h>
	#include <string.h>
	#include <ctype.h>
	#include "editor.h"

	string_t *buf;
	char token_buf[64];
	char *ptr;
	char *ptr2;

	string_t *word;

	char quote_delimiter;

	char *vargs[32];
	int i = 0;

%}

ws         [[:blank:]]+
optws		{ws}?
digit		[0-9]
number		[+-]?{digit}*"."?{digit}+
uint		{digit}+

punct		[\\\-\.\,<>?/:|{}!@#$%^&*()_`~]
word		([[:alpha:]]|[\-\.\,<>:|@#%^*()_`~])([[:alnum:]]|{punct})*

delimiter	[;]
quote		[\'\"]

/*range		^(\{{optws}{uint}?{optws}\,{optws}{uint}?{optws}\})*/

%x STRING
%x DEF
%x SEARCH

%%
	word = string_alloc("");
	
kd{ws}[^[:blank:]]+{ws} { 
	BEGIN DEF;
	
	/* get the def name */
	ptr = yytext + 2;
	while(*ptr == ' ') ptr++;
	ptr2 = token_buf;
	while(*ptr != ' ') {
		*ptr2++ = *ptr++;
	}
	*ptr2 = '\0';
	
	buf = string_alloc("");
}
<DEF>{ws}ke {
	vargs[0] = "kd";
	vargs[1] = token_buf;
	vargs[2] = buf->data;
	vargs[3] = "ke";
	execute_command(4, vargs);
	
	BEGIN 0;
	token_buf[0] = '\0';
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

{ws}	{
	if (strlen(word->data) > 0 && i < 32) {
		vargs[i++] = strdup(word->data);
		string_truncate(word, 0);
	}
}

"["[+\-0-9]*[,][+\-0-9]*"]" {
	ptr = yytext + 1;
	ptr2 = token_buf;
	while(isdigit(*ptr) || *ptr == '+' || *ptr == '-')
		*ptr2++ = *ptr++;
	ptr++;
	*ptr2++ = '\0';
	while(isdigit(*ptr) || *ptr == '+' || *ptr == '-')
		*ptr2++ = *ptr++;
	*ptr2 = '\0';
	
	debug("found goto :%s:%s:", token_buf, token_buf + strlen(token_buf) + 1);
	vargs[0] = "[";
	vargs[1] = token_buf;
	vargs[2] = token_buf + strlen(token_buf) + 1;
	execute_command(3, vargs);
	
}

[/\\] {
	BEGIN SEARCH;

	buf = string_alloc("");
	quote_delimiter = *yytext;
	debug("SEARCH start %c", quote_delimiter);
}

<SEARCH>[\\][\\/] {
	debug("SEARCH escaped slash");
	string_append(buf, "%c", *yytext);
}

<SEARCH>[/\\] {
	if (*yytext == quote_delimiter)
	{
		debug("SEARCH delim");
		token_buf[0] = quote_delimiter;
		token_buf[1] = '\0';
		vargs[0] = token_buf;
		vargs[1] = buf->data;
		execute_command(2, vargs);

		BEGIN 0;
		string_free(buf);
	}
	else {
		debug("SEARCH escaped slash non delimiting");
		string_append(buf, "%c", *yytext);
	}

}
<SEARCH>. {
	debug("SEARCH char %c", *yytext);
	string_append(buf, "%c", *yytext);
}

<SEARCH><<EOF>> {
	debug("SEARCH eof");
	token_buf[0] = quote_delimiter;
	token_buf[1] = '\0';
	vargs[0] = token_buf;
	vargs[1] = buf->data;
	execute_command(2, vargs);

	BEGIN 0;
	string_free(buf);
}

[!$\\?&=] {
	token_buf[0] = *yytext;
	token_buf[1] = '\0';
	vargs[0] = strdup(token_buf);
	i = 1;
}

{word} {
       debug("found word %s", yytext);
		if (i < 32) vargs[i++] = strdup(yytext);
	}
{number} {
	debug("found num %s", yytext);
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
{delimiter}|\n	{
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
		debug (" [lex] *** Unexpected character! ***\n");
		}

%%
	
int yywrap(void)
{
	return 1;
}
