/* --------------------------------------------------------------------------
 * Unparse expressions and types - for use in error messages, type checker
 * and for debugging.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: output.c,v $
 * $Revision: 1.40 $
 * $Date: 2005/11/02 15:57:56 $
 * ------------------------------------------------------------------------*/

#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "errors.h"
#include "output.h"
#include "char.h"
#include <ctype.h>

#define DEPTH_LIMIT     15

/* --------------------------------------------------------------------------
 * Local function prototypes:
 * ------------------------------------------------------------------------*/

static Void local putChr         (Int);
static Void local putStr         (String);
static Void local putInt         (Int);

static Void local put            (Int,Cell);
static Void local putFlds        (Cell,List);
static Void local putComp        (Cell,List);
static Void local putQual        (Cell);
static Bool local isDictVal      (Cell);
static Cell local maySkipDict    (Cell);
static Void local putAp          (Int,Cell);
static Void local putOverInfix   (Int,Text,Syntax,Cell);
static Void local putInfix       (Int,Text,Syntax,Cell,Cell);
static Void local putSimpleAp    (Cell,Int);
static Void local putTuple       (Int,Cell);
static Int  local unusedTups     (Int,Cell);
static Void local unlexVar       (Text);
static Void local unlexFullVar   (Name);
static Void local unlexOp        (Text);
static Void local unlexCharConst (Char);
static Void local unlexStrConst  (Text);

static Void local putSigType     (Cell);
static Void local putContext     (List,List,Int);
static Void local putPred        (Cell,Int);
static Void local putType        (Cell,Int,Int);
static Void local putModule      (Module);
static Void local putTyVar       (Int);
static Bool local putTupleType   (Cell,Int);
static Void local putApType      (Type,Int,Int);

static Void local putKind        (Kind);
static Void local putKinds	 (Kinds);


static Bool local isStrConst	 (Cell);
static Void local putStrConst	 (Cell);
static Bool local isCharCell	 (Cell);
static Char local getCellChar    (Cell);

/* --------------------------------------------------------------------------
 * Basic output routines:
 * ------------------------------------------------------------------------*/

static FILE *outputStream;             /* current output stream        	   */
#if DEBUG_SHOWSC
static Int  outColumn = 0;             /* current output column number 	   */
#endif				       				       
								       
#define OPEN(b)    if (b) putChr('('); 				       
#define CLOSE(b)   if (b) putChr(')'); 				       
								       
static Void local putChr(Int c)            /* print single character       	   */
{
    Putc(c,outputStream);	       				       
#if DEBUG_SHOWSC
    outColumn++;		       				       
#endif				       				       
}				       				       
								       
static Void local putStr(String s)            /* print string                 	   */
{
    for (; *s; s++) {		       				       
	Putc(*s,outputStream);	       				       
#if DEBUG_SHOWSC
	outColumn++;		       				       
#endif				       				       
    }				       				       
}				       				       
								       
static Void local putInt(Int n)            /* print integer                	   */
{
    static char intBuf[16];
    sprintf(intBuf,"%d",n);
    putStr(intBuf);
}

/* --------------------------------------------------------------------------
 * Precedence values (See Haskell 1.3 report, p.12):
 * ------------------------------------------------------------------------*/

#define ALWAYS      FUN_PREC           /* Always use parens (unless atomic)*/
				       /* User defined operators have prec */
				       /* in the range MIN_PREC..MAX_PREC  */
#define ARROW_PREC  MAX_PREC           /* for printing -> in type exprs    */
#define COCO_PREC   (MIN_PREC-1)       /* :: is left assoc, low precedence */
#define COND_PREC   (MIN_PREC-2)       /* conditional expressions          */
#define WHERE_PREC  (MIN_PREC-3)       /* where expressions                */
#define LAM_PREC    (MIN_PREC-4)       /* lambda abstraction               */
#define NEVER       LAM_PREC           /* Never use parentheses            */


/* --------------------------------------------------------------------------
 * Print an expression (used to display context of type errors):
 * ------------------------------------------------------------------------*/

static Int putDepth = 0;               /* limits depth of printing DBG     */
static Bool inString= 0;	       /* is char string being put?        */

static Void local put(Int d, Cell e)             /* print expression e in context of */
                                       /* operator of precedence d         */
{
    List xs;

    if (putDepth>DEPTH_LIMIT) {
	putStr("...");
	return;
    }
    else
	putDepth++;


    switch (whatIs(e)) {
	case FINLIST    : putChr('[');
			  xs = snd(e);
			  if (nonNull(xs)) {
			      put(NEVER,hd(xs));
			      while (nonNull(xs=tl(xs))) {
				  putChr(',');
				  put(NEVER,hd(xs));
			      }
			  }
			  putChr(']');
			  break;

        case AP         : putAp(d,e);
			  break;

	case NAME       : 
	                  putModule(name(e).mod);
	                  unlexVar(name(e).text);
			  break;

	case VARIDCELL  :
	case VAROPCELL  :
	case DICTVAR    :
	case CONIDCELL  :
	case CONOPCELL  : 
	                  unlexVar(textOf(e));
			  break;


	case FREECELL   : putStr("{free!}");
			  break;

	case TUPLE      : putTuple(tupleOf(e),e);
			  break;

	case WILDCARD   : putChr('_');
			  break;

	case ASPAT      : put(NEVER,fst(snd(e)));
			  putChr('@');
			  put(ALWAYS,snd(snd(e)));
			  break;

	case LAZYPAT    : putChr('~');
			  put(ALWAYS,snd(e));
			  break;


	case DOCOMP     : putStr("do {...}");
			  break;

	case COMP       : putComp(fst(snd(e)),snd(snd(e)));
			  break;

	case MONADCOMP  : putComp(fst(snd(snd(e))),snd(snd(snd(e))));
			  break;

	case CHARCELL   : unlexCharConst(charOf(e));
			  break;

	case INTCELL    : {   Int i = intOf(e);
			      if (i<0 && d>=UMINUS_PREC) putChr('(');
			      putInt(i);
			      if (i<0 && d>=UMINUS_PREC) putChr(')');
			  }
			  break;

#if BIGNUMS
	case NEGNUM     :
	case ZERONUM    :
	case POSNUM     : xs = bigOut(e,NIL,d>=UMINUS_PREC);
			  for (; nonNull(xs); xs=tl(xs))
			      putChr(charOf(arg(hd(xs))));
			  break;
#endif

	case DOUBLECELL : {   Double f = (Double)doubleOf(e);
			      if (f<0 && d>=UMINUS_PREC) putChr('(');
			      putStr(doubleToString(f));
			      if (f<0 && d>=UMINUS_PREC) putChr(')');
			  }
			  break;

	case STRCELL    : unlexStrConst(textOf(e));
			  break;

	case LETREC     : OPEN(d>WHERE_PREC);
#if DEBUG_CODE
			  putStr("let {");
			  put(NEVER,fst(snd(e)));
			  putStr("} in ");
#else
			  putStr("let {...} in ");
#endif
			  put(WHERE_PREC+1,snd(snd(e)));
			  CLOSE(d>WHERE_PREC);
			  break;

	case COND       : OPEN(d>COND_PREC);
			  putStr("if ");
			  put(COND_PREC+1,fst3(snd(e)));
			  putStr(" then ");
			  put(COND_PREC+1,snd3(snd(e)));
			  putStr(" else ");
			  put(COND_PREC+1,thd3(snd(e)));
			  CLOSE(d>COND_PREC);
			  break;

	case LAMBDA     : xs = fst(snd(e));
			  if (whatIs(xs)==BIGLAM)
			      xs = snd(snd(xs));
			  while (nonNull(xs) && isDictVal(hd(xs)))
			      xs = tl(xs);
			  if (isNull(xs)) {
			      put(d,snd(snd(snd(e))));
			      break;
			  }
			  OPEN(d>LAM_PREC);
			  putChr('\\');
			  if (nonNull(xs)) {
			      put(ALWAYS,hd(xs));
			      while (nonNull(xs=tl(xs))) {
				  putChr(' ');
				  put(ALWAYS,hd(xs));
			      }
			  }
			  putStr(" -> ");
			  put(LAM_PREC,snd(snd(snd(e))));
			  CLOSE(d>LAM_PREC);
			  break;

	case ESIGN      : OPEN(d>COCO_PREC);
			  put(COCO_PREC,fst(snd(e)));
			  putStr(" :: ");
			  putSigType(snd(snd(e)));
			  CLOSE(d>COCO_PREC);
			  break;

	case BIGLAM	: put(d,snd(snd(e)));
			  break;

	case CASE       : putStr("case ");
			  put(NEVER,fst(snd(e)));
#if DEBUG_CODE
			  putStr(" of {");
			  put(NEVER,snd(snd(e)));
			  putChr('}');
#else
			  putStr(" of {...}");
#endif
			  break;

	case CONFLDS    : putFlds(fst(snd(e)),snd(snd(e)));
			  break;

	case UPDFLDS    : putFlds(fst3(snd(e)),thd3(snd(e)));
			  break;

	case INDIRECT   : putChr('^');
			  put(ALWAYS,snd(e));
			  break;
	default         : /*internal("put");*/
			  putChr('$');
			  putInt(e);
			  break;
    }
    putDepth--;
}



static Void local putFlds(Cell exp, List fs)       /* Output exp using labelled fields*/
{
    put(ALWAYS,exp);
    putStr(" {");
    for (; nonNull(fs); fs=tl(fs)) {
	Cell v = hd(fs);
	if (isVar(v))
	    put(NEVER,v);
	else {
	    Cell f = fst(v);
	    Cell e = snd(v);
	    Text t = isName(f) ? name(f).text :
		     isVar(f)  ? textOf(f)    : inventText();
	    Text s = isName(e) ? name(e).text :
		     isVar(e)  ? textOf(e)    : inventText();

	    put(NEVER,f);
	    if (haskell98 || s!=t) {
		putStr(" = ");
		put(NEVER,e);
	    }
	}
	if (nonNull(tl(fs)))
	    putStr(", ");
    }
    putChr('}');
}

static Void local putComp(Cell e, List qs)         /* print comprehension             */
{
    putStr("[ ");
    put(NEVER,e);
    if (nonNull(qs)) {
	putStr(" | ");
	putQual(hd(qs));
	while (nonNull(qs=tl(qs))) {
	    putStr(", ");
	    putQual(hd(qs));
	}
    }
    putStr(" ]");
}

static Void local putQual(Cell q)            /* print list comp qualifier       */
{
    switch (whatIs(q)) {
	case BOOLQUAL : put(NEVER,snd(q));
			return;

	case QWHERE   : putStr("let {...}");
			return;

	case FROMQUAL : put(ALWAYS,fst(snd(q)));
			putStr("<-");
			put(NEVER,snd(snd(q)));
			return;
    }
}

static Bool local isDictVal(Cell e)          /* Look for dictionary value       */
{
#if !DEBUG_CODE
    Cell h = getHead(e);
    switch (whatIs(h)) {
	case DICTVAR : return TRUE;
	case NAME    : return isDfun(h);
    }
#endif
    return FALSE;
}

static Cell local maySkipDict(Cell e)        /* descend function application,   */
{                               /* ignoring dict aps               */
    while (isAp(e) && isDictVal(arg(e)))
	e = fun(e);
    return e;
}

static Void local putAp(Int d, Cell e)            /* print application (args>=1)     */
{
    Cell   h    = NIL;
    Text   t    = NIL;
    Syntax sy   = NIL;
    Int    args = 0;

    for (h=e; isAp(h); h=fun(h))        /* find head of expression, looking*/
	if (!isDictVal(arg(h)))         /* for dictionary arguments        */
	    args++;

    if (args==0) {                      /* Special case when *all* args    */
	put(d,h);                       /* are dictionary values           */
	return;
    }

    switch (whatIs(h)) {

	case TUPLE      : OPEN(args>tupleOf(h) && d>=FUN_PREC);
			  putTuple(tupleOf(h),e);
			  CLOSE(args>tupleOf(h) && d>=FUN_PREC);
			  return;

	case NAME       : if (args==1 &&
			      ((h==nameFromInt     && isInt(arg(e)))    ||
			       (h==nameFromInteger && isBignum(arg(e))) ||
			       (h==nameFromDouble  && isDouble(arg(e))))) {
			      put(d,arg(e));
			      return;
			  }
			  t  = name(h).text;
			  sy = syntaxOf(h);
			  break;

	case VARIDCELL  :
	case VAROPCELL  :
	case DICTVAR    :
	case CONIDCELL  :
	case CONOPCELL  : sy = defaultSyntax(t = textOf(h));
			  break;


	default         : sy = APPLIC;
			  break;
    }

    e = maySkipDict(e);

    if (sy==APPLIC) {                   /* print simple application        */
	OPEN(d>=FUN_PREC);
	putSimpleAp(e,args);
	CLOSE(d>=FUN_PREC);
	return;
    }
    else if (args==1) {                 /* print section of the form (e+)  */
	putChr('(');
	put(FUN_PREC-1,arg(e));
	putChr(' ');
	unlexOp(t);
	putChr(')');
    }
    else if (args==2 && isStrConst(e))
        putStrConst(e);
    else if (args==2)                  /* infix expr of the form e1 + e2   */
	putInfix(d,t,sy,arg(maySkipDict(fun(e))),arg(e));
    else {                             /* o/w (e1 + e2) e3 ... en   (n>=3) */
	OPEN(d>=FUN_PREC);
	putOverInfix(args,t,sy,e);
	CLOSE(d>=FUN_PREC);
    }
}

static Void local putStrConst(Cell e)
{
    putChr('"'); 
    while (isAp(e) && isAp(fun(e)) && fun(fun(e))==nameCons) {
        putStr(unlexChar(getCellChar(arg(fun(e))),'"'));
        e = arg(e);
    }
    if (e!=nameNil)
        internal("putStrConst");
    putChr('"');
}

static Bool local isStrConst(Cell e)
{
    while (isAp(e) && isAp(fun(e)) && fun(fun(e))==nameCons) {
        if (!isCharCell(arg(fun(e))))
            return FALSE;
        e = arg(e);
    }
    return e==nameNil;
}

static Bool local isCharCell(Cell e)
{
    while(1)
        switch(whatIs(e)) {
	    case CHARCELL	: return 1;
	    case INDIRECT	: e = snd(e);
	    			  break;
	    default		: return 0;
	}
}

static Char local getCellChar(Cell e)
{
    while(1)
        switch(whatIs(e)) {
	    case CHARCELL	: return charOf(e);
	    case INDIRECT	: e = snd(e);
	    			  break;
	    default		: internal("error in getCellChar");
	}
}

static Void local putOverInfix(Int args, Text t, Syntax sy, Cell e)
                                       /* infix applied to >= 3 arguments  */
{
    if (args>2) {
	putOverInfix(args-1,t,sy,maySkipDict(fun(e)));
	putChr(' ');
	put(FUN_PREC,arg(e));
    }
    else
	putInfix(ALWAYS,t,sy,arg(maySkipDict(fun(e))),arg(e));
}

static Void local putInfix(Int d, Text t, Syntax sy, Cell e, Cell f)  /* print infix expression          */
                                        /* Infix operator symbol           */
                                        /* with name t, syntax s           */
{                          /* Left and right operands         */
    Syntax a = assocOf(sy);
    Int    p = precOf(sy);
    OPEN(d>p);
    put((a==LEFT_ASS ? p : 1+p), e);
    putChr(' ');
    unlexOp(t);
    putChr(' ');
    put((a==RIGHT_ASS ? p : 1+p), f);
    CLOSE(d>p);
}

static Void local putSimpleAp(Cell e, Int n)      /* print application e0 e1 ... en  */
{
    if (n>0) {
	putSimpleAp(maySkipDict(fun(e)),n-1);
	putChr(' ');
	put(FUN_PREC,arg(e));
    }
    else
	put(FUN_PREC,e);
}

static Void local putTuple(Int ts, Cell e)        /* Print tuple expression, allowing*/
                                        /* for possibility of either too   */
{                               /* few or too many args to constr  */
    Int i;
    putChr('(');
    if ((i=unusedTups(ts,e))>0) {
	while (--i>0)
	    putChr(',');
	putChr(')');
    }
}

static Int local unusedTups(Int ts, Cell e)       /* print first part of tuple expr  */
                                        /* returning number of constructor */
{                               /* args not yet printed ...        */
    if (isAp(e)) {
	if ((ts=unusedTups(ts,fun(e))-1)>=0) {
	    put(NEVER,arg(e));
	    putChr(ts>0?',':')');
	}
	else {
	    putChr(' ');
	    put(FUN_PREC,arg(e));
	}
    }
    return ts;
}

static Void local unlexVar(Text t)          /* print text as a variable name    */
{                              /* operator symbols must be enclosed*/
    String s = textToStr(t);           /* in parentheses... except [] ...  */
    if ((isascii(s[0]) && isalpha(s[0])) || s[0]=='_' || s[0]=='[' || s[0]=='(')
	putStr(s);
    else {
	putChr('(');
	putStr(s);
	putChr(')');
    }
}

static Void local unlexFullVar(Name n)  /* print text as a variable name    */
{
  Module m = name(n).mod;
  Text t = name(n).text;
  if (name(n).parent 
      && isName(name(n).parent)
      && isPair(name(name(n).parent).defn)
      && snd(name(name(n).parent).defn) == n) {
    /* Constructor with strict fields are handled in
     * a strange manner. Here we print the true construtor name
     */
    unlexFullVar(name(n).parent);  
  } else {
    if (name(n).primDef) {
      putStr("Prelude");
    } else {
      putStr(textToStr(module(m).text));
    }
    putChr('.');
    putStr(textToStr(t));
  }
}


static Void local unlexOp(Text t)           /* print text as operator name      */
{                              /* alpha numeric symbols must be    */
    String s = textToStr(t);           /* enclosed by backquotes           */

    if (isascii(s[0]) && isalpha(s[0])) {
	putChr('`');
	putStr(s);
	putChr('`');
    }
    else
	putStr(s);
}

static Void local unlexCharConst(Char c)
{
    putChr('\'');
    putStr(unlexChar(c,'\''));
    putChr('\'');
}

static Void local unlexStrConst(Text t)
{
    String s            = textToStr(t);
    static Char SO      = 14;          /* ASCII code for '\SO'             */
    Bool   lastWasSO    = FALSE;
    Bool   lastWasDigit = FALSE;
    Bool   lastWasEsc   = FALSE;

    putChr('\"');
    while (*s) {
      String ch;
	Char   c  = ' ';
	Char   sc = getStrChr(&s);

	ch = unlexChar(sc,'\"');

	if ((lastWasSO && *ch=='H') ||
		(lastWasEsc && lastWasDigit && isascii(*ch) && isdigit(*ch)))
	    putStr("\\&");

	lastWasEsc   = (*ch=='\\');
	lastWasSO    = (sc==SO);
	for (; *ch; c = *ch++)
	    putChr(*ch);
	lastWasDigit = (isascii(c) && isdigit(c));
    }
    putChr('\"');
}

/* --------------------------------------------------------------------------
 * Pretty printer for supercombinator definitions:
 * (i.e., for lambda-lifter output, immediately prior to code generation.)
 * ------------------------------------------------------------------------*/

#if DEBUG_SHOWSC
static Void local pIndent       (Int);
static Void local pPut          (Int,Cell,Int);
static Void local pPutAp        (Int,Cell,Int);
static Void local pPutSimpleAp  (Cell,Int);
static Void local pPutTuple     (Int,Cell,Int);
static Int  local punusedTups   (Int,Cell,Int);
static Void local pPutOffset    (Int);
static Int  local pPutLocals    (List,Int);
static Void local pLiftedStart  (Cell,Int,String);
static Void local pLifted       (Cell,Int,String);
static Int  local pDiscr        (Cell,Int);

static Void local pIndent(Int n)           /* indent to particular position    */
{
    outColumn = n;
    while (0<n--) {
	Putc(' ',outputStream);
    }
}

static Void local pPut(Int d, Cell e, Int co)         /* pretty print expr in context of  */
                                       /* operator of precedence d         */
                                       /* with current offset co           */
{
    switch (whatIs(e)) {
	case AP         : pPutAp(d,e,co);
			  break;

	case OFFSET     : pPutOffset(offsetOf(e));
			  break;

        case NAME       : 
                          unlexFullVar(e);
			  break;


	case TUPLE      : pPutTuple(tupleOf(e),e,co);
			  break;

	case CHARCELL   : unlexCharConst(charOf(e));
			  break;

	case INTCELL    : {   Int i = intOf(e);
			      if (i<0 && d>=UMINUS_PREC) putChr('(');
			      putInt(i);
			      if (i<0 && d>=UMINUS_PREC) putChr(')');
			  }
			  break;

	case DOUBLECELL : {   Double f = (Double)doubleOf(e);
			      if (f<0 && d>=UMINUS_PREC) putChr('(');
			      putStr(doubleToString(f));
			      if (f<0 && d>=UMINUS_PREC) putChr(')');
			  }
			  break;

	case STRCELL    : unlexStrConst(textOf(e));
			  break;
#if BIGNUMS
	case NEGNUM     :
	case ZERONUM    :
	case POSNUM     : {   List xs = bigOut(e,NIL,d>=UMINUS_PREC);
			      for (; nonNull(xs); xs=tl(xs))
				  putChr(charOf(arg(hd(xs))));
			  }
			  break;
#endif
	case LETREC     : OPEN(d>WHERE_PREC);
			  co += pPutLocals(fst(snd(e)),co);
			  pPut(WHERE_PREC+1, snd(snd(e)), co);
			  CLOSE(d>WHERE_PREC);
			  break;

        case COND       : { Int  left = outColumn;
	                    OPEN(d>COND_PREC);
			    putStr("case ");
			    pPut(COND_PREC+1,fst3(snd(e)),co);
			    putStr(" of");
			    pIndent(left+2);
			    putStr(" { Prelude.True -> ");
			    pPut(COND_PREC+1,snd3(snd(e)),co);
			    pIndent(left+2);
			    putStr(" ; Prelude.False ->  ");
			    pPut(COND_PREC+1,thd3(snd(e)),co);
			    pIndent(left+2);
			    putStr("}");
			    CLOSE(d>COND_PREC);
			    break;
	                   }
	default         : printf("[e = %d, whatIs(e) = %d]\n",e,whatIs(e));
	                  internal("pPut");
    }
}

static Void local pPutAp(Int d, Cell e, Int co)        /* print application (args>=1)     */
{
    Cell h = getHead(e);
    if (isTuple(h)) {
	Int args = argCount;
	OPEN(args>tupleOf(h) && d>=FUN_PREC);
	pPutTuple(tupleOf(h),e,co);
	CLOSE(args>tupleOf(h) && d>=FUN_PREC);
	return;
    }
    OPEN(d>=FUN_PREC);
    pPutSimpleAp(e,co);
    CLOSE(d>=FUN_PREC);
}

static Void local pPutSimpleAp(Cell e, Int co)    /* print application e0 e1 ... en  */
{
    if (isAp(e)) {
	pPutSimpleAp(fun(e),co);
	putChr(' ');
	pPut(FUN_PREC,arg(e),co);
    }
    else
	pPut(FUN_PREC,e,co);
}

static Void local pPutTuple(Int ts, Cell e, Int co)    /* Print tuple expression, allowing*/
                                        /* for possibility of either too   */
                                        /* few or too many args to constr  */
{
    Int i;
    putStr("(#");
    putInt(ts);
    putChr(' ');
    if ((i=punusedTups(ts,e,co))>0) {
	while (--i>0)
	    putChr(' ');
    }
    putChr(')');
}

static Int local punusedTups(Int ts, Cell e, Int co)   /* print first part of tuple expr  */
                                        /* returning number of constructor */
                                        /* args not yet printed ...        */
{
    if (isAp(e)) {
	if ((ts=punusedTups(ts,fun(e),co)-1)>=0) {
	    pPut(ALWAYS,arg(e),co);
	    putStr(ts>0?" ":"");
	}
	else {
	    putChr(' ');
	    pPut(FUN_PREC,arg(e),co);
	}
    }
    return ts;
}

static Void local pPutOffset(Int n)         /* pretty print offset number      */
{
    putChr('_');
    putInt(n);
}

static Int local pPutLocals(List vs, Int co)      /* pretty print locals             */
{
    Int left = outColumn;
    Int n    = length(vs);
    Int i;

    putStr("let ");
    for (i=0; i<n; i++) {
	pPutOffset(co+i+1);
	putChr(' ');
	pLiftedStart(hd(vs),co+n,"=");
	vs = tl(vs);
	if (nonNull(vs))
	    pIndent(left+4);
    }
    pIndent(left);
    putStr("in ");
    return n;
}

static Void local pLiftedStart(Cell e, Int co, String eq) /* print start of definition       */
{
    putStr(eq);
    putChr(' ');
    pLifted(e,co,eq);
}

static Void local pLifted(Cell e, Int co, String eq)      /* print lifted definition         */
{
    switch (whatIs(e)) {
	case GUARDED : {   Int  left = outColumn;
	                   Int count = 0;
                           Int tmp;
			   List gs   = snd(e);
			   if (isNull(gs))
			       internal("pLifted");
			   for (;;) {
			       count++;
			       putStr("case ");
			       pPut(NEVER,fst(hd(gs)),co);
			       putStr(" of\n");
			       pIndent(left + 2);
			       putStr("{ Prelude.True -> ");
			       pPut(NEVER,snd(hd(gs)),co);
			       putStr("\n");
			       pIndent(left + 2);
			       putStr("; _ -> \n");
			       gs = tl(gs);
			       pIndent(left);
			       if (!nonNull(gs))
				   break;
			   }
			   putStr("_fatbar ");
			   for(tmp = 0;tmp < count;tmp++) {
			     putChr('}');
			   }
			   putStr(";\n");
		       }
		       break;

	case LETREC  : co += pPutLocals(fst(snd(e)),co);
		       pLifted(snd(snd(e)), co, eq);
		       break;

	case FATBAR  : {   Int left = outColumn;
                           putStr("let_ _fatbar = \n");
			   pIndent(left+2);
			   pLifted(snd(snd(e)),co,eq);
			   pIndent(left);
                           putStr("in\n");
			   pIndent(left+2);
			   pLifted(fst(snd(e)),co,eq);
		       }
		       break;

	case CASE    : {   Int  left = outColumn;
			   List cs   = snd(snd(e));
			   putStr("case ");
			   pPut(NEVER,fst(snd(e)),co);
			   putStr(" of {\n");
			   for (; nonNull(cs); cs=tl(cs)) {
			       Int arity;
			       pIndent(left+2);
			       arity = pDiscr(fst(hd(cs)),co);
			       putChr(' ');
			       pLiftedStart(snd(hd(cs)),co+arity,"->");
			   }
			   pIndent(left+2);
			   putStr("_ -> _fatbar\n");
			   pIndent(left);
			   putStr("};\n");
		       }
		       break;

	case NUMCASE : {   Int  left = outColumn;
			   Cell t    = snd(e);
			   Cell h     = getHead(snd3(t));
			   String eqInt     = "Prelude.primPmInt";
			   String eqInteger = "Prelude.primPmInteger";
			   String eqDouble  = "Prelude.primPmFlt";
			   String theEq     = "** BAD EQUALITY **";
			   Int  ar = 0;
			   putStr("case ");
			   switch (whatIs(h)) {
			   case NAME: 
			     if (h==nameFromInt) {
			       theEq = eqInt;
			     } else if (h == nameFromInteger) {
			       theEq = eqInteger;
			     } else if (h == nameFromDouble) {
			       theEq = eqDouble;
			     } else {
			       ERRMSG(0) "error in NUMCASE" EEND;
			     }
			     break;
			   case ADDPAT:
			       ERRMSG(0) "error in NUMCASE " EEND;
			     break;
			   default:
			       ERRMSG(0) "error in NUMCASE" EEND;
			     break;			     
			   }
			   putStr(theEq);
			   putStr(" (");
			   pPut(NEVER,arg(fun(snd3(t))),co);
			   putStr(") (");
			   pPut(NEVER,arg(snd3(t)),co);
			   putStr(") (");
			   pPut(NEVER,fst3(t),co);
			   putStr(") of\n");
			   pIndent(left+2);
			   putStr("{ Prelude.True ");
			   pLiftedStart(thd3(t),co+ar,"->");
			   putStr("\n");
			   pIndent(left+2);
			   putStr("  _ -> _fatbar\n");
			   pIndent(left+2);
			   putStr("};\n");
			   pIndent(left);
		       }
		       break;

	default      : pPut(NEVER,e,co);
		       putStr(";\n");
		       break;
    }
}

static Int local pDiscr(Cell d, Int co)           /* pretty print discriminator      */
{
    Int arity = 0;

    switch (whatIs(d)) {

	case NAME     : {   Int i = 0;
			    arity = name(d).arity;
			    unlexFullVar(d);
			    for (; i<arity; ++i) {
				putChr(' ');
				pPutOffset(co+arity-i);
			    }
			}
			break;

	case TUPLE    : {   Int i = 0;
			    arity = tupleOf(d);
			    putChr('#');
			    putInt(arity);
			    putChr(' ');
			    pPutOffset(co+arity);
			    while (++i<arity) {
				putChr(' ');
				pPutOffset(co+arity-i);
			    }
			}
			break;

	default       : pPut(NEVER,d,co);
			break;
    }
    return arity;
}

Void printSc(FILE*fp, Text t, Int arity, Cell e)              /* Pretty print sc defn on fp      */
{
    Int i;
    outputStream = fp;

    putChr('\n');
    outColumn = 0;
    unlexVar(t);
    for (i=0; i<arity; i++) {
	putChr(' ');
	pPutOffset(arity-i);
    }
    putChr(' ');
    pLiftedStart(e,arity,"=");
}
#endif /* defined( DEBUG_SHOWSC ) */

/* --------------------------------------------------------------------------
 * Print type expression:
 * ------------------------------------------------------------------------*/

static Void local putSigType(Cell t)         /* print (possibly) generic type   */
{
    Int fr = 0;
    if (isPolyType(t)) {
	Kinds ks = polySigOf(t);
	for (; isAp(ks); ks=tl(ks))
	    fr++;
	t = monotypeOf(t);
    }

    putType(t,NEVER,fr);		/* Finally, print rest of type ... */
}

static Void local putContext(List ps, List qs, Int fr)	/* print context list		   */
{
    Int len = length(ps) + length(qs);
    Int c   = len;
    Bool useParens = len!=1;
    if (useParens)
	putChr('(');
    for (; nonNull(ps); ps=tl(ps)) {
	putPred(hd(ps),fr);
	if (--c > 0) {
	    putStr(", ");
	}
    }
    for (; nonNull(qs); qs=tl(qs)) {
	putPred(hd(qs),fr);
	if (--c > 0) {
	    putStr(", ");
	}
    }
    if (useParens)
	putChr(')');
}

static Void local putPred(Cell pi, Int fr)	/* Output predicate		   */
{
    if (isAp(pi)) {
	putPred(fun(pi),fr);
	putChr(' ');
	putType(arg(pi),ALWAYS,fr);
    }
    else if (isClass(pi)) {
        putModule(cclass(pi).mod);
        putStr(textToStr(cclass(pi).text));
    } else if (isCon(pi)) {
	putStr(textToStr(textOf(pi)));
    }
    else
	putStr("<unknownPredicate>");
}

static Void local putType(Cell t, Int prec, Int fr)	/* print nongeneric type expression*/
{
    switch(whatIs(t)) {
	case TYCON     : 
	                 putModule(tycon(t).mod);
	                 putStr(textToStr(tycon(t).text));
			 break;

	case TUPLE     : {   Int n = tupleOf(t);
			     putChr('(');
			     while (--n > 0)
				 putChr(',');
			     putChr(')');
			 }
			 break;

	case POLYTYPE  : {   Kinds ks = polySigOf(t);
			     OPEN(prec>=ARROW_PREC);
			     putStr("forall ");
			     for (; isAp(ks); ks=tl(ks)) {
				 putTyVar(fr++);
				 if (isAp(tl(ks)))
				     putChr(' ');
			     }
			     putStr(". ");
			     putType(monotypeOf(t),NEVER,fr);
			     CLOSE(prec>=ARROW_PREC);
			 }
			 break;

	case CDICTS    :
	case QUAL      : OPEN(prec>=ARROW_PREC);
			 if (whatIs(snd(snd(t)))==CDICTS) {
			     putContext(fst(snd(t)),fst(snd(snd(snd(t)))),fr);
			     putStr(" => ");
			     putType(snd(snd(snd(snd(t)))),NEVER,fr);
			 } else {
			     putContext(fst(snd(t)),NIL,fr);
			     putStr(" => ");
			     putType(snd(snd(t)),NEVER,fr);
			 }
			 CLOSE(prec>=ARROW_PREC);
			 break;

	case EXIST     :
	case RANK2     : putType(snd(snd(t)),prec,fr);
			 break;

	case OFFSET    : putTyVar(offsetOf(t));
			 break;

	case VARIDCELL :
	case VAROPCELL : putChr('_');
			 unlexVar(textOf(t));
			 break;

	case INTCELL   : putChr('_');
			 putInt(intOf(t));
			 break;

	case AP       : {   Cell typeHead = getHead(t);
			    Bool brackets = (argCount!=0 && prec>=ALWAYS);
			    Int  args	 = argCount;

			    if (typeHead==typeList) {
				if (argCount==1) {
				    putChr('[');
				    putType(arg(t),NEVER,fr);
				    putChr(']');
				    return;
				}
			    }
			    else if (typeHead==typeArrow) {
				if (argCount==2) {
				    OPEN(prec>=ARROW_PREC);
				    putType(arg(fun(t)),ARROW_PREC,fr);
				    putStr(" -> ");
				    putType(arg(t),NEVER,fr);
				    CLOSE(prec>=ARROW_PREC);
				    return;
				}
/*
				else if (argCount==1) {
				    putChr('(');
				    putType(arg(t),ARROW_PREC,fr);
				    putStr("->)");
				    return;
				}
*/
			    }
			    else if (isTuple(typeHead)) {
				if (argCount==tupleOf(typeHead)) {
				    putChr('(');
				    putTupleType(t,fr);
				    putChr(')');
				    return;
				}
			    }
			    OPEN(brackets);
			    putApType(t,args,fr);
			    CLOSE(brackets);
			}
			break;

	default       : putStr("(bad type)");
    }
}

static Void local putTyVar(Int n)           /* print type variable             */
{
    static String alphabet              /* for the benefit of EBCDIC :-)   */
		="abcdefghijklmnopqrstuvwxyz";
    putChr(alphabet[n%26]);
    if (n /= 26)                        /* just in case there are > 26 vars*/
	putInt(n);
}

static Bool local putTupleType(Cell e, Int fr)	/* print tuple of types, returning */
            /* TRUE if something was printed,  */
{				/* FALSE otherwise; used to control*/
    if (isAp(e)) {			/* printing of intermed. commas    */
	if (putTupleType(fun(e),fr))
	    putChr(',');
	putType(arg(e),NEVER,fr);
	return TRUE;
    }
    return FALSE;
}

static Void local putApType(Cell t, Int n, Int fr)	/* print type application          */
{
    if (n>0) {
	putApType(fun(t),n-1,fr);
	putChr(' ');
	putType(arg(t),ALWAYS,fr);
    }
    else
	putType(t,ALWAYS,fr);
}

/* --------------------------------------------------------------------------
 * Print kind expression:
 * ------------------------------------------------------------------------*/

static Void local putKind(Kind k)            /* print kind expression           */
{
    switch (whatIs(k)) {
	case AP      : if (isAp(fst(k))) {
			   putChr('(');
			   putKind(fst(k));
			   putChr(')');
		       }
		       else
			   putKind(fst(k));
		       if (whatIs(snd(k)) != NIL) {
			   putStr(" -> ");
			   putKind(snd(k));
		       }
		       break;


	case STAR    : putChr('*');
		       break;

	case OFFSET  : putTyVar(offsetOf(k));
		       break;

	case INTCELL : putChr('_');
		       putInt(intOf(k));
		       break;

	default      : putStr("(bad kind)");
    }
}

static Void local putKinds(Kinds ks)		/* Print list of kinds		   */
{
    if (isNull(ks))
	putStr("()");
    else if (nonNull(tl(ks))) {
	putChr('(');
	putKind(hd(ks));
	while (nonNull(ks=tl(ks))) {
	    putChr(',');
	    putKind(hd(ks));
	}
	putChr(')');
    }
    else
	putKind(hd(ks));
}

/* --------------------------------------------------------------------------
 * Print observations
 * ------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
 * Print qualified module name (if wanted):
 * ------------------------------------------------------------------------*/
Bool useQualifiedNames = FALSE;

static Void local putModule(Module m)            /* print module qualifier        */
{
  if (useQualifiedNames && !isPrelude(m)) { /* leave out "Prelude." qualifiers, too noisy. */
       putStr(textToStr(module(m).text));
       putChr('.');
    }
}
/* --------------------------------------------------------------------------
 * Main drivers:
 * ------------------------------------------------------------------------*/

Void printExp(FILE*fp, Cell e)			/* print expr on specified stream  */
{
    outputStream = fp;
    putDepth     = 0;
    put(NEVER,e);
}


Void printType(FILE*fp, Cell t)			/* print type on specified stream  */
{
    outputStream = fp;
    putSigType(t);
}

Void printContext(FILE*fp, List qs)		/* print context on spec. stream   */
{
    outputStream = fp;
    putContext(qs,NIL,0);
}

Void printPred(FILE*fp, Cell pi)			/* print predicate pi on stream    */
{
    outputStream = fp;
    putPred(pi,0);
}

Void printKind(FILE*fp, Kind k)			/* print kind k on stream          */
{
    outputStream = fp;
    putKind(k);
}

Void printKinds(FILE*fp, Kinds ks)			/* print list of kinds on stream   */
{
    outputStream = fp;
    putKinds(ks);
}

Void printFD(FILE* fp, Pair fd)			/* print functional dependency	   */
{
    List us;
    outputStream = fp;
    for (us=fst(fd); nonNull(us); us=tl(us)) {
        putTyVar(offsetOf(hd(us)));
	if (nonNull(tl(us))) {
	    putChr(' ');
	}
    }
    putStr(" -> ");
    for (us=snd(fd); nonNull(us); us=tl(us)) {
	putTyVar(offsetOf(hd(us)));
	if (nonNull(tl(us))) {
	    putChr(' ');
	}
    }
}


/*-------------------------------------------------------------------------*/
