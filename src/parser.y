/* --------------------------------------------------------------------------
 * Cuddles parser.  The lexer is in input.c; the two meet through the token
 * numbers and yylval in the generated parser.h, and through yylex.
 *
 * Regenerate parser.c and parser.h with:  bison -o parser.c parser.y
 * (in this directory, GNU Bison 3.8).  Expect 6 shift/reduce conflicts;
 * they are all resolved in an appropriate manner.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 * ------------------------------------------------------------------------*/

%code requires {
#define YYSTYPE			 Cell
}

%{
#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "errors.h"
#include "input.h"
#include "module.h"
#include "script.h"
#include "goal.h"
#include "machdep.h"	/* needPrims */

static void defTycon(int, Cell, Cell, Cell, Cell);
static Cell sigdecl(Cell, List, Type);
static Cell fixdecl(Cell, List, Syntax, Cell);
static Cell grded(List);
static Cell bang(Type);
static Cell only(Cell);
static Cell letrec(List, Cell);
static Cell qualType(List, Type);
static List exportSelf(void);
#define yyerror(s)		 /* errors handled elsewhere */

static Cell gcShadow(int, Cell);
static void syntaxError(char *);
static char *unexpected(void);
static Cell checkPrec(Cell);
static Cell buildTuple(List);
static List checkCtxt(List);
static Cell checkPred(Cell);
static Pair checkDo(List);
static Cell desugarGuards(List);
static Cell guardMatch(Cell, List, Cell);
static Cell checkTyLhs(Cell);
static Cell checkConstr(Cell);
static Cell starKind(Cell);
static Cell namedKind(Cell);



/* For the purposes of reasonably portable garbage collection, it is
 * necessary to simulate the YACC stack on the Cuddles stack to keep
 * track of all intermediate constructs.  The lexical analyser
 * pushes a token onto the stack for each token that is found, with
 * these elements being removed as reduce actions are performed,
 * taking account of look-ahead tokens as described by gcShadow()
 * below.
 *
 * Of the non-terminals used below, only start, topDecl & begin
 * do not leave any values on the Cuddles stack.  The same is true for the
 * terminals EXPR and SCRIPT.  At the end of a successful parse, there
 * should only be one element left on the stack, containing the result
 * of the parse.
 */


%}

%token EXPR       CTXT       SCRIPT
%token CASEXP     OF         DATA       TYPE       IF
%token THEN       ELSE       WHERE      LET        IN
%token INFIXN     INFIXL     INFIXR     PRIMITIVE  TNEWTYPE
%token DEFAULT    DERIVING   DO         TCLASS     TINSTANCE
%token REPEAT	  ALL        NUMLIT     CHARLIT	   STRINGLIT
%token VAROP      VARID	     CONOP      CONID
%token QVAROP     QVARID     QCONOP     QCONID
%token COCO       '='        UPTO       '@'        '\\'
%token '|'        '-'        FROM       ARROW      '~'
%token '!'        IMPLIES    '('        ','        ')'
%token '['        ';'        ']'        '`'	   '.'
%token TMODULE    IMPORT     HIDING     QUALIFIED  ASMOD
%token NEEDPRIMS
%token FOREIGN

%%
/*- Top level script/module structure -------------------------------------*/

start	  : EXPR exp wherePart		{inputExpr = letrec($3,$2); sp-=2;}
	  | CTXT classContext		{inputContext = $2;	    sp-=1;}
	  | SCRIPT topModule		{valDefns  = $2;	    sp-=1;}
	  | error			{syntaxError("input");}
	  ;

/*- Haskell module header/import parsing: -----------------------------------
 * Syntax for Haskell modules (module headers and imports) is parsed but
 * most of it is ignored.  However, module names in import declarations
 * are used, of course, if import chasing is turned on.
 *-------------------------------------------------------------------------*/

/* In Haskell 1.2, the default module header was "module Main where"
 * In 1.3, this changed to "module Main(main) where".
 * We use the 1.2 header because it breaks much less pre-module code.
 */
topModule : startMain begin modBody end {
					 setExportList(singleton(ap(MODULEENT,mkCon(moduleRec(currentModule)->text))));
					 $$ = gcShadow(3,$3);
					}
          | startMain '{' modBody '}'   {
					 setExportList(singleton(ap(MODULEENT,mkCon(moduleRec(currentModule)->text))));
					 $$ = gcShadow(4,$3);
					}
	  | TMODULE modname expspec WHERE '{' modBody end
					{setExportList($3);   $$ = gcShadow(7,$6);}
	  | TMODULE modname expspec WHERE error
					{syntaxError("declaration");}
	  | TMODULE error		{syntaxError("module definition");}
	  ;
/* To implement the Haskell module system, we have to keep track of the
 * current module.  We rely on the use of LALR parsing to ensure that this 
 * side effect happens before any declarations within the module.
 */
startMain : /* empty */			{startModule(conMain); 
					 $$ = gcShadow(0,NIL);}
	  ;
modname	  : qconid			{startModule(mkCon(mkNestedQual($1))); $$ = gcShadow(1,NIL);}
	  ;
modid	  : qconid			{$$ = mkCon(mkNestedQual($1));}
	  | STRINGLIT			{ char *modName = findPathname(textToStr(textOf($1)));
					  if (modName) { /* fillin pathname if known */
					      $$ = mkStr(findText(modName));
					  } else {
					      $$ = $1;
					  }
					}
	  ;
modBody	  : /* empty */                 {$$ = gcShadow(0,NIL); }
          | ';' modBody 		{$$ = gcShadow(2,$2);}
          | topDecls			{$$ = gcShadow(1,$1);}
	  | impDecls chase		{$$ = gcShadow(2,NIL);}
	  | impDecls ';' chase topDecls	{$$ = gcShadow(4,$4);}
	  ;

/*- Exports: --------------------------------------------------------------*/

expspec	  : /* empty */			{$$ = gcShadow(0,exportSelf());}
	  | '(' ')'			{$$ = gcShadow(2,NIL);}
	  | '(' ',' ')'			{$$ = gcShadow(3,NIL);}
	  | '(' exports ')'		{$$ = gcShadow(3,$2);}
	  | '(' exports ',' ')'		{$$ = gcShadow(4,$2);}
	  ;
exports	  : exports ',' export		{$$ = gcShadow(3,cons($3,$1));}
	  | export			{$$ = gcShadow(1,singleton($1));}
	  ;
/* The qcon should be qconid.  
 * Relaxing the rule lets us explicitly export (:) from the Prelude.
 */
export	  : qvar			{$$ = $1;}
	  | qcon			{$$ = $1;}
	  | qconid '(' UPTO ')'		{$$ = gcShadow(4,pair($1,DOTDOT));}
	  | qconid '(' qnames ')'	{$$ = gcShadow(4,pair($1,$3));}
	  | TMODULE modid		{$$ = gcShadow(2,ap(MODULEENT,$2));}
	  ;
qnames	  : /* empty */			{$$ = gcShadow(0,NIL);}
	  | ','				{$$ = gcShadow(1,NIL);}
	  | qnames1			{$$ = $1;}
	  | qnames1 ','			{$$ = gcShadow(2,$1);}
	  ;
qnames1	  : qnames1 ',' qname		{$$ = gcShadow(3,cons($3,$1));}
	  | qname			{$$ = gcShadow(1,singleton($1));}
	  ;
qname	  : qvar			{$$ = $1;}
	  | qcon			{$$ = $1;}
	  ;

/*- Import declarations: --------------------------------------------------*/

impDecls  : impDecls ';' impDecl	{imps = cons($3,imps); $$=gcShadow(3,NIL);}
	  | impDecls ';'		{$$   = gcShadow(2,NIL); }
	  | impDecl			{imps = singleton($1); $$=gcShadow(1,NIL);}
	  ;
chase	  : /* empty */			{if (chase(imps)) {
					     clearStack();
					     onto(imps);
					     done();
					     closeAnyInput();
					     return 0;
					 }
					 $$ = gcShadow(0,NIL);
					}
	  ;
/* Note that qualified import ignores the import list. */
impDecl	  : IMPORT modid impspec	{addUnqualImport($2,NIL,$3);
					 $$ = gcShadow(3,$2);}
	  | IMPORT modid ASMOD modid impspec
					{addUnqualImport($2,$4,$5);
					 $$ = gcShadow(5,$2);}
	  | IMPORT QUALIFIED modid ASMOD modid impspec
					{addQualImport($3,$5,$6);
					 $$ = gcShadow(6,$3);}
	  | IMPORT QUALIFIED modid impspec
					{addQualImport($3,$3,$4);
					 $$ = gcShadow(4,$3);}
	  | IMPORT error		{syntaxError("import declaration");}
	  ;
impspec	  : /* empty */			{$$ = gcShadow(0,DOTDOT);}
	  | HIDING '(' imports ')'	{$$ = gcShadow(4,ap(HIDDEN,$3));}
	  | '(' imports ')'		{$$ = gcShadow(3,$2);}
	  ;
imports	  : /* empty */			{$$ = gcShadow(0,NIL);}
	  | ','				{$$ = gcShadow(1,NIL);}
	  | imports1			{$$ = $1;}
	  | imports1 ','		{$$ = gcShadow(2,$1);}
	  ;
imports1  : imports1 ',' import		{$$ = gcShadow(3,cons($3,$1));}
	  | import			{$$ = gcShadow(1,singleton($1));}
	  ;
import	  : var				{$$ = $1;}
	  | CONID			{$$ = gcShadow(1,pair($1,NONE));}
	  | CONID '(' UPTO ')'		{$$ = gcShadow(4,pair($1,DOTDOT));}
	  | CONID '(' names ')'		{$$ = gcShadow(4,pair($1,$3));}
	  ;
names	  : /* empty */			{$$ = gcShadow(0,NIL);}
	  | ','				{$$ = gcShadow(1,NIL);}
	  | names1			{$$ = $1;}
	  | names1 ','			{$$ = gcShadow(2,$1);}
	  ;
names1	  : names1 ',' name		{$$ = gcShadow(3,cons($3,$1));}
	  | name			{$$ = gcShadow(1,singleton($1));}
	  ;
name	  : var				{$$ = $1;}
	  | con				{$$ = $1;}
	  ;

/*- Top-level declarations: -----------------------------------------------*/

topDecls  : topDecls ';'		{$$ = gcShadow(2,$1);}
          | topDecls ';' topDecl	{$$ = gcShadow(2,$1);}
	  | topDecls ';' decl		{$$ = gcShadow(3,cons($3,$1));}
	  | topDecl			{$$ = gcShadow(0,NIL);}
	  | decl			{$$ = gcShadow(1,cons($1,NIL));}
	  ;

/*- Type declarations: ----------------------------------------------------*/

topDecl	  : TYPE tyLhs '=' type		{defTycon(4,$3,$2,$4,SYNONYM);}
	  | TYPE tyLhs '=' type IN invars
					{defTycon(6,$3,$2,
						    ap($4,$6),RESTRICTSYN);}
	  | TYPE CONID COCO kind	{kindSigDefn(intOf($3),textOf($2),$4);
					 sp-=4;}
	  | TYPE error			{syntaxError("type declaration");}
	  | DATA btype2 '=' constrs deriving
					{defTycon(5,$3,checkTyLhs($2),
						    ap(rev($4),$5),DATATYPE);}
	  | DATA context IMPLIES tyLhs '=' constrs deriving
					{defTycon(7,$5,$4,
						  ap(qualType($2,rev($6)),
						     $7),DATATYPE);}
	  | DATA btype2			{defTycon(2,$1,checkTyLhs($2),
						    ap(NIL,NIL),DATATYPE);}
	  | DATA context IMPLIES tyLhs  {defTycon(4,$1,$4,
						  ap(qualType($2,NIL),
						     NIL),DATATYPE);}
	  | DATA btype2 WHERE gadtCons deriving
					{defTycon(5,$3,checkTyLhs($2),
						    ap(gadtConstrs($3,$2,$4,DATATYPE),
						       $5),DATATYPE);}
	  | DATA btype2 COCO kind WHERE gadtCons deriving
					{Cell lhs = kindedHeader($3,checkTyLhs($2),$4);
					 defTycon(7,$5,lhs,
						    ap(gadtConstrs($5,lhs,$6,DATATYPE),
						       $7),DATATYPE);}
	  | DATA error			{syntaxError("data declaration");}
	  | TNEWTYPE btype2 '=' nconstr deriving
					{defTycon(5,$3,checkTyLhs($2),
						    ap($4,$5),NEWTYPE);}
	  | TNEWTYPE context IMPLIES tyLhs '=' nconstr deriving
					{defTycon(7,$5,$4,
						  ap(qualType($2,$6),
						     $7),NEWTYPE);}
	  | TNEWTYPE btype2 WHERE gadtCons deriving
					{defTycon(5,$3,checkTyLhs($2),
						    ap(gadtConstrs($3,$2,$4,NEWTYPE),
						       $5),NEWTYPE);}
	  | TNEWTYPE error		{syntaxError("newtype declaration");}
	  | NEEDPRIMS NUMLIT		{if (isInt($2)) {
					     needPrims(intOf($2), NULL);
					 } else {
					     syntaxError("needprims decl");
					 }
					 sp-=2;}
	  | NEEDPRIMS error		{syntaxError("needprims decl");}
	  ;
tyLhs	  : tyLhs varid			{$$ = gcShadow(2,ap($1,$2));}
	  | tyLhs kindedVar		{$$ = gcShadow(2,ap($1,$2));}
	  | CONID			{$$ = $1;}
	  | error			{syntaxError("type defn lhs");}
	  ;
invars	  : invars ',' invar		{$$ = gcShadow(3,cons($3,$1));}
	  | invar			{$$ = gcShadow(1,cons($1,NIL));}
	  ;
invar	  : var COCO topType		{$$ = gcShadow(3,sigdecl($2,singleton($1),
									$3));}
	  | var				{$$ = $1;}
	  ;
constrs	  : constrs '|' pconstr		{$$ = gcShadow(3,cons($3,$1));}
	  | pconstr			{$$ = gcShadow(1,cons($1,NIL));}
	  ;
pconstr	  : ALL varids '.' qconstr	{$$ = gcShadow(4,ap(POLYTYPE,
						     pair(rev($2),$4)));}
	  | constr			{$$ = $1;}
	  ;
qconstr	  : context IMPLIES constr	{$$ = gcShadow(3,qualType($1,$3));}
	  | constr			{$$ = $1;}
	  ;
constr	  : '!' btype conop bbtype	{$$ = gcShadow(4,ap(ap($3,bang($2)),$4));}
	  | btype1    conop bbtype	{$$ = gcShadow(3,ap(ap($2,$1),$3));}
	  | btype2    conop bbtype	{$$ = gcShadow(3,ap(ap($2,$1),$3));}
	  | bpolyType conop bbtype	{$$ = gcShadow(3,ap(ap($2,$1),$3));}
	  | btype2			{$$ = checkConstr($1);}
	  | btype3			{$$ = checkConstr($1);}
	  | con '{' fieldspecs '}'	{$$ = gcShadow(4,ap(LABC,pair($1,rev($3))));}
	  | con '{' '}'			{$$ = gcShadow(3,ap(LABC,pair($1,NIL)));}
	  | error			{syntaxError("data type declaration");}
	  ;
btype3	  : btype2 '!' atype		{$$ = gcShadow(3,ap($1,bang($3)));}
	  | btype2 bpolyType		{$$ = gcShadow(2,ap($1,$2));}
	  | btype3 atype		{$$ = gcShadow(2,ap($1,$2));}
	  | btype3 '!' atype		{$$ = gcShadow(3,ap($1,bang($3)));}
	  | btype3 bpolyType		{$$ = gcShadow(2,ap($1,$2));}
	  | '(' CONOP ')'		{$$ = gcShadow(3,$2);}
	  ;
bbtype	  : '!' btype			{$$ = gcShadow(2,bang($2));}
	  | btype			{$$ = $1;}
	  | bpolyType			{$$ = $1;}
	  ;
nconstr	  : pconstr			{$$ = gcShadow(1,singleton($1));}
	  ;

/*- GADT syntax: the constructors as signatures, one per line, in a where
 *  block.  gadtConstrs (tycon.c) turns each into the constructor the
 *  equation form would have had. ---------------------------------------*/

gadtCons  : '{' gadtCons0 end		{$$ = gcShadow(3,rev($2));}
	  | '{' gadtCons1 end		{$$ = gcShadow(3,rev($2));}
	  ;
gadtCons0 : /* empty */			{$$ = gcShadow(0,NIL);}
	  | gadtCons0 ';'		{$$ = gcShadow(2,$1);}
	  | gadtCons1 ';'		{$$ = gcShadow(2,$1);}
	  ;
gadtCons1 : gadtCons0 gadtCon		{$$ = gcShadow(2,cons($2,$1));}
	  ;
gadtCon	  : conids COCO gadtType	{$$ = gcShadow(3,triple($2,rev($1),$3));}
	  ;
conids	  : conids ',' con		{$$ = gcShadow(3,cons($3,$1));}
	  | con				{$$ = gcShadow(1,singleton($1));}
	  ;
gadtType  : ALL varids '.' gadtType0	{$$ = gcShadow(4,ap(POLYTYPE,
						     pair(rev($2),$4)));}
	  | gadtType0			{$$ = $1;}
	  ;
gadtType0 : context IMPLIES gadtType1	{$$ = gcShadow(3,qualType($1,$3));}
	  | gadtType1			{$$ = $1;}
	  ;
gadtType1 : gadtArg ARROW gadtType1	{$$ = gcShadow(3,fn($1,$3));}
	  | btype2			{$$ = $1;}
	  | '{' fieldspecs '}' ARROW btype2
					{$$ = gcShadow(5,ap(LABC,pair(rev($2),$5)));}
	  ;
gadtArg	  : btype			{$$ = $1;}
	  | '!' atype			{$$ = gcShadow(2,bang($2));}
	  | bpolyType			{$$ = $1;}
	  ;
fieldspecs: fieldspecs ',' fieldspec	{$$ = gcShadow(3,cons($3,$1));}
	  | fieldspec			{$$ = gcShadow(1,cons($1,NIL));}
	  ;
fieldspec : vars COCO polyType		{$$ = gcShadow(3,pair(rev($1),$3));}
	  | vars COCO type		{$$ = gcShadow(3,pair(rev($1),$3));}
	  | vars COCO '!' type		{$$ = gcShadow(4,pair(rev($1),bang($4)));}
	  ;
deriving  : /* empty */			{$$ = gcShadow(0,NIL);}
	  | DERIVING qconid		{$$ = gcShadow(2,singleton($2));}
	  | DERIVING '(' derivs0 ')'	{$$ = gcShadow(4,$3);}
	  ;
derivs0	  : /* empty */			{$$ = gcShadow(0,NIL);}
	  | derivs			{$$ = gcShadow(1,rev($1));}
	  ;
derivs	  : derivs ',' qconid		{$$ = gcShadow(3,cons($3,$1));}
	  | qconid			{$$ = gcShadow(1,singleton($1));}
	  ;

/*- Processing definitions of primitives ----------------------------------*/

topDecl	  : PRIMITIVE prims COCO topType{primDefn($1,$2,$4); sp-=4;}
	  ;
prims	  : prims ',' prim		{$$ = gcShadow(3,cons($3,$1));}
	  | prim			{$$ = gcShadow(1,cons($1,NIL));}
	  | error			{syntaxError("primitive defn");}
	  ;
prim	  : var STRINGLIT		{$$ = gcShadow(2,pair($1,$2));}
	  | var				{$$ = $1;}
	  ;

/*- Foreign Function Interface --------------------------------------------*/

topDecl   : FOREIGN IMPORT var STRINGLIT var COCO topType 
               {foreignImport($1,$3,NIL,$4,$5,$7); sp-=7;}
          | FOREIGN IMPORT var var COCO topType 
               {foreignImport($1,$3,NIL,$4,$4,$6); sp-=6;}
          | FOREIGN IMPORT var var STRINGLIT var COCO topType 
               {foreignImport($1,$3,$4,$5,$6,$8); sp-=8;}
          | FOREIGN IMPORT var var var COCO topType 
               {foreignImport($1,$3,$4,$5,$5,$7); sp-=7;}
          | FOREIGN var    var STRINGLIT var COCO topType 
               {foreignExport($1,$2,$3,$4,$5,$7); sp-=7;}
	  ;

/*- Class declarations: ---------------------------------------------------*/

topDecl	  : TCLASS crule fds wherePart	{classDefn(intOf($1),$2,$4,$3); sp-=4;}
	  | TINSTANCE irule wherePart	{instDefn(intOf($1),$2,$3);  sp-=3;}
	  | DEFAULT '(' dtypes ')'	{defaultDefn(intOf($1),$3);  sp-=4;}
	  | TCLASS error		{syntaxError("class declaration");}
	  | TINSTANCE error		{syntaxError("instance declaration");}
	  | DEFAULT error		{syntaxError("default declaration");}
	  ;
crule	  : context IMPLIES btype2	{$$ = gcShadow(3,pair($1,checkPred($3)));}
	  | btype2			{$$ = gcShadow(1,pair(NIL,checkPred($1)));}
	  ;
irule	  : context IMPLIES btype2	{$$ = gcShadow(3,pair($1,checkPred($3)));}
	  | btype2			{$$ = gcShadow(1,pair(NIL,checkPred($1)));}
	  ;
dtypes	  : /* empty */			{$$ = gcShadow(0,NIL);}
	  | dtypes1			{$$ = gcShadow(1,rev($1));}
	  ;
dtypes1	  : dtypes1 ',' type		{$$ = gcShadow(3,cons($3,$1));}
	  | type			{$$ = gcShadow(1,cons($1,NIL));}
	  ;
fds	  : /* empty */			{$$ = gcShadow(0,NIL);}
	  | '|' fds1			{$$ = gcShadow(2,rev($2));}
	  ;
fds1	  : fds1 ',' fd			{$$ = gcShadow(3,cons($3,$1));}
	  | fd				{$$ = gcShadow(1,cons($1,NIL));}
	  ;
fd	  : varids0 ARROW varids0	{$$ = gcShadow(3,pair(rev($1),rev($3)));}
	  | error			{syntaxError("functional dependency");}
	  ;
varids0   : /* empty */			{$$ = gcShadow(0,NIL);}
	  | varids0 varid		{$$ = gcShadow(2,cons($2,$1));}
	  ;

/*- Type expressions: -----------------------------------------------------*/

topType	  : ALL varids '.' topType0	{$$ = gcShadow(4,ap(POLYTYPE,
						     pair(rev($2),$4)));}
	  | topType0			{$$ = $1;}
	  ;
topType0  : context IMPLIES topType1	{$$ = gcShadow(3,qualType($1,$3));}
	  | topType1			{$$ = $1;}
	  ;
topType1  : bpolyType ARROW topType1	{$$ = gcShadow(3,fn($1,$3));}
	  | btype1    ARROW topType1	{$$ = gcShadow(3,fn($1,$3));}
	  | btype2    ARROW topType1	{$$ = gcShadow(3,fn($1,$3));}
	  | btype			{$$ = $1;}
	  ;
polyType  : ALL varids '.' sigType	{$$ = gcShadow(4,ap(POLYTYPE,
						     pair(rev($2),$4)));}
	  | bpolyType			{$$ = $1;}
	  ;
bpolyType : '(' polyType ')'		{$$ = gcShadow(3,$2);}
	  ;
varids	  : varids varid		{$$ = gcShadow(2,cons($2,$1));}
	  | varid			{$$ = gcShadow(1,singleton($1));}
	  ;
sigType   : context IMPLIES type	{$$ = gcShadow(3,qualType($1,$3));}
	  | type			{$$ = $1;}
	  ;
context	  : classContext		{$$ = $1;}
	  | eqpred			{$$ = gcShadow(1,singleton($1));}
	  | '(' eqpred ')'		{$$ = gcShadow(3,singleton($2));}
	  | '(' preds ')'		{$$ = gcShadow(3,checkCtxt(rev($2)));}
 	  ;
/* A context of class predicates alone; it is what the :xplain command
 * reads, and its parenthesised forms are shared with tuple types.  An
 * equality can only be a context, so a list with one in it has a
 * nonterminal of its own. */
classContext
	  : '(' ')'			{$$ = gcShadow(2,NIL);}
	  | btype2			{$$ = gcShadow(1,singleton(checkPred($1)));}
	  | '(' btype2 ')'		{$$ = gcShadow(3,singleton(checkPred($2)));}
	  | '(' btypes2 ')'		{$$ = gcShadow(3,checkCtxt(rev($2)));}
 	  ;
preds	  : preds ',' pred		{$$ = gcShadow(3,cons($3,$1));}
	  | btypes2 ',' eqpred		{$$ = gcShadow(3,cons($3,$1));}
	  | btype2  ',' eqpred		{$$ = gcShadow(3,cons($3,cons($1,NIL)));}
	  | eqpred  ',' pred		{$$ = gcShadow(3,cons($3,cons($1,NIL)));}
	  ;
pred	  : btype2			{$$ = $1;}
	  | eqpred			{$$ = $1;}
	  ;
eqpred	  : btype '~' btype		{$$ = gcShadow(3,ap(ap(EQUALITY,$1),$3));}
	  ;
type	  : type1			{$$ = $1;}
	  | btype2			{$$ = $1;}
	  ;
type1	  : btype1			{$$ = $1;}
	  | bpolyType ARROW type	{$$ = gcShadow(3,fn($1,$3));}
	  | btype1    ARROW type	{$$ = gcShadow(3,fn($1,$3));}
	  | btype2    ARROW type	{$$ = gcShadow(3,fn($1,$3));}
	  | error			{syntaxError("type expression");}
	  ;
btype	  : btype1			{$$ = $1;}
	  | btype2			{$$ = $1;}
	  ;
btype1	  : btype1 atype		{$$ = gcShadow(2,ap($1,$2));}
	  | atype1			{$$ = $1;}
	  ;
btype2	  : btype2 atype		{$$ = gcShadow(2,ap($1,$2));}
	  | qconid			{$$ = $1;}
	  ;
atype	  : atype1			{$$ = $1;}
	  | qconid			{$$ = $1;}
	  ;
atype1	  : varid			{$$ = $1;}
	  | '(' ')'			{$$ = gcShadow(2,typeUnit);}
	  | '(' ARROW ')'		{$$ = gcShadow(3,typeArrow);}
	  | '(' type1 ')'		{$$ = gcShadow(3,$2);}
	  | '(' btype2 ')'		{$$ = gcShadow(3,$2);}
	  | '(' tupCommas ')'		{$$ = gcShadow(3,$2);}
	  | '(' btypes2 ')'		{$$ = gcShadow(3,buildTuple($2));}
	  | '(' typeTuple ')'		{$$ = gcShadow(3,buildTuple($2));}
	  | '[' type ']'		{$$ = gcShadow(3,ap(typeList,$2));}
	  | '[' ']'			{$$ = gcShadow(2,typeList);}
	  | '_'				{$$ = gcShadow(1,inventVar());}
	  | kindedVar			{$$ = $1;}
	  ;
btypes2	  : btypes2 ',' btype2		{$$ = gcShadow(3,cons($3,$1));}
	  | btype2  ',' btype2		{$$ = gcShadow(3,cons($3,cons($1,NIL)));}
	  ;
typeTuple : type1     ',' type		{$$ = gcShadow(3,cons($3,cons($1,NIL)));}
	  | btype2    ',' type1		{$$ = gcShadow(3,cons($3,cons($1,NIL)));}
	  | btypes2   ',' type1		{$$ = gcShadow(3,cons($3,$1));}
	  | typeTuple ',' type		{$$ = gcShadow(3,cons($3,$1));}
	  ;

/*- Kinds: ----------------------------------------------------------------
 * Written in a standalone signature, type T :: kind, and on the variables a
 * declaration binds, data T (f :: kind) = ...  A kind is a tree over STAR
 * (written * or Type), CONSTRAINT and ->, with no variables.
 */

kind	  : akind ARROW kind		{$$ = gcShadow(3,ap($1,$3));}
	  | akind			{$$ = $1;}
	  ;
akind	  : VAROP			{$$ = gcShadow(1,starKind($1));}
	  | CONID			{$$ = gcShadow(1,namedKind($1));}
	  | '(' kind ')'		{$$ = gcShadow(3,$2);}
	  ;
kindedVar : '(' varid COCO kind ')'	{$$ = gcShadow(5,ap(KINDED,pair($2,$4)));}
	  ;

/*- Value declarations: ---------------------------------------------------*/

gendecl	  : INFIXN optDigit ops		{$$ = gcShadow(3,fixdecl($1,$3,NON_ASS,$2));}
	  | INFIXN error		{syntaxError("fixity decl");}
	  | INFIXL optDigit ops		{$$ = gcShadow(3,fixdecl($1,$3,LEFT_ASS,$2));}
	  | INFIXL error		{syntaxError("fixity decl");}
	  | INFIXR optDigit ops		{$$ = gcShadow(3,fixdecl($1,$3,RIGHT_ASS,$2));}
	  | INFIXR error		{syntaxError("fixity decl");}
	  | vars COCO topType		{$$ = gcShadow(3,sigdecl($2,$1,$3));}
	  | vars COCO error		{syntaxError("type signature");}
	  ;
optDigit  : NUMLIT			{$$ = gcShadow(1,checkPrec($1));}
	  | /* empty */			{$$ = gcShadow(0,mkInt(DEF_PREC));}
	  ;
ops	  : ops ',' op			{$$ = gcShadow(3,cons($3,$1));}
	  | op				{$$ = gcShadow(1,singleton($1));}
	  ;
vars	  : vars ',' var		{$$ = gcShadow(3,cons($3,$1));}
	  | var				{$$ = gcShadow(1,singleton($1));}
	  ;
decls	  : '{' decls0 end		{$$ = gcShadow(3,$2);}
	  | '{' decls1 end		{$$ = gcShadow(3,$2);}
	  ;
decls0	  : /* empty */			{$$ = gcShadow(0,NIL);}
	  | decls0 ';'			{$$ = gcShadow(2,$1);}
	  | decls1 ';'			{$$ = gcShadow(2,$1);}
	  ;
decls1	  : decls0 decl			{$$ = gcShadow(2,cons($2,$1));}
	  ;
decl	  : gendecl			{$$ = $1;}
	  | funlhs rhs			{$$ = gcShadow(2,ap(FUNBIND,pair($1,$2)));}
	  | funlhs COCO type rhs	{$$ = gcShadow(4,ap(FUNBIND,
						     pair($1,ap(RSIGN,
								ap($4,$3)))));}
	  | pat0 rhs			{$$ = gcShadow(2,ap(PATBIND,pair($1,$2)));}
	  ;
funlhs	  : funlhs0			{$$ = $1;}
	  | funlhs1			{$$ = $1;}
	  ;
funlhs0	  : pat10_vI varop    pat0	{$$ = gcShadow(3,ap2($2,$1,$3));}
	  | var      '+'      pat0	{$$ = gcShadow(3,ap2(varPlus,$1,$3));}
	  | infixPat varop    pat0	{$$ = gcShadow(3,ap2($2,$1,$3));}
	  | NUMLIT   varop    pat0	{$$ = gcShadow(3,ap2($2,$1,$3));}
	  | var      varop_pl pat0	{$$ = gcShadow(3,ap2($2,$1,$3));}
	  ;
funlhs1	  : '(' funlhs0 ')' apat	{$$ = gcShadow(4,ap($2,$4));}
	  | '(' funlhs1 ')' apat	{$$ = gcShadow(4,ap($2,$4));}
	  | var     apat		{$$ = gcShadow(2,ap($1,$2));}
	  | funlhs1 apat		{$$ = gcShadow(2,ap($1,$2));}
	  ;
rhs	  : rhs1 wherePart		{$$ = gcShadow(2,letrec($2,$1));}
	  | error			{syntaxError("declaration");}
	  ;
rhs1	  : '=' exp			{$$ = gcShadow(2,pair($1,$2));}
	  | gdrhs			{$$ = gcShadow(1,desugarGuards(rev($1)));}
	  ;
gdrhs	  : gdrhs gddef			{$$ = gcShadow(2,cons($2,$1));}
	  | gddef			{$$ = gcShadow(1,singleton($1));}
	  ;
gddef	  : '|' quals '=' exp		{$$ = gcShadow(4,pair($3,pair(rev($2),$4)));}
	  ;
wherePart : /* empty */			{$$ = gcShadow(0,NIL);}
	  | WHERE decls			{$$ = gcShadow(2,$2);}
	  ;
/*- Patterns: -------------------------------------------------------------*/

pat	  : pat_npk			{$$ = $1;}
	  ;
pat_npk	  : pat0 COCO type		{$$ = gcShadow(3,ap(ESIGN,pair($1,$3)));}
	  | pat0			{$$ = $1;}
	  ;
pat0	  : var				{$$ = $1;}
	  | NUMLIT			{$$ = $1;}
	  | pat0_vI			{$$ = $1;}
	  ;
pat0_vI	  : pat10_vI			{$$ = $1;}
	  | infixPat			{$$ = gcShadow(1,ap(INFIX,$1));}
	  ;
infixPat  : '-' pat10			{$$ = gcShadow(2,ap(NEG,only($2)));}
	  | '-' error			{syntaxError("pattern");}
	  | var qconop pat10		{$$ = gcShadow(3,ap(ap($2,only($1)),$3));}
	  | var qconop '-' pat10	{$$ = gcShadow(4,ap(NEG,ap2($2,only($1),$4)));}
	  | NUMLIT qconop pat10		{$$ = gcShadow(3,ap(ap($2,only($1)),$3));}
	  | NUMLIT qconop '-' pat10	{$$ = gcShadow(4,ap(NEG,ap2($2,only($1),$4)));}
	  | pat10_vI qconop pat10	{$$ = gcShadow(3,ap(ap($2,only($1)),$3));}
	  | pat10_vI qconop '-' pat10	{$$ = gcShadow(4,ap(NEG,ap2($2,only($1),$4)));}
	  | infixPat qconop pat10	{$$ = gcShadow(3,ap(ap($2,$1),$3));}
	  | infixPat qconop '-' pat10	{$$ = gcShadow(4,ap(NEG,ap(ap($2,$1),$4)));}
	  ;
pat10	  : fpat			{$$ = $1;}
	  | apat			{$$ = $1;}
	  ;
pat10_vI  : fpat			{$$ = $1;}
	  | apat_vI			{$$ = $1;}
	  ;
fpat	  : fpat apat			{$$ = gcShadow(2,ap($1,$2));}
	  | gcon apat			{$$ = gcShadow(2,ap($1,$2));}
	  ;
apat	  : NUMLIT			{$$ = $1;}
	  | var				{$$ = $1;}
	  | apat_vI			{$$ = $1;}
	  ;
apat_vI	  : var '@' apat		{$$ = gcShadow(3,ap(ASPAT,pair($1,$3)));}
	  | gcon			{$$ = $1;}
	  | qcon '{' patbinds '}'	{$$ = gcShadow(4,ap(CONFLDS,pair($1,$3)));}
	  | CHARLIT			{$$ = $1;}
	  | STRINGLIT			{$$ = $1;}
	  | '_'				{$$ = gcShadow(1,WILDCARD);}
	  | '(' pat_npk ')'		{$$ = gcShadow(3,$2);}
	  | '(' pats2 ')'		{$$ = gcShadow(3,buildTuple($2));}
	  | '[' pats1 ']'		{$$ = gcShadow(3,ap(FINLIST,rev($2)));}
	  | '~' apat			{$$ = gcShadow(2,ap(LAZYPAT,$2));}
	  ;
pats2	  : pats2 ',' pat		{$$ = gcShadow(3,cons($3,$1));}
	  | pat ',' pat			{$$ = gcShadow(3,cons($3,singleton($1)));}
	  ;
pats1	  : pats1 ',' pat		{$$ = gcShadow(3,cons($3,$1));}
	  | pat				{$$ = gcShadow(1,singleton($1));}
	  ;
patbinds  : /* empty */			{$$ = gcShadow(0,NIL);}
	  | patbinds1			{$$ = gcShadow(1,rev($1));}
	  ;
patbinds1 : patbinds1 ',' patbind	{$$ = gcShadow(3,cons($3,$1));}
	  | patbind			{$$ = gcShadow(1,singleton($1));}
	  ;
patbind	  : qvar '=' pat		{$$ = gcShadow(3,pair($1,$3));}
	  | var				{$$ = $1;}
	  ;

/*- Expressions: ----------------------------------------------------------*/

exp	  : exp_err			{$$ = $1;}
	  | error			{syntaxError("expression");}
	  ;
exp_err	  : exp0a COCO sigType		{$$ = gcShadow(3,ap(ESIGN,pair($1,$3)));}
	  | exp0			{$$ = $1;}
	  ;
exp0	  : exp0a			{$$ = $1;}
	  | exp0b			{$$ = $1;}
	  ;
/* The `a' and `b' forms below are not an accident and must stay in step.
 * An `exp10b' is one of the forms whose body extends as far to the right as
 * it can -- a lambda, a `let', an `if', a `do' without braces -- and Haskell
 * allows one of those only as the LAST operand of an operator chain:
 * `True && \x -> x' parses, and `\x -> x + 1' is a lambda whose body is
 * `x + 1', never `(\x -> x) + 1'.  `infixExpb' is therefore `infixExpa'
 * with the final operand changed, and it is spelled out because bison has
 * no way to parameterise a rule by a nonterminal.  Merging the two would
 * admit a greedy form in every operand position and make `\x -> x + 1'
 * ambiguous.  Edit one and you must edit the other. */

exp0a	  : infixExpa			{$$ = gcShadow(1,ap(INFIX,$1));}
	  | exp10a			{$$ = $1;}
	  ;
exp0b	  : infixExpb			{$$ = gcShadow(1,ap(INFIX,$1));}
	  | exp10b			{$$ = $1;}
	  ;
infixExpa : infixExpa qop '-' exp10a	{$$ = gcShadow(4,ap(NEG,ap(ap($2,$1),$4)));}
	  | infixExpa qop exp10a	{$$ = gcShadow(3,ap(ap($2,$1),$3));}
	  | '-' exp10a			{$$ = gcShadow(2,ap(NEG,only($2)));}
	  | exp10a qop '-' exp10a	{$$ = gcShadow(4,ap(NEG,
						     ap(ap($2,only($1)),$4)));}
	  | exp10a qop exp10a		{$$ = gcShadow(3,ap(ap($2,only($1)),$3));}
	  ;
infixExpb : infixExpa qop '-' exp10b	{$$ = gcShadow(4,ap(NEG,ap(ap($2,$1),$4)));}
	  | infixExpa qop exp10b	{$$ = gcShadow(3,ap(ap($2,$1),$3));}
	  | '-' exp10b			{$$ = gcShadow(2,ap(NEG,only($2)));}
	  | exp10a qop '-' exp10b	{$$ = gcShadow(4,ap(NEG,
						     ap(ap($2,only($1)),$4)));}
	  | exp10a qop exp10b		{$$ = gcShadow(3,ap(ap($2,only($1)),$3));}
	  ;
exp10a	  : CASEXP exp OF '{' alts end	{$$ = gcShadow(6,ap(CASE,pair($2,rev($5))));}
	  | DO '{' stmts end		{$$ = gcShadow(4,ap(DOCOMP,checkDo($3)));}
	  | appExp			{$$ = $1;}
	  ;
exp10b	  : '\\' pats ARROW exp		{$$ = gcShadow(4,ap(LAMBDA,      
						     pair(rev($2),
							  pair($3,$4))));}
	  | LET decls IN exp		{$$ = gcShadow(4,letrec($2,$4));}
	  | IF exp then_exp else_exp	{$$ = gcShadow(4,ap(COND,triple($2,$3,$4)));}
	  ;
/* Allow optional semicolons before 'then' and 'else' (as suggested by
   John Meacham), to remove a common pitfall when using if-then-else
   inside do expressions with implicit layout. */
then_exp  : ';' THEN exp		{$$ = gcShadow(3,$3);}
	  | THEN exp			{$$ = gcShadow(2,$2);}
	  ;
else_exp  : ';' ELSE exp		{$$ = gcShadow(3,$3);}
	  | ELSE exp			{$$ = gcShadow(2,$2);}
	  ;

pats      : pats apat			{$$ = gcShadow(2,cons($2,$1));}
	  | apat			{$$ = gcShadow(1,cons($1,NIL));}
	  ;
appExp    : appExp aexp			{$$ = gcShadow(2,ap($1,$2));}
	  | aexp			{$$ = $1;}
	  ;
aexp	  : qvar			{$$ = $1;}
	  | qvar '@' aexp		{$$ = gcShadow(3,ap(ASPAT,pair($1,$3)));}
	  | '~' aexp			{$$ = gcShadow(2,ap(LAZYPAT,$2));}
	  | '_'				{$$ = gcShadow(1,WILDCARD);}
	  | gcon			{$$ = $1;}
	  | qcon '{' fbinds '}'		{$$ = gcShadow(4,ap(CONFLDS,pair($1,$3)));}
	  | aexp '{' fbinds '}'		{$$ = gcShadow(4,ap(UPDFLDS,
						     triple($1,NIL,$3)));}
	  | NUMLIT			{$$ = $1;}
	  | CHARLIT			{$$ = $1;}
	  | STRINGLIT			{$$ = $1;}
	  | REPEAT			{$$ = $1;}
	  | '(' exp ')'			{$$ = gcShadow(3,$2);}
	  | '(' exps2 ')'		{$$ = gcShadow(3,buildTuple($2));}
	  | '[' list ']'		{$$ = gcShadow(3,$2);}
	  | '(' exp10a qop ')'		{$$ = gcShadow(4,ap($3,$2));}
	  | '(' qvarop_mi exp0 ')'	{$$ = gcShadow(4,ap(ap(nameFlip,$2),$3));}
	  | '(' qconop exp0 ')'		{$$ = gcShadow(4,ap(ap(nameFlip,$2),$3));}
	  ;
exps2	  : exps2 ',' exp		{$$ = gcShadow(3,cons($3,$1));}
	  | exp ',' exp			{$$ = gcShadow(3,cons($3,cons($1,NIL)));}
	  ;
alts      : alts1                       {$$ = $1;}
          | ';' alts                    {$$ = gcShadow(2,$2);}
          ;
alts1     : alts1 ';' alt               {$$ = gcShadow(3,cons($3,$1));}
          | alts1 ';'                   {$$ = gcShadow(2,$1);}
          | alt                         {$$ = gcShadow(1,cons($1,NIL));}
	  ;
alt	  : pat altRhs wherePart	{$$ = gcShadow(3,pair($1,letrec($3,$2)));}
	  ;
altRhs	  : guardAlts			{$$ = gcShadow(1,desugarGuards(rev($1)));}
	  | ARROW exp			{$$ = gcShadow(2,pair($1,$2));}
	  | error			{syntaxError("case expression");}
	  ;
guardAlts : guardAlts guardAlt		{$$ = gcShadow(2,cons($2,$1));}
	  | guardAlt			{$$ = gcShadow(1,cons($1,NIL));}
	  ;
guardAlt  : '|' quals ARROW exp		{$$ = gcShadow(4,pair($3,pair(rev($2),$4)));}
	  ;

stmts     : stmts1                      {$$ = $1;}
          | ';' stmts                   {$$ = gcShadow(2,$2);}
	  ;
stmts1	  : stmts1 ';' stmt		{$$ = gcShadow(3,cons($3,$1));}
          | stmts1 ';'			{$$ = gcShadow(2,$1);}
	  | stmt			{$$ = gcShadow(1,cons($1,NIL));}
	  ;

stmt      : exp_err FROM exp		{$$ = gcShadow(3,ap(FROMQUAL,pair($1,$3)));}
	  | LET decls			{$$ = gcShadow(2,ap(QWHERE,$2));}
/*	  | IF exp			{$$ = gcShadow(2,ap(BOOLQUAL,$2));}*/
	  | exp_err			{$$ = gcShadow(1,ap(DOQUAL,$1));}
	  ;
fbinds	  : /* empty */			{$$ = gcShadow(0,NIL);}
	  | fbinds1			{$$ = gcShadow(1,rev($1));}
	  ;
fbinds1	  : fbinds1 ',' fbind		{$$ = gcShadow(3,cons($3,$1));}
	  | fbind			{$$ = gcShadow(1,singleton($1));}
	  ;
fbind	  : var				{$$ = $1;}
	  | qvar '=' exp		{$$ = gcShadow(3,pair($1,$3));}
	  ;

/*- List Expressions: -------------------------------------------------------*/

list	  : exp				{$$ = gcShadow(1,ap(FINLIST,cons($1,NIL)));}
	  | exps2			{$$ = gcShadow(1,ap(FINLIST,rev($1)));}
	  | exp '|' quals		{$$ = gcShadow(3,ap(COMP,pair($1,rev($3))));}
	  | exp         UPTO exp	{$$ = gcShadow(3,ap(ap(nameFromTo,$1),$3));}
	  | exp ',' exp UPTO		{$$ = gcShadow(4,ap(ap(nameFromThen,$1),$3));}
	  | exp         UPTO		{$$ = gcShadow(2,ap(nameFrom,$1));}
	  | exp ',' exp UPTO exp	{$$ = gcShadow(5,ap(ap(ap(nameFromThenTo,
								$1),$3),$5));}
	  ;
quals	  : quals ',' qual		{$$ = gcShadow(3,cons($3,$1));}
	  | qual			{$$ = gcShadow(1,cons($1,NIL));}
	  ;
qual	  : exp FROM exp		{$$ = gcShadow(3,ap(FROMQUAL,pair($1,$3)));}
	  | exp				{$$ = gcShadow(1,ap(BOOLQUAL,$1));}
	  | LET decls			{$$ = gcShadow(2,ap(QWHERE,$2));}
	  ;

/*- Identifiers and symbols: ----------------------------------------------*/

gcon	  : qcon			{$$ = $1;}
	  | '(' ')'			{$$ = gcShadow(2,nameUnit);}
	  | '[' ']'			{$$ = gcShadow(2,nameNil);}
	  | '(' tupCommas ')'		{$$ = gcShadow(3,$2);}
	  ;
tupCommas : tupCommas ','		{$$ = gcShadow(2,mkTuple(tupleOf($1)+1));}
	  | ','				{$$ = gcShadow(1,mkTuple(2));}
	  ;
varid	  : VARID			{$$ = $1;}
	  | HIDING			{$$ = gcShadow(1,varHiding);}
	  | QUALIFIED			{$$ = gcShadow(1,varQualified);}
	  | ASMOD			{$$ = gcShadow(1,varAsMod);}
	  ;
qconid	  : QCONID			{$$ = $1;}
	  | CONID			{$$ = $1;}
	  ;
var	  : varid			{$$ = $1;}
	  | '(' VAROP ')'		{$$ = gcShadow(3,$2);}
	  | '(' '+' ')'			{$$ = gcShadow(3,varPlus);}
	  | '(' '-' ')'			{$$ = gcShadow(3,varMinus);}
	  | '(' '!' ')'			{$$ = gcShadow(3,varBang);}
	  | '(' '.' ')'			{$$ = gcShadow(3,varDot);}
	  ;
qvar	  : QVARID			{$$ = $1;}
	  | '(' QVAROP ')'		{$$ = gcShadow(3,$2);}
	  | var				{$$ = $1;}
	  ;
con	  : CONID			{$$ = $1;}
	  | '(' CONOP ')'		{$$ = gcShadow(3,$2);}
	  ;
qcon	  : QCONID			{$$ = $1;}
	  | '(' QCONOP ')'		{$$ = gcShadow(3,$2);}
	  | con				{$$ = $1;}
	  ;
varop	  : '+'				{$$ = gcShadow(1,varPlus);}
	  | '-'				{$$ = gcShadow(1,varMinus);}
	  | varop_mipl			{$$ = $1;}
	  ;
varop_mi  : '+'				{$$ = gcShadow(1,varPlus);}
	  | varop_mipl			{$$ = $1;}
	  ;
varop_pl  : '-'				{$$ = gcShadow(1,varMinus);}
	  | varop_mipl			{$$ = $1;}
	  ;
varop_mipl: VAROP			{$$ = $1;}
	  | '`' varid '`'		{$$ = gcShadow(3,$2);}
	  | '!'				{$$ = gcShadow(1,varBang);}
	  | '.'				{$$ = gcShadow(1,varDot);}
	  ;
qvarop	  : '-'				{$$ = gcShadow(1,varMinus);}
	  | qvarop_mi			{$$ = $1;}
	  ;
qvarop_mi : QVAROP			{$$ = $1;}
	  | '`' QVARID '`'		{$$ = gcShadow(3,$2);}
	  | varop_mi			{$$ = $1;}
	  ;

conop	  : CONOP 			{$$ = $1;}
	  | '`' CONID  '`'		{$$ = gcShadow(3,$2);}
	  ;
qconop	  : QCONOP			{$$ = $1;}
	  | '`' QCONID '`'		{$$ = gcShadow(3,$2);}
	  | conop			{$$ = $1;}
	  ;
op	  : varop			{$$ = $1;}
	  | conop			{$$ = $1;}
	  ;
qop	  : qvarop			{$$ = $1;}
	  | qconop			{$$ = $1;}
	  ;

/*- Tricks to force insertion of leading and closing braces ---------------*/

begin	  : /* empty */			{goOffside(startColumn);}
	  ;
					/* deal with trailing semicolon    */
end	  : '}'				{$$ = $1;}
	  | error			{yyerrok; 
					 if (canUnOffside()) {
					     unOffside();
					     /* insert extra token on stack*/
					     push(NIL);
					     pushed(0) = pushed(1);
					     pushed(1) = mkInt(column);
					 }
					 else
					     syntaxError("declaration");
					}
	  ;

/*-------------------------------------------------------------------------*/

%%

/* Keep parsed fragments on stack */
static Cell gcShadow(int n, Cell e) {
    /* If a look ahead token is held then the required stack transformation
     * is:
     *   pushed: n               1     0          1     0
     *           x1  |  ...  |  xn  |  la   ===>  e  |  la
     *                                top()            top()
     *
     * Othwerwise, the transformation is:
     *   pushed: n-1             0        0
     *           x1  |  ...  |  xn  ===>  e
     *                         top()     top()
     */
    if (yychar>=0) {
	pushed(n-1) = top();
	pushed(n)   = e;
    }
    else
	pushed(n-1) = e;
    sp -= (n-1);
    return e;
}

/* Report on syntax error */
static void syntaxError(char *s) {
    errHead(row); errPrintf("Syntax error in %s (unexpected %s)", s, unexpected());
    errFail();
}

/* Find name for unexpected token */
static char *unexpected(void) {
    static char buffer[100];
    static char *fmt = "%s \"%s\"";
    static char *kwd = "keyword";

    switch (yychar) {
	case 0         : return "end of input";

#define keyword(kw) snprintf(buffer,sizeof(buffer),fmt,kwd,kw); return buffer;
	case INFIXL    : keyword("infixl");
	case INFIXR    : keyword("infixr");
	case INFIXN    : keyword("infix");
	case TINSTANCE : keyword("instance");
	case TCLASS    : keyword("class");
	case PRIMITIVE : keyword("primitive");
	case CASEXP    : keyword("case");
	case OF        : keyword("of");
	case IF        : keyword("if");
	case THEN      : keyword("then");
	case ELSE      : keyword("else");
	case WHERE     : keyword("where");
	case TYPE      : keyword("type");
	case DATA      : keyword("data");
	case TNEWTYPE  : keyword("newtype");
	case LET       : keyword("let");
	case IN        : keyword("in");
	case DERIVING  : keyword("deriving");
	case DEFAULT   : keyword("default");
	case IMPORT    : keyword("import");
	case TMODULE   : keyword("module");
	case ALL       : keyword("forall");
#undef keyword

	case ARROW     : return "`->'";
	case '='       : return "`='";
	case COCO      : return "`::'";
	case '-'       : return "`-'";
	case '!'       : return "`!'";
	case ','       : return "comma";
	case '@'       : return "`@'";
	case '('       : return "`('";
	case ')'       : return "`)'";
	case '{'       : return "`{', possibly due to bad layout";
	case '}'       : return "`}', possibly due to bad layout";
	case '_'       : return "`_'";
	case '|'       : return "`|'";
	case '.'       : return "`.'";
	case ';'       : return "`;', possibly due to bad layout";
	case UPTO      : return "`..'";
	case '['       : return "`['";
	case ']'       : return "`]'";
	case FROM      : return "`<-'";
	case '\\'      : return "backslash (lambda)";
	case '~'       : return "tilde";
	case '`'       : return "backquote";
	case VAROP     :
	case VARID     :
	case CONOP     :
	case CONID     : snprintf(buffer,sizeof(buffer),"symbol \"%s\"",
				 textToStr(textOf(yylval)));
			 return buffer;
	case QVAROP    :
	case QVARID    :
	case QCONOP    :
	case QCONID    : snprintf(buffer,sizeof(buffer),"symbol \"%s\"",
				 identToStr(yylval));
			 return buffer;
	case HIDING    : return "symbol \"hiding\"";
	case QUALIFIED : return "symbol \"qualified\"";
	case ASMOD     : return "symbol \"as\"";
	case NUMLIT    : return "numeric literal";
	case CHARLIT   : return "character literal";
	case STRINGLIT : return "string literal";
	case IMPLIES   : return "`=>'";
	default        : return "token";
    }
}

/* Check for valid precedence value */
static Cell checkPrec(Cell p) {
    if (!isInt(p) || intOf(p)<MIN_PREC || intOf(p)>MAX_PREC) {
	errHead(row); errPrintf("Precedence value must be an integer in the range [%d..%d]",
		    MIN_PREC, MAX_PREC);
	errFail();
    }
    return p;
}

/* --------------------------------------------------------------------------
 * Pattern guards (Haskell 2010).  A guard is parsed as a list of qualifiers
 * (line,(quals,rhs)); a plain boolean guard is left as (line,(cond,rhs)).
 * A guard with anything else is desugared here, so that no later stage of
 * the system has to know about pattern guards:
 *
 *      | q1, ..., qn = e
 *
 * becomes the boolean guard
 *
 *      | case m of { Just _ -> True; Nothing -> False }
 *            = case m of { Just r -> r }
 *
 * together with a hidden binding, placed in a let around the guarded rhs,
 *
 *      m = Q(q1, ..., qn)   where   Q()           = Just e
 *                                   Q(p <- x, qs) = case x of { p -> Q(qs); _ -> Nothing }
 *                                   Q(b, qs)      = if b then Q(qs) else Nothing
 *                                   Q(let ds, qs) = let ds in Q(qs)
 *
 * Evaluating the guard performs the matches and tests in order; the rhs
 * stays a thunk inside the Just, with every variable the qualifiers bind
 * in scope.  If all guards fail the guarded rhs fails as before, so the
 * next equation is tried.
 * ------------------------------------------------------------------------*/

/* Build Q above */
static Cell guardMatch(Cell l, List qs, Cell res) {
    Cell q;
    if (isNull(qs))
	return res;
    q   = hd(qs);
    res = guardMatch(l,tl(qs),res);
    switch (whatIs(q)) {
	case BOOLQUAL : return ap(COND,triple(snd(q),res,nameNothing));
	case QWHERE   : return ap(LETREC,pair(snd(q),res));
	case FROMQUAL : return ap(CASE,pair(snd(snd(q)),
			       cons(pair(fst(snd(q)),pair(l,res)),
			       cons(pair(WILDCARD,pair(l,nameNothing)),NIL))));
	default       : internal("guardMatch");
    }
    return NIL;/*NOTREACHED*/
}

/* [(line,(quals,rhs))] -> rhs */
static Cell desugarGuards(List gs) {
    List binds = NIL;
    List g;
    for (g=gs; nonNull(g); g=tl(g)) {
	Cell l   = fst(hd(g));
	List qs  = fst(snd(hd(g)));
	Cell e   = snd(snd(hd(g)));
	if (nonNull(qs) && isNull(tl(qs)) && whatIs(hd(qs))==BOOLQUAL) {
	    fst(snd(hd(g))) = snd(hd(qs));	/* ordinary boolean guard */
	} else {
	    Cell m  = inventVar();
	    Cell r  = inventVar();
	    Cell mv = mkVar(textOf(m));
	    Cell rv = mkVar(textOf(r));
	    binds   = cons(ap(PATBIND,pair(m,pair(l,guardMatch(l,qs,ap(nameJust,e))))),
			   binds);
	    fst(snd(hd(g)))
		    = ap(CASE,pair(mv,
			 cons(pair(ap(nameJust,WILDCARD),pair(l,nameTrue)),
			 cons(pair(nameNothing,pair(l,nameFalse)),NIL))));
	    snd(snd(hd(g)))
		    = ap(CASE,pair(mkVar(textOf(m)),
			 cons(pair(ap(nameJust,r),pair(l,rv)),NIL)));
	}
    }
    return letrec(binds,grded(gs));
}

/* Build tuple (x1,...,xn) from list [xn,...,x1] */
static Cell buildTuple(List tup) {
    int  n = 0;
    Cell t = tup;
    Cell x;

    do {				/*    .                    .	   */
	x      = fst(t);		/*   / \                  / \	   */
	fst(t) = snd(t);		/*  xn  .                .   xn	   */
	snd(t) = x;			/*       .    ===>      .	   */
	x      = t;			/*        .            .	   */
	t      = fun(x);		/*         .          .		   */
	n++;				/*        / \        / \	   */
    } while (nonNull(t));		/*       x1  NIL   (n)  x1	   */
    fst(x) = mkTuple(n);
    return tup;
}

/* Validate context */
static List checkCtxt(Type con) {
    mapOver(checkPred, con);
    return con;
}

/* Check that type expr is a valid constraint: a class applied to types, or
 * an equality */
static Cell checkPred(Cell c) {
    Cell cn = getHead(c);
    if (!isQCon(cn) && !isEquality(c))
	syntaxError("class expression");
    return c;
}

/* Convert reversed list of dquals to an (expr,quals) pair */
static Pair checkDo(List dqs) {
    if (isNull(dqs) || whatIs(hd(dqs))!=DOQUAL) {
	errHead(row); errPrintf("Last generator in do {...} must be an expression");
	errFail();
    }
    fst(dqs) = snd(fst(dqs));		/* put expression in fst of pair   */
    snd(dqs) = rev(snd(dqs));		/* & reversed list of quals in snd */
    return dqs;
}


/* Check that lhs is of the form T a1 ... a, with an ai possibly (a :: k) */
static Cell checkTyLhs(Cell c) {
    Cell tlhs = c;
    while (isAp(tlhs) && (whatIs(arg(tlhs))==VARIDCELL || whatIs(arg(tlhs))==KINDED)) {
	tlhs = fun(tlhs);
    }
    if (whatIs(tlhs)!=CONIDCELL) {
	errHead(row); errPrintf("Illegal left hand side in data type declaration");
	errFail();
    }
    return c;
}

/* Check that data constructor has an unqualified conid as head */
static Cell checkConstr(Cell c) {
    Cell chd = c;
    while (isAp(chd)) {
	chd = fun(chd);
    }
    if (whatIs(chd)==QUALIDENT) {
	errHead(row); errPrintf("Qualified constructor in data type declaration");
	errFail();
    }
    return c;
}



/*-------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
 * Building declarations and types:
 * ------------------------------------------------------------------------*/

/* Define tycon, popping n items */
/* The kind written as an operator: only * */
static Cell starKind(Cell op) {
    if (strcmp(textToStr(textOf(op)),"*")!=0) {
	errHead(row); errPrintf("Syntax error in kind (unexpected symbol \"%s\")",
				textToStr(textOf(op)));
	errFail();
    }
    return STAR;
}

/* The kind written as a name: Type is *, and Constraint is what a class has */
static Cell namedKind(Cell con) {
    char *s = textToStr(textOf(con));
    if (strcmp(s,"Type")==0) {
	return STAR;
    }
    if (strcmp(s,"Constraint")==0) {
	return CONSTRAINT;
    }
    errHead(row); errPrintf("Syntax error in kind (unexpected constructor \"%s\")",s);
    errFail();
}

static void defTycon(int n, Cell l, Cell lhs, Cell rhs, Cell w) {
    tyconDefn(intOf(l),lhs,rhs,w);
    sp -= n;
}

static Cell sigdecl(Cell l, List vs, Type t) {
    return ap(SIGDECL,triple(l,vs,t));
}

static Cell fixdecl(Cell l, List ops, Syntax a, Cell p) {
    return ap(FIXDECL,triple(l,ops,mkInt(mkSyntax(a,intOf(p)))));
}

static Cell grded(List gs)   { return ap(GUARDED,gs); }
static Cell bang (Type t)    { return ap(BANG,t); }
static Cell only (Cell t)    { return ap(ONLY,t); }

/* Wrap e in bs, if any */
static Cell letrec(List bs, Cell e) {
    return nonNull(bs) ? ap(LETREC,pair(bs,e)) : e;
}

/* Qualify t by ps, if any */
static Cell qualType(List ps, Type t) {
    return nonNull(ps) ? ap(QUAL,pair(ps,t)) : t;
}

/* The module's own name */
static List exportSelf(void) {
    return singleton(ap(MODULEENT,mkCon(moduleRec(currentModule)->text)));
}
