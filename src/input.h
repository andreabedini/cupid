/* --------------------------------------------------------------------------
 * What the parser (parser.y) needs from the lexer (input.c).  Everything
 * else input.c exports is in connect.h.
 * ------------------------------------------------------------------------*/

#ifndef INPUT_H
#define INPUT_H

#include "prelude.h"
#include "storage.h"

extern int yylex(void);          /* next token; its value is in yylval */
extern int row;                  /* current input line, for errors     */
extern void closeAnyInput(void); /* end of a module: close the source  */

/* The layout algorithm: the grammar's begin/end rules open and close
 * layout contexts, and can ask whether the current token may close one */
extern int column, startColumn;
extern void goOffside(int);
extern void unOffside(void);
extern bool canUnOffside(void);

/* Identifiers the grammar compares against or builds with */
extern Cell conMain;
extern Cell varMinus;
extern Cell varPlus;
extern Cell varBang;
extern Cell varDot;
extern Cell varHiding;
extern Cell varQualified;
extern Cell varAsMod;
extern List imps;

#endif /* INPUT_H */
