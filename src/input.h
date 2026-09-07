/* --------------------------------------------------------------------------
 * What the parser (parser.y) needs from the lexer (input.c).  Everything
 * else input.c exports is in connect.h.
 * ------------------------------------------------------------------------*/

#ifndef INPUT_H
#define INPUT_H

extern Int  yylex         (Void);	/* next token; its value is in yylval */
extern Int  row;			/* current input line, for errors     */
extern Void closeAnyInput (Void);	/* end of a module: close the source  */

/* The layout algorithm: the grammar's begin/end rules open and close
 * layout contexts, and can ask whether the current token may close one */
extern Int  column, startColumn;
extern Void goOffside     (Int);
extern Void unOffside     (Void);
extern Bool canUnOffside  (Void);

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
