/*
 * dsp_dld/arm/dld_cmd_spec.l
 *
 * DSP Dynamic Loader Daemon: dld_cmd_spec.l
 *
 * Copyright (C) 2003-2005 Nokia Corporation
 *
 * Written by Kiyotaka Takahashi <kiyotaka.takahashi@nokia.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * 2005/01/26:  DSP Gateway version 3.3
 */

DZ			[0-9]
D			[1-9]
O			[0-7]
L			[a-zA-Z_]
H			[a-fA-F0-9]

%{
#include <stdio.h>
/*
#include "y.tab.h"
*/

#define ECHO // stop echo to sdtout

void count(void);
void comment(void);
%}

%%
"/*"			{ comment(); }

0[xX][a-fA-F0-9]+[hH]*	{
				struct expr_tree *p;
				count();
				p = expr_tree_new(ET_CONSTANT, strtoul(yytext, NULL, 16), NULL);
				yylval.expr = p;
				return(CONSTANT);
			}
0[a-fA-F0-9]+[hH]	{
				struct expr_tree *p;
				count();
				p = expr_tree_new(ET_CONSTANT, strtoul(yytext, NULL, 16), NULL);
				yylval.expr = p;
				return(CONSTANT);
			}
[0-9]+			{
				struct expr_tree *p;
				count();
				p = expr_tree_new(ET_CONSTANT, atoi(yytext), NULL);
				yylval.expr = p;
				return(CONSTANT);
			}

"MEMORY"		{ count(); return(MEMORY); }
"SECTIONS"		{ count(); return(SECTIONS); }
"align"			{ count(); return(ALIGN); }
"ALIGN"			{ count(); return(ALIGN); }
"attr"			{ count(); return(ATTR); }
"ATTR"			{ count(); return(ATTR); }
"block"			{ count(); return(BLOCK); }
"BLOCK"			{ count(); return(BLOCK); }
"COPY"			{ count(); return(COPY); }
"DSECT"			{ count(); return(DSECT); }
"f"			{ count(); return(FILL); }
"fill"			{ count(); return(FILL); }
"FILL"			{ count(); return(FILL); }
"group"			{ count(); return(GROUP); }
"GROUP"			{ count(); return(GROUP); }
"l"			{ count(); return(LENGTH); }
"len"			{ count(); return(LENGTH); }
"length"		{ count(); return(LENGTH); }
"LENGTH"		{ count(); return(LENGTH); }
"load"			{ count(); return(LOAD); }
"LOAD"			{ count(); return(LOAD); }
"NOLOAD"		{ count(); return(NOLOAD); }
"o"			{ count(); return(ORIGIN); }
"org"			{ count(); return(ORIGIN); }
"origin"		{ count(); return(ORIGIN); }
"ORIGIN"		{ count(); return(ORIGIN); }
"page"			{ count(); return(PAGE); }
"PAGE"			{ count(); return(PAGE); }
"range"			{ count(); return(RANGE); }
"run"			{ count(); return(RUN); }
"RUN"			{ count(); return(RUN); }
"spare"			{ count(); return(SPARE); }
"type"			{ count(); return(TYPE); }
"TYPE"			{ count(); return(TYPE); }
"UNION"			{ count(); return(UNION); }

"."			{ count(); return('.'); }
":"			{ count(); return(':'); }
"["			{ count(); return('['); }
"]"			{ count(); return(']'); }
">"			{ count(); return('>'); }
"|"			{ count(); return('|'); }

"("			{ count(); return '('; }
")"			{ count(); return ')'; }
","			{ count(); return ','; }
"{"			{ count(); return '{'; }
"}"			{ count(); return '}'; }
";"			{ count(); return ';'; }
"*"			{ count(); return '*'; }
"="			{ count(); return '='; }




"+="			{ count(); return(ADD_ASSIGN); }
"-="			{ count(); return(SUB_ASSIGN); }
"*="			{ count(); return(MUL_ASSIGN); }
"/="			{ count(); return(DIV_ASSIGN); }
"%="			{ count(); return(MOD_ASSIGN); }
"&="			{ count(); return(AND_ASSIGN); }
"^="			{ count(); return(XOR_ASSIGN); }
"|="			{ count(); return(OR_ASSIGN); }
">>"			{ count(); return(RIGHT_OP); }
"<<"			{ count(); return(LEFT_OP); }
"<="			{ count(); return(LE_OP); }
">="			{ count(); return(GE_OP); }
"=="			{ count(); return(EQ_OP); }
"!="			{ count(); return(NE_OP); }
"&"			{ count(); return('&'); }
"~"			{ count(); return('~'); }
"-"			{ count(); return('-'); }
"+"			{ count(); return('+'); }
"/"			{ count(); return('/'); }
"%"			{ count(); return('%'); }
"<"			{ count(); return('<'); }
"^"			{ count(); return('^'); }


[ \t\v\n\f]		{ count(); }

[a-zA-Z][a-zA-Z0-9_.$/\\]*	{
				struct expr_tree *p;
				count();
				p = expr_tree_new(ET_VAR, 0, yytext);
				yylval.expr = p;
				return ID;
				}
[_.$][a-zA-Z0-9_.$]+	{
				struct expr_tree *p;
				count();
				p = expr_tree_new(ET_VAR, 0, yytext);
				yylval.expr = p;
				return ID;
			}
[_.$][a-zA-Z0-9_.$:]+[a-zA-Z0-9_.$]	{
				struct expr_tree *p;
				count();
				p = expr_tree_new(ET_VAR, 0, yytext);
				yylval.expr = p;
				return ID;
			}

%%

int yywrap(void)
{
	return(1);
}

void comment(void)
{
	char c, c1;

loop:
	while ((c = input()) != '*' && c != 0)
		;//putchar(c);

	if ((c1 = input()) != '/' && c != 0)
	{
		unput(c1);
		goto loop;
	}

	if (c != 0)
		;//putchar(c1);
	return;
}

int column = 0;

void count(void)
{
	int i;

//printf("%s\n", yytext);

	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n')
			column = 0;
		else if (yytext[i] == '\t')
			column += 8 - (column % 8);
		else
			column++;

	ECHO;
}

int check_type()
{
	return(IDENTIFIER);
}
