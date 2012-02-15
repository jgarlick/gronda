%{
	#include <stdarg.h>
	#include <stdio.h>
	#include <string.h>
	#include <ctype.h>
	#include "editor.h"

	#define MAX_ARGS (32)

	string_t *buf;
	char token_buf[64];
	char *ptr;
	char *ptr2;

	string_t *word;

	char quote_delimiter;

	char *vargs[MAX_ARGS];
	int i = 0;

	void add_arg(char *str) {
		if (i < MAX_ARGS)
			vargs[i++] = strdup(str);
		else
			debug("[lex] Out of tokens.");	
	}

	void execute_and_reset() {
		int j;
		if (i != 0) {
			execute_command(i, vargs);

			for (j = 0; j < i; j++)
				free(vargs[j]);
		}
		i = 0;	
	}

	void execute_search() {
		token_buf[0] = quote_delimiter;
		token_buf[1] = '\0';
		vargs[0] = token_buf;

		if(string_length(buf) > 0) {
			vargs[1] = buf->data;
			i = 2;
		} else {
			vargs[1] = NULL;
			i = 1;
		}
		execute_command(i, vargs);

		BEGIN 0;
		i = 0;
		string_free(buf);
	}

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
				add_arg(buf->data);
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
	if (strlen(word->data) > 0) {
		add_arg(word->data);
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
	if(i > 0) {
		yymore();
		REJECT;
	} else {
		BEGIN SEARCH;

		buf = string_alloc("");
		quote_delimiter = *yytext;
		debug("SEARCH start %c", quote_delimiter);
	}
}

<SEARCH>[\\][\\/] {
	string_append(buf, "%c", *yytext);
}

<SEARCH>[/\\] {
	if (*yytext == quote_delimiter)
		execute_search();
	else
		string_append(buf, "%c", *yytext);
}
<SEARCH>. {
	string_append(buf, "%c", *yytext);
}

<SEARCH><<EOF>> {
	execute_search();
}

[!$\\?&=] {
	token_buf[0] = *yytext;
	token_buf[1] = '\0';
	vargs[0] = strdup(token_buf);
	i = 1;
}

{word} {
		debug("found word %s", yytext);
		add_arg(yytext);
	}
{number} {
		debug("found num %s", yytext);
		add_arg(yytext);
	}


<<EOF>>		{
			execute_and_reset();
			yyterminate();
		}
{delimiter}|\n	{
			execute_and_reset();
		}
.		{
		debug (" [lex] *** Unexpected character! ***");
		}

%%
	
int yywrap(void)
{
	return 1;
}
