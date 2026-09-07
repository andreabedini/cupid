/* --------------------------------------------------------------------------
 * Static Analysis for Cuddles
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: static.c,v $
 * $Revision: 1.180 $
 * $Date: 2006/08/30 18:46:22 $
 * ------------------------------------------------------------------------*/

#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "scc.h"
#include "tagcheck.h"
#include "machdep.h"
#include "errors.h"
#include "output.h"
#include "subst.h"
#include "module.h"
#include "opts.h"
#include "goal.h"

/* --------------------------------------------------------------------------
 * local function prototypes:
 * ------------------------------------------------------------------------*/

static Void   local kindError		(Int,Constr,Constr,String,Kind,Int);
static Void   local checkQualImport	(Pair);
static Void   local checkUnqualImport	(Triple);

static Void   local checkTyconDefn	(Tycon);
static Void   local depConstrs		(Tycon,List,Cell);
static Int    local userArity		(Name);
static List   local addSels		(Int,Name,List,List);
static List   local selectCtxt		(List,List);
static Void   local checkSynonyms	(List);
static List   local visitSyn		(List,Tycon,List);
static Type   local fullerExpand	(Type);
static Type   local instantiateNewtype	(Name,Type);
static Type   local instantiateSyn	(Type,Type);

static Void   local checkClassDefn	(Class);
static Void   local checkClassDefn2	(Class);
static Void local checkClassDefn2_      (List);
static Cell   local depPredExp		(Int,List,Cell);
static Void   local checkMems		(Class,List,Cell);
static Void   local checkMems2		(Class,Cell);
static Void   local addMembers		(Class);
static Name   local newMember		(Int,Int,Cell,Type,Class);
static Name   local newDSel		(Class,Int);
static Text   local generateText	(String,Class);
static Int    local visitClass		(Class);

static List   local classBindings	(String,Class,List);
static Name   local memberName		(Class,Text);
static List   local numInsert		(Int,Cell,List);

static List   local typeVarsIn		(Cell,List,List,List);
static List   local maybeAppendVar	(Cell,List);

static Type   local checkSigType	(Int,String,Cell,Type);
static Void   local checkOptQuantVars	(Int,List,List);
static Type   local depTopType		(Int,List,Type);
static Type   local depCompType		(Int,List,Type);
static Type   local depTypeExp		(Int,List,Type);
static Type   local depTypeVar		(Int,List,Text);
static List   local checkQuantVars	(Int,List,List,Cell);
static List   local offsetTyvarsIn	(Type,List);
static List   local otvars		(Cell,List);
static Bool   local osubset		(List,List);
static Void   local kindConstr		(Int,Int,Int,Constr);
static Kind   local kindAtom		(Int,Constr);
static Void   local kindPred		(Int,Int,Int,Cell);
static Void   local kindType		(Int,String,Type);
static Void   local fixKinds		(Void);

static Void   local kindTCGroup		(List);
static Void   local initTCKind		(Cell);
static Void   local kindTC		(Cell);
static Void   local genTC		(Cell);

static Void   local checkInstDefn	(Inst);
static Void   local insertInst		(Inst);
static Bool   local instCompare		(Inst,Inst);
static Name   local newInstImp		(Inst);
static Void   local kindInst		(Inst,Int);
static Void   local checkDerive		(Tycon,List,List,Cell);
static Void   local addDerInst		(Int,Class,List,List,Type,Int);
static Void   local deriveContexts	(List);
static Void   local initDerInst		(Inst);
static Void   local calcInstPreds	(Inst);
static List   local calcFunDeps		(List);
static Void   local maybeAddPred	(Cell,Int,Int,List);
static Cell   local copyAdj		(Cell,Int,Int);
static Void   local tidyDerInst		(Inst);
static List   local inheritFundeps      (Class,Cell,Int);
static Void   local extendFundeps       (Class);

static List   local otvarsZonk		(Cell,List,Int);
static Void   local addDerivImp		(Inst);
static List   local getDiVars		(Int);
static Cell   local mkBind		(String,List);
static Cell   local mkVarAlts		(Int,Cell);

static List   local deriveEq		(Tycon);
static Pair   local mkAltEq		(Int,List);
static List   local deriveOrd		(Tycon);
static Pair   local mkAltOrd		(Int,List);
static List   local makeDPats2		(Cell,Int);

static List   local deriveEnum		(Tycon);
static List   local deriveIx		(Tycon);
static Bool   local isEnumType		(Tycon);
static List   local mkIxBinds		(Int,Cell,Int);
static Cell   local prodRange		(Int,List,Cell,Cell,Cell);
static Cell   local prodIndex		(Int,List,Cell,Cell,Cell);
static Cell   local prodInRange		(Int,List,Cell,Cell,Cell);

static List   local deriveShow		(Tycon);
static Cell   local mkAltShow		(Int,Cell,Int);
static Cell   local showsPrecRhs	(Cell,Cell,Int);

static List   local deriveRead		(Cell);
static Cell   local mkReadCon		(Name,Cell,Cell);
static Cell   local mkReadPrefix	(Cell);
static Cell   local mkReadInfix		(Cell);
static Cell   local mkReadTuple		(Cell);
static Cell   local mkReadRecord	(Cell,List);

static List   local deriveBounded	(Tycon);
static List   local mkBndBinds		(Int,Cell,Int);

static Void   local checkDefaultDefns	(Void);

static List   local checkPrimDefn	(Triple);
static Name   local addNewPrim		(Int,Text,String,Cell);

static Void   local checkForeignImport  (Name);
static Void   local checkForeignExport  (Name);
static Void   local linkForeign         (Name);

static Cell   local tidyInfix		(Int,Cell);
static Pair   local attachFixity	(Int,Cell);
static Syntax local lookupSyntax	(Text);

static Cell   local checkPat		(Int,Cell);
static Cell   local checkMaybeCnkPat	(Int,Cell);
static Cell   local checkApPat		(Int,Int,Cell);
static Void   local addToPatVars	(Int,Cell);
static Name   local conDefined		(Int,Cell,Bool);
static Void   local checkIsCfun		(Int,Name);
static Void   local checkCfunArgs	(Int,Cell,Int);
static Cell   local checkPatType	(Int,String,Cell,Type);
static Cell   local applyBtyvs		(Cell);
static Cell   local bindPat		(Int,Cell);
static Void   local bindPats		(Int,List);

static List   local extractSigdecls	(List);
static List   local extractFixdecls	(List);
static List   local extractBindings	(List);
static List   local getPatVars		(Int,Cell,List);
static List   local addPatVar		(Int,Cell,List);
static List   local eqnsToBindings	(List,List,List,List);
static Void   local notDefined		(Int,List,Cell);
static Cell   local findBinding		(Text,List);
static Cell   local getAttr		(List,Cell);
static Void   local addSigdecl		(List,Cell);
static Void   local addFixdecl		(List,List,List,List,Triple);
static Void   local dupFixity		(Int,Text);
static Void   local missFixity		(Int,Text);

static List   local dependencyAnal	(List);
static List   local topDependAnal	(List);
static Void   local addDepField		(Cell);
static Void   local remDepField		(List);
static Void   local remDepField1	(Cell);
static Void   local clearScope		(Void);
static Void   local withinScope		(List);
static Void   local leaveScope		(Void);
static Void   local saveSyntax		(Cell,Cell);
static Void   local dropNameClash	(Cell);

static Void   local depBinding		(Cell);
static Void   local depDefaults		(Class);
static Void   local depInsts		(Inst);
static Void   local depClassBindings	(List);
static Void   local depAlt		(Cell);
static Cell   local depLetRec		(Bool,Int,Cell);
static Void   local depRhs		(Cell);
static Void   local depGuard		(Cell);
static Void   local depPair		(Int,Cell);
static Void   local depTriple		(Int,Cell);
static Void   local depComp		(Int,Cell,List);
static Void   local depCaseAlt		(Int,Cell);
static Cell   local depVar		(Int,Cell,Bool);
static Cell   local depQVar		(Int,Cell,Bool);
static Void   local depConFlds		(Int,Cell,Bool);
static Void   local depUpdFlds		(Int,Cell);
static List   local depFields		(Int,Cell,List,Bool);
static Void   local checkNameAmbigName  (Int,Name,Bool);
static Void   local checkNameAmbig      (Int,Text,Cell);
static Cell   local checkTyconAmbig     (Int,Text,Cell);


static List   local tcscc		(List,List);
static List   local bscc		(List);

static Void   local addRSsigdecls	(Pair);
static Void   local allNoPrevDef	(Cell);
static Void   local noPrevDef		(Int,Cell);
static List   local odiff		(List,List);

static Void   local duplicateError	(Int,Module,Text,String);
static Void   local checkTypeIn		(Pair);


/* --------------------------------------------------------------------------
 * The code in this file is arranged in roughly the following order:
 *  - Kind inference preliminaries
 *  - Module declarations
 *  - Type declarations (data, type, newtype, type in)
 *  - Class declarations
 *  - Type signatures
 *  - Instance declarations
 *  - Default declarations
 *  - Primitive definitions
 *  - Foreign Function Interface declarations
 *  - Patterns
 *  - Infix expressions
 *  - Value definitions
 *  - Top-level static analysis and control
 * ------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
 * Kind checking preliminaries:
 * ------------------------------------------------------------------------*/

Bool kindExpert = FALSE;		/* TRUE => display kind errors in  */
					/*	   full detail		   */

static Void local kindError(Int l, Constr c, Constr in, String wh, Kind k, Int o)
             /* line number near constuctor exp */
             /* constructor			   */
              /* context (if any)		   */
              /* place in which error occurs	   */
             /* expected kind (k,o)		   */
{				/* inferred kind (typeIs,typeOff)  */
    clearMarks();

    if (!kindExpert) {			/* for those with a fear of kinds  */
	ERRMSG(l) "Illegal type" ETHEN
	if (nonNull(in)) {
	    ERRTEXT " \"" ETHEN ERRTYPE(in);
	    ERRTEXT "\""  ETHEN
	}
	ERRTEXT " in %s\n", wh
	EEND;
    }

    ERRMSG(l) "Kind error in %s", wh ETHEN
    if (nonNull(in)) {
	ERRTEXT "\n*** expression     : " ETHEN ERRTYPE(in);
    }
    ERRTEXT "\n*** constructor    : " ETHEN ERRTYPE(c);
    ERRTEXT "\n*** kind           : " ETHEN ERRKIND(copyType(typeIs,typeOff));
    ERRTEXT "\n*** does not match : " ETHEN ERRKIND(copyType(k,o));
    if (unifyFails) {
	ERRTEXT "\n*** because        : %s", unifyFails ETHEN
    }
    ERRTEXT "\n"
    EEND;
}

#define shouldKind(l,c,in,wh,k,o)	if (!kunify(typeIs,typeOff,k,o)) \
					    kindError(l,c,in,wh,k,o)
#define checkKind(l,a,m,c,in,wh,k,o)	kindConstr(l,a,m,c); \
					shouldKind(l,c,in,wh,k,o)
#define inferKind(k,o)			typeIs=k; typeOff=o

static List unkindTypes;		/* types in need of kind annotation*/

/* --------------------------------------------------------------------------
 * Static analysis of modules:
 * ------------------------------------------------------------------------*/

Void startModule(Cell nm)                             /* switch to a new module */
{
    Module m;
    Text t = textOf(nm);
    if (!isCon(nm)) internal("startModule");
    if (isNull(m = findModule(t))) {
	m = newModule(t);
	if ( moduleUserPrelude == 0 && t == textUserPrelude ) {
	  moduleUserPrelude = m;
	}
    } else if (!isPreludeScript()) {
	/* You're allowed to break the rules in the Prelude! */
	ERRMSG(0) "Module \"%s\" already loaded", textToStr(t)
	EEND;
    }
    setCurrModule(m);
}

Void setExportList(List exps)              /* Add export list to current module */
{
    module(currentModule).exports = exps;
}

static Void local checkQualImport(Pair i)   /* Process qualified import         */
{
    Module m = findModid(snd(i));
    if (isNull(m)) {
	ERRMSG(0) "Module \"%s\" not previously loaded",
		  textToStr(textOf(snd(i)))
	EEND;
    }
    snd(i)=m;
}

static Void local checkUnqualImport(Pair i) /* Process unqualified import       */
{
    Module m = findModid(fst(i));
    if (isNull(m)) {
	ERRMSG(0) "Module \"%s\" not previously loaded",
		  textToStr(textOf(fst(i)))
	EEND;
    }
    fst(i)=m;
}

/* the bulk of the module system implementation now resides in module.c */


/* --------------------------------------------------------------------------
 * Static analysis of type declarations:
 *
 * Type declarations come in two forms:
 * - data declarations - define new constructed data types
 * - type declarations - define new type synonyms
 *
 * A certain amount of work is carried out as the declarations are
 * read during parsing.  In particular, for each type constructor
 * definition encountered:
 * - check that there is no previous definition of constructor
 * - ensure type constructor not previously used as a class name
 * - make a new entry in the type constructor table
 * - record line number of declaration
 * - Build separate lists of newly defined constructors for later use.
 * ------------------------------------------------------------------------*/

Void tyconDefn(Int line, Cell lhs, Cell rhs, Cell what)	/* process new type definition	   */
              /* definition line number	   */
             /* left hand side of definition	   */
             /* right hand side of definition   */
{				/* SYNONYM/DATATYPE/etc...	   */
    Text t = textOf(getHead(lhs));
    Tycon tc = findTycon(t);

    if ( nonNull(tc) ) {
	ERRMSG(line) "Multiple declarations of type constructor \"%s\"",
		     textToStr(t)
	EEND;
    } else if (nonNull(tc) && nonNull(tycon(tc).clashes)) {
        List ls = tycon(tc).clashes;
	ERRMSG(line) "Ambiguous type constructor occurrence \"%s\"", textToStr(t) ETHEN
        ERRTEXT "\n*** Could refer to: " ETHEN
       ERRTEXT "%s.%s ", textToStr(module(tycon(tc).mod).text), textToStr(tycon(tc).text) ETHEN
	for(;nonNull(ls);ls=tl(ls)) {
	  ERRTEXT "%s.%s ", textToStr(module(tycon(hd(ls)).mod).text), textToStr(tycon(hd(ls)).text)
	  ETHEN
	}
	ERRTEXT "\n" EEND;
    } else if (nonNull(findClass(t))) {
	ERRMSG(line) "\"%s\" used as both class and type constructor",
		     textToStr(t)
	EEND;
    }
    else {
	Tycon nw	= newTycon(t);
	tyconDefns      = cons(nw,tyconDefns);
	tycon(nw).line  = line;
	tycon(nw).arity = argCount;
	tycon(nw).what  = what;
	if (what==RESTRICTSYN) {
	    typeInDefns = cons(pair(nw,snd(rhs)),typeInDefns);
	    rhs         = fst(rhs);
	}
	tycon(nw).defn  = pair(lhs,rhs);
    }
}

Void setTypeIns(List bs)			/* set local synonyms for given	   */
{				/* binding group		   */
    List cvs = typeInDefns;
    for (; nonNull(cvs); cvs=tl(cvs)) {
	Tycon c  = fst(hd(cvs));
	List  vs = snd(hd(cvs));
	for (tycon(c).what = RESTRICTSYN; nonNull(vs); vs=tl(vs)) {
	    if (nonNull(findBinding(textOf(hd(vs)),bs))) {
		tycon(c).what = SYNONYM;
		break;
	    }
	}
    }
}

Void clearTypeIns() {			/* clear list of local synonyms	   */
    for (; nonNull(typeInDefns); typeInDefns=tl(typeInDefns))
	tycon(fst(hd(typeInDefns))).what = RESTRICTSYN;
}

/* --------------------------------------------------------------------------
 * Further analysis of Type declarations:
 *
 * In order to allow the definition of mutually recursive families of
 * data types, the static analysis of the right hand sides of type
 * declarations cannot be performed until all of the type declarations
 * have been read.
 *
 * Once parsing is complete, we carry out the following:
 *
 * - check format of lhs, extracting list of bound vars and ensuring that
 *   there are no repeated variables and no Skolem variables.
 * - run dependency analysis on rhs to check that only bound type vars
 *   appear in type and that all constructors are defined.
 *   Replace type variables by offsets, constructors by Tycons.
 * - use list of dependents to sort into strongly connected components.
 * - ensure that there is not more than one synonym in each group.
 * - kind-check each group of type definitions.
 *
 * - check that there are no previous definitions for constructor
 *   functions in data type definitions.
 * - install synonym expansions and constructor definitions.
 * ------------------------------------------------------------------------*/

static List tcDeps = NIL;		/* list of dependent tycons/classes*/

static Void local checkTyconDefn(Tycon d)	/* validate type constructor defn  */
{
    Cell lhs    = fst(tycon(d).defn);
    Cell rhs    = snd(tycon(d).defn);
    Int  line   = tycon(d).line;
    List tyvars = getArgs(lhs);
    List temp;
					/* check for repeated tyvars on lhs*/
    for (temp=tyvars; nonNull(temp); temp=tl(temp))
	if (nonNull(varIsMember(textOf(hd(temp)),tl(temp)))) {
	    ERRMSG(line) "Repeated type variable \"%s\" on left hand side",
			 textToStr(textOf(hd(temp)))
	    EEND;
	}

    tcDeps = NIL;			/* find dependents		   */
    switch (whatIs(tycon(d).what)) {
	case RESTRICTSYN :
	case SYNONYM	 : rhs = depTopType(line,tyvars,rhs);
			   if (cellIsMember(d,tcDeps)) {
			       ERRMSG(line) "Recursive type synonym \"%s\"",
					    textToStr(tycon(d).text)
			       EEND;
			   }
			   break;

	case DATATYPE	 :
	case NEWTYPE	 : depConstrs(d,tyvars,rhs);
			   rhs = fst(rhs);
			   break;

	default		 : internal("checkTyconDefn");
			   break;
    }

    tycon(d).defn = rhs;
    tycon(d).kind = tcDeps;
    tcDeps	  = NIL;
}

static Void local depConstrs(Tycon t, List tyvars, Cell cd)
            /* Define constructor functions and*/
                 /* do dependency analysis for data */
{				/* definitions (w or w/o deriving) */
    Int  line      = tycon(t).line;
    List ctxt      = NIL;
    Int  conNo     = 1;
    Type lhs	   = t;
    List cs	   = fst(cd);
    List derivs    = snd(cd);
    List compTypes = NIL;
    List sels	   = NIL;
    Int  i;

    for (i=0; i<tycon(t).arity; ++i)	/* build representation for tycon  */
	lhs = ap(lhs,mkOffset(i));	/* applied to full comp. of args   */

    if (isQualType(cs)) {		/* allow for possible context	   */
	ctxt = fst(snd(cs));
	cs   = snd(snd(cs));
	map2Over(depPredExp,line,tyvars,ctxt);
    }

    if (nonNull(cs) && isNull(tl(cs)))	/* Single constructor datatype?	   */
	conNo = 0;

    for (; nonNull(cs); cs=tl(cs)) {	/* For each constructor function:  */
	Cell con   = hd(cs);
	List sig   = dupList(tyvars);
	List evs   = NIL;		/* locally quantified vars	   */
	List lps   = NIL;		/* locally bound predicates	   */
	List ctxt1 = ctxt;		/* constructor function context	   */
	List scs   = NIL;		/* strict components		   */
	List fs    = NONE;		/* selector names		   */
	Type type  = lhs;		/* constructor function type	   */
	Int  arity = 0;			/* arity of constructor function   */
	Int  nr2   = 0;			/* Number of rank 2 args	   */
	Name n;				/* name for constructor function   */

	if (whatIs(con)==POLYTYPE) {	/* Locally quantified vars	   */
	    evs = fst(snd(con));
	    con = snd(snd(con));
	    sig = checkQuantVars(line,evs,sig,con);
	}

	if (isQualType(con)) {		/* Local predicates		   */
	    List us;
	    lps     = fst(snd(con));
	    for (us = typeVarsIn(lps,NIL,NIL,NIL); nonNull(us); us=tl(us))
		if (!varIsMember(textOf(hd(us)),tyvars) &&
		    !varIsMember(textOf(hd(us)),evs)) {
		    ERRMSG(line)
			"Variable \"%s\" in constraint is not locally bound",
			textToStr(textOf(hd(us)))
		    EEND;
		}
	    map2Over(depPredExp,line,sig,lps);
	    con     = snd(snd(con));
	    arity   = length(lps);
	}

	if (whatIs(con)==LABC) {	/* Skeletize constr components	   */
	    Cell fls = snd(snd(con));	/* get field specifications	   */
	    con      = fst(snd(con));
	    fs	     = NIL;
	    for (; nonNull(fls); fls=tl(fls)) { /* for each field spec:    */
		List vs     = fst(hd(fls));
		Type t      = snd(hd(fls));	/* - scrutinize type	   */
		Bool banged = whatIs(t)==BANG;
		t           = depCompType(line,sig,(banged ? arg(t) : t));
		while (nonNull(vs)) {		/* - add named components  */
		    Cell us = tl(vs);
		    tl(vs)  = fs;
		    fs      = vs;
		    vs      = us;
		    con     = ap(con,t);
		    arity++;
		    if (banged)
			scs = cons(mkInt(arity),scs);
		}
	    }
	    fs  = rev(fs);
	    scs = rev(scs);		/* put strict comps in ascend ord  */
	}
	else {				/* Non-labelled constructor	   */
	    Cell c = con;
	    Int  compNo;
	    for (; isAp(c); c=fun(c))
		arity++;
	    for (compNo=arity, c=con; isAp(c); c=fun(c)) {
		Type t = arg(c);
		if (whatIs(t)==BANG) {
		    scs = cons(mkInt(compNo),scs);
		    t   = arg(t);
		}
		compNo--;
		arg(c) = depCompType(line,sig,t);
	    }
	}

	if (nonNull(ctxt1))		/* Extract relevant part of context*/
	    ctxt1 = selectCtxt(ctxt1,offsetTyvarsIn(con,NIL));

	for (i=arity; isAp(con); i--) {	/* Calculate type of constructor   */
	    Type ty  = fun(con);
	    Type cmp = arg(con);
	    fun(con) = typeArrow;
	    if (isPolyOrQualType(cmp)) {
		if (nonNull(derivs)) {
		    ERRMSG(line) "Cannot derive instances for types" ETHEN
		    ERRTEXT      " with polymorphic or qualified components"
		    EEND;
		}
		if (nr2==0)
		    nr2 = i;
	    }
	    if (nonNull(derivs))	/* and build list of components	   */
		compTypes = cons(cmp,compTypes);
	    type     = ap(con,type);
	    con      = ty;
	}

 	if (nr2>0) {			/* Add rank 2 annotation	   */
	    type = ap(RANK2,pair(mkInt(nr2-length(lps)),type));
	}

	if (nonNull(evs)) {		/* Add existential annotation	   */
	    if (nonNull(derivs)) {
		ERRMSG(line) "Cannot derive instances for types" ETHEN
		ERRTEXT      " with existentially typed components"
		EEND;
	    }
	    if (fs!=NONE) {
		ERRMSG(line)
		   "Cannot use selectors with existentially typed components"
		EEND;
	    }
	    type = ap(EXIST,pair(mkInt(length(evs)),type));
	}

	if (nonNull(lps)) {		/* Add local preds part to type	   */
	    type = ap(CDICTS,pair(lps,type));
	}

	if (nonNull(ctxt1)) {		/* Add context part to type	   */
	    type = ap(QUAL,pair(ctxt1,type));
	}

	if (nonNull(sig)) {		/* Add quantifiers to type	   */
	    List ts1 = sig;
	    for (; nonNull(ts1); ts1=tl(ts1)) {
		hd(ts1) = NIL;
	    }
	    type = mkPolyType(sig,type);
	}

	n = findName(textOf(con));	/* Allocate constructor fun name   */
	if (isNull(n)) {
	    n = newName(textOf(con),NIL);
	} else if (name(n).defn!=PREDEFINED && name(n).mod == currentModule) {
	    /* A local repeated definition */
	    duplicateError(line,name(n).mod,name(n).text,"data constructor");
	} else if (name(n).defn!=PREDEFINED) {
	    Name oldnm = n;
	    removeName(n);
	    n            = newName(textOf(con),NIL);
	    name(n).defn = PREDEFINED;
	    name(n).clashes = cons(oldnm,name(n).clashes);
	}

	name(n).arity  = arity;		/* Save constructor fun details	   */
	name(n).line   = line;
	name(n).parent = t;
	name(n).number = cfunNo(conNo++);
	name(n).type   = type;
	if (tycon(t).what==NEWTYPE) {
	    if (nonNull(lps)) {
		ERRMSG(line)
		   "A newtype constructor cannot have class constraints"
		EEND;
	    }
	    if (arity!=1) {
		ERRMSG(line)
		   "A newtype constructor must have exactly one argument"
		EEND;
	    }
	    if (nonNull(scs)) {
		ERRMSG(line)
		   "Illegal strictness annotation for newtype constructor"
		EEND;
	    }
	    name(n).defn = nameId;
	} else {
	    implementCfun(n,scs);
	}

	hd(cs) = n;
	if (fs!=NONE) {
	    sels = addSels(line,n,fs,sels);
	}
    }

    if (nonNull(sels)) {
	sels     = rev(sels);
	fst(cd)  = appendOnto(fst(cd),sels);
	selDefns = cons(sels,selDefns);
    }

    if (nonNull(derivs)) {		/* Generate derived instances	   */
	map3Proc(checkDerive,t,ctxt,compTypes,derivs);
    }
}

static Int local userArity(Name c)		/* Find arity for cfun, ignoring   */
{				/* CDICTS parameters		   */
    Int  a = name(c).arity;
    Type t = name(c).type;
    Int  w;
    if (isPolyType(t)) {
	t = monotypeOf(t);
    }
    if ((w=whatIs(t))==QUAL) {
	t=snd(snd(t));
	w = whatIs(t);
    }
    if (w==CDICTS) {
	a -= length(fst(snd(t)));
    }
    return a;
}

static List cfunSfuns;			/* List of (Cfun,[SelectorVar])	   */
					/*  - used for deriving Show       */

static List local addSels(Int line, Name c, List fs, List ss)	/* Add fields to selector list	   */
              /* line number of constructor	   */
            /* corresponding constr function   */
            /* list of fields (varids)	   */
{				/* list of existing selectors	   */
    Int sn    = 1;
    cfunSfuns = cons(pair(c,fs),cfunSfuns);
    for (; nonNull(fs); fs=tl(fs), ++sn) {
	List ns = ss;
	Text t  = textOf(hd(fs));

	if (nonNull(varIsMember(t,tl(fs)))) {
	    ERRMSG(line) "Repeated field name \"%s\" for constructor \"%s\"",
			 textToStr(t), textToStr(name(c).text)
	    EEND;
	}

	while (nonNull(ns) && t!=name(hd(ns)).text) {
	    ns = tl(ns);
	}

	if (nonNull(ns)) {
	    name(hd(ns)).defn = cons(pair(c,mkInt(sn)),name(hd(ns)).defn);
	} else {
	    Name n;
	    Name oldnm = findName(t);
	    if ( nonNull(oldnm) ) {
	        if ( name(oldnm).mod == currentModule ) {
		    ERRMSG(line) "Multiple declarations for selector \"%s\"",
			         textToStr(t)
		    EEND;
		} else {
		    removeName(oldnm);
		}
	    }
	    n               = newName(t,c);
	    name(n).line    = line;
	    name(n).number  = SELNAME;
	    name(n).defn    = singleton(pair(c,mkInt(sn)));
	    if (nonNull(oldnm)) {
		name(n).clashes = cons(oldnm,name(n).clashes);
	    }
	    ss              = cons(n,ss);
	}
    }
    return ss;
}

static List local selectCtxt(List ctxt, List vs)	/* calculate subset of context	   */
{
    if (isNull(vs)) {
	return NIL;
    } else {
	List ps = NIL;
	for (; nonNull(ctxt); ctxt=tl(ctxt)) {
	    List us = offsetTyvarsIn(hd(ctxt),NIL);
	    for (; nonNull(us) && cellIsMember(hd(us),vs); us=tl(us)) {
	    }
	    if (isNull(us)) {
		ps = cons(hd(ctxt),ps);
	    }
	}
	return rev(ps);
    }
}

static Void local checkSynonyms(List ts)	/* Check for mutually recursive	   */
{				/* synonyms			   */
    List syns = NIL;
    for (; nonNull(ts); ts=tl(ts)) {	/* build list of all synonyms	   */
	Tycon t = hd(ts);
	switch (whatIs(tycon(t).what)) {
	    case SYNONYM     :
	    case RESTRICTSYN : syns = cons(t,syns);
			       break;
	    default          : break;
	}
    }
    while (nonNull(syns)) {		/* then visit each synonym	   */
	syns = visitSyn(NIL,hd(syns),syns);
    }
}

static List local visitSyn(List path, Tycon t, List syns)	/* visit synonym definition to look*/
               /* for cycles			   */
{
    if (cellIsMember(t,path)) {		/* every elt in path depends on t  */
	ERRMSG(tycon(t).line)
	    "Type synonyms \"%s\" and \"%s\" are mutually recursive",
	    textToStr(tycon(t).text), textToStr(tycon(hd(path)).text)
	EEND;
    } else {
	List ds    = tycon(t).kind;
	List path1 = NIL;
	for (; nonNull(ds); ds=tl(ds)) {
	    if (cellIsMember(hd(ds),syns)) {
		if (isNull(path1)) {
		    path1 = cons(t,path);
		}
		syns = visitSyn(path1,hd(ds),syns);
	    }
	}
    }
    tycon(t).defn = fullExpand(tycon(t).defn);
    return removeCell(t,syns);
}

/* --------------------------------------------------------------------------
 * Expanding out all type synonyms and newtypes in a type expression:
 * ------------------------------------------------------------------------*/

static Type local fullerExpand(Type t)	/* find full expansion of type exp */
{				/* assuming that all relevant type */
    Cell h = t;				/* synonym defns of lower rank have*/
    Int  n = 0;				/* already been fully expanded but */
    List args;                          /* not assuming same for newtypes  */
    for (args=NIL; isAp(h); h=fun(h), n++) {
        /* Does not apply recursively because the ffi is only interested
         * in the top level constructors
         */
	args = cons(arg(h),args);
    }
    t = applyToArgs(h,args);
    if (isSynonym(h) && n>=tycon(h).arity) {
	if (n==tycon(h).arity) {
	    t = instantiateSyn(tycon(h).defn,t);
	} else {
	    Type p = t;
	    while (--n > tycon(h).arity) {
		p = fun(p);
	    }
	    fun(p) = instantiateSyn(tycon(h).defn,fun(p));
	}
        t = fullerExpand(t); /* chase synonym chains. */
    } else if (isNewtype(h) && n==tycon(h).arity && h != typeIO) {
        if (n != 0) {
            /* Not supported because I don't understand the typechecker
             * well enough.  For those that grok the data structures, it
             * should be simple.
             */
            ERRMSG(name(h).line) "Use of polymorphic newtype '" ETHEN
            ERRTYPE(t);
            ERRTEXT "' not supported in foreign function declarations."
            EEND;
        }
        t = instantiateNewtype(hd(tycon(h).defn),t);
        t = fullerExpand(t); /* chase chains of newtypes */
    }
    return t;
}

Bool hasIOResultType(Type ty) /* return TRUE if FFI/primitive type sig is an IO action. */
{
  Type t = ty;
  if (isPolyType(t)) {
        t = monotypeOf(t);
    }
  t = fullerExpand(t);
  while (getHead(t) == typeArrow && argCount == 2) {
    t = fullerExpand(arg(t));
  }
  return (getHead(t) == typeIO && argCount == 1);
}

static Type local instantiateNewtype(Name c, Type env) /* instantiate type using      */
            /* env to determine appropriate    */
{				/* values for OFFSET type vars	   */
    Type t = NIL;
    assert(isName(c));
    t = name(c).type;
    if (isPolyType(t)) {
        t = monotypeOf(t);
    }
    assert(getHead(t)==typeArrow && argCount==2);
    t = arg(fun(t));
    /* This is probably where we should invoke instantiateSyn(t,env) */
    return t;
}

/* --------------------------------------------------------------------------
 * Expanding out all type synonyms in a type expression:
 * ------------------------------------------------------------------------*/

Type fullExpand(Type t)			/* find full expansion of type exp */
{				/* assuming that all relevant      */
    Cell h = t;				/* synonym defns of lower rank have*/
    Int  n = 0;				/* already been fully expanded	   */
    List args;
    for (args=NIL; isAp(h); h=fun(h), n++) {
	args = cons(fullExpand(arg(h)),args);
    }
    t = applyToArgs(h,args);
    if (isSynonym(h) && n>=tycon(h).arity) {
	if (n==tycon(h).arity) {
	    t = instantiateSyn(tycon(h).defn,t);
	} else {
	    Type p = t;
	    while (--n > tycon(h).arity) {
		p = fun(p);
	    }
	    fun(p) = instantiateSyn(tycon(h).defn,fun(p));
	}
    }
    return t;
}

static Type local instantiateSyn(Type t, Type env)	/* instantiate type according using*/
            /* env to determine appropriate    */
{				/* values for OFFSET type vars	   */
    switch (whatIs(t)) {
	case AP      : return ap(instantiateSyn(fun(t),env),
				 instantiateSyn(arg(t),env));

	case OFFSET  : return nthArg(offsetOf(t),env);

	default	     : return t;
    }
}

/* --------------------------------------------------------------------------
 * Static analysis of class declarations:
 *
 * Performed in a similar manner to that used for type declarations.
 *
 * The first part of the static analysis is performed as the declarations
 * are read during parsing.  The parser ensures that:
 * - the class header and all superclass predicates are of the form
 *   ``Class var''
 *
 * The classDefn() function:
 * - ensures that there is no previous definition for class
 * - checks that class name has not previously been used as a type constr.
 * - make new entry in class table
 * - record line number of declaration
 * - build list of classes defined in current script for use in later
 *   stages of static analysis.
 * ------------------------------------------------------------------------*/

Void classDefn(Int line, Cell head, List ms, List fds)       /* process new class definition	   */
                    /* definition line number	   */
                    /* class header :: ([Supers],Class) */
                  /* class definition body		   */
{			       /* functional dependencies	   */
    Text ct    = textOf(getHead(snd(head)));
    Int  arity = argCount;

    if (nonNull(findClass(ct))) {
	ERRMSG(line) "Multiple declarations of class \"%s\"",
		     textToStr(ct)
	EEND;
    } else if (nonNull(findTycon(ct))) {
	ERRMSG(line) "\"%s\" used as both class and type constructor",
		     textToStr(ct)
	EEND;
    } else {
	Class nw	   = newClass(ct);
	cclass(nw).line    = line;
	cclass(nw).arity   = arity;
	cclass(nw).head    = snd(head);
	cclass(nw).supers  = fst(head);
	cclass(nw).members = ms;
	cclass(nw).level   = 0;
	cclass(nw).fds	   = fds;
	cclass(nw).xfds	   = NIL;
	classDefns	   = cons(nw,classDefns);
    }
}

/* --------------------------------------------------------------------------
 * Further analysis of class declarations:
 *
 * Full static analysis of class definitions must be postponed until the
 * complete script has been read and all static analysis on type definitions
 * has been completed.
 *
 * Once this has been achieved, we carry out the following checks on each
 * class definition:
 * - check that variables in header are distinct
 * - replace head by skeleton
 * - check superclass declarations, replace by skeletons
 * - split body of class into members and declarations
 * - make new name entry for each member function
 * - record member function number (eventually an offset into dictionary!)
 * - no member function has a previous definition ...
 * - no member function is mentioned more than once in the list of members
 * - each member function type is valid, replace vars by offsets
 * - qualify each member function type by class header
 * - only bindings for members appear in defaults
 * - only function bindings appear in defaults
 * - check that extended class hierarchy does not contain any cycles
 * ------------------------------------------------------------------------*/

static Void local checkClassDefn(Class c)    /* validate class definition	   */
{
    List tyvars = NIL;
    Cell temp   = cclass(c).head;
    List fs     = NIL;
    List ss     = NIL;

    for (; isAp(temp); temp=fun(temp)) {
	if (!isVar(arg(temp))) {
	    ERRMSG(cclass(c).line) "Type variable required in class head"
	    EEND;
	}
	if (nonNull(varIsMember(textOf(arg(temp)),tyvars))) {
	    ERRMSG(cclass(c).line)
		"Repeated type variable \"%s\" in class head",
		textToStr(textOf(arg(temp)))
	    EEND;
	}
	tyvars = cons(arg(temp),tyvars);
    }

    for (fs=cclass(c).fds; nonNull(fs); fs=tl(fs)) {
	Pair fd = hd(fs);
	List vs = snd(fd);

	/* Check for trivial dependency
	 */
	if (isNull(vs)) {
	    ERRMSG(cclass(c).line) "Functional dependency is trivial"
	    EEND;
	}

	/* Check for duplicated vars on right hand side, and for vars on
	 * right that also appear on the left:
	 */
	for (vs=snd(fd); nonNull(vs); vs=tl(vs)) {
	    if (varIsMember(textOf(hd(vs)),fst(fd))) {
		ERRMSG(cclass(c).line)
		    "Trivial dependency for variable \"%s\"",
		    textToStr(textOf(hd(vs)))
		EEND;
	    }
	    if (varIsMember(textOf(hd(vs)),tl(vs))) {
		ERRMSG(cclass(c).line)
		    "Repeated variable \"%s\" in functional dependency",
		    textToStr(textOf(hd(vs)))
		EEND;
	    }
	    hd(vs) = depTypeVar(cclass(c).line,tyvars,textOf(hd(vs)));
	}

	/* Check for duplicated vars on left hand side:
	 */
	for (vs=fst(fd); nonNull(vs); vs=tl(vs)) {
	    if (varIsMember(textOf(hd(vs)),tl(vs))) {
		ERRMSG(cclass(c).line)
		    "Repeated variable \"%s\" in functional dependency",
		    textToStr(textOf(hd(vs)))
		EEND;
	    }
	    hd(vs) = depTypeVar(cclass(c).line,tyvars,textOf(hd(vs)));
	}
    }

    /* add in the tyvars from the `supers' so that we don't
       prematurely complain about undefined tyvars */
    tyvars = typeVarsIn(cclass(c).supers,NIL,NIL,tyvars);
    cclass(c).tyvars = dupList(tyvars);

    if (cclass(c).arity==0) {
	cclass(c).head = c;
    } else {
	Int args = cclass(c).arity - 1;
	for (temp=cclass(c).head; args>0; temp=fun(temp), args--) {
	    arg(temp) = mkOffset(args);
	}
	arg(temp) = mkOffset(0);
	fun(temp) = c;
    }

    tcDeps	        = NIL;		/* find dependents		   */
    map2Over(depPredExp,cclass(c).line,tyvars,cclass(c).supers);


    cclass(c).numSupers = length(cclass(c).supers);
    cclass(c).defaults  = extractBindings(cclass(c).members);	/* defaults*/
    ss			= extractSigdecls(cclass(c).members);
    fs			= extractFixdecls(cclass(c).members);
    cclass(c).members   = pair(ss,fs);
    map2Proc(checkMems,c,tyvars,ss);

    cclass(c).kinds     = tcDeps;
    tcDeps              = NIL;
}

static Void local checkClassDefn2_(List cs)
{
    mapProc(checkClassDefn2,cs);
}

static Void local checkClassDefn2(Class c)    /* validate class definition, pt 2 */
{				/* can only finish this job after  */
					/* we've inherited fds             */
					/* and are in dependency order     */
    if (!isTycon(c)) {
	List tvts = offsetTyvarsIn(cclass(c).head,NIL);
	List tvps = offsetTyvarsIn(cclass(c).supers,NIL);
	List fds  = calcFunDeps(cclass(c).supers);
	tvts = oclose(fds,tvts);
	tvts = odiff(tvps,tvts);

	if (!isNull(tvts)) {
	    ERRMSG(cclass(c).line) "Undefined type variable \"%s\"",
	      textToStr(textOf(nth(offsetOf(hd(tvts)),cclass(c).tyvars)))
	    EEND;
	}
    }
}

/* --------------------------------------------------------------------------
 * Functional dependencies are inherited from superclasses.
 * For example, if I've got the following classes:
 *
 * class C a b | a -> b
 * class C [b] a => D a b
 *
 * then C will have the dependency ([a], [b]) as expected, and D will inherit
 * the dependency ([b], [a]) from C.
 * When doing pairwise improvement, we have to consider not just improving
 * when we see a pair of Cs or a pair of Ds in the context, but when we've
 * got a C and a D as well.  In this case, we only improve when the
 * predicate in question matches the type skeleton in the relevant superclass
 * constraint.  E.g., we improve the pair (C [Int] a, D b Int) (unifying
 * a and b), but we don't improve the pair (C Int a, D b Int).
 * To implement functional dependency inheritance, we calculate
 * the closure of all functional dependencies, and store the result
 * in an additional field `xfds' (extended functional dependencies).
 * The `xfds' field is a list of functional dependency lists, annotated
 * with a list of predicate skeletons constraining when improvement can
 * happen against this dependency list.  For example, the xfds field
 * for C above would be:
 *     [([C a b], [([a], [b])])]
 * and the xfds field for D would be:
 *     [([C [b] a, D a b], [([b], [a])])]
 * Self-improvement (of a C with a C, or a D with a D) is treated as a
 * special case of an inherited dependency.
 * ------------------------------------------------------------------------*/
static List local inheritFundeps(Class c, Cell pi, Int o)
{
    Int alpha = newKindedVars(cclass(c).kinds);
    List scs = cclass(c).supers;
    List xfds = NIL;
    Cell this = NIL;
    /* alloc additional vars for any vars in supers not in the head */
    newKindvars(length(cclass(c).tyvars) - cclass(c).arity);

    /* better not fail ;-) */
    if (!matchPred(pi,o,cclass(c).head,alpha)) {
	/* If the qualified type is not valid, for instance by
	 * having type variables occurring free in the context,
	 * but not in the head -- we will end up here.
	 *
	 * Silently give up & assume that checkClassDefn2() will
	 * catch the error condition.
	 */
	return xfds;
    }
    this = copyPred(pi,o);
    for (; nonNull(scs); scs=tl(scs)) {
	Class s = getHead(hd(scs));
	if (isClass(s)) {
	    List sfds = inheritFundeps(s,hd(scs),alpha);
	    for (; nonNull(sfds); sfds=tl(sfds)) {
		Cell h = hd(sfds);
		xfds = cons(pair(cons(this,fst(h)),snd(h)),xfds);
	    }
	}
    }
    if (nonNull(cclass(c).fds)) {
	List fds = NIL, fs = cclass(c).fds;
	for (; nonNull(fs); fs=tl(fs)) {
	    fds = cons(pair(otvars(this,fst(hd(fs))),
			    otvars(this,snd(hd(fs)))),fds);
	}
	xfds = cons(pair(cons(this,NIL),fds),xfds);
    }
    return xfds;
}

static Void local extendFundeps(Class c)
{
    Int alpha;
    emptySubstitution();
    alpha = newKindedVars(cclass(c).kinds);
    cclass(c).xfds = inheritFundeps(c,cclass(c).head,alpha);

    /* we can now check for ambiguity */
    map1Proc(checkMems2,c,fst(cclass(c).members));
}

static Cell local depPredExp(Int line, List tyvars, Cell pred)
{
    Int  args = 0;
    Cell prev = NIL;
    Cell h    = pred;
    for (; isAp(h); args++) {
	arg(h) = depTypeExp(line,tyvars,arg(h));
	prev   = h;
	h      = fun(h);
    }

    if (isQCon(h)) {			/* standard class constraint	   */
	Class c = findQualClass(h);
	if (isNull(c)) {
	    ERRMSG(line) "Undefined class \"%s\"", identToStr(h)
	    EEND;
	}
	if (!isQualIdent(h) && nonNull(cclass(c).clashes)) {
	    List ls = cclass(c).clashes;
	    ERRMSG(line) "Ambiguous class occurrence \"%s\"", textToStr(cclass(c).text) ETHEN
	    ERRTEXT "\n*** Could refer to: " ETHEN
	    ERRTEXT "%s.%s ", textToStr(module(cclass(c).mod).text), textToStr(cclass(c).text) ETHEN
	    for (;nonNull(ls);ls=tl(ls)) {
		ERRTEXT "%s.%s ",
			textToStr(module(cclass(hd(ls)).mod).text),
			textToStr(cclass(hd(ls)).text)
			ETHEN
	    }
	    ERRTEXT "\n" EEND;
        }
	if (isNull(prev)) {
	    pred = c;
	} else {
	    fun(prev) = c;
	}
	if (args!=cclass(c).arity) {
	    ERRMSG(line) "Wrong number of arguments for class \"%s\"",
			textToStr(cclass(c).text)
	    EEND;
	}
	if (cellIsMember(c,classDefns) && !cellIsMember(c,tcDeps)) {
	    tcDeps = cons(c,tcDeps);
	}
    }
    else
    {
	internal("depPredExp");
    }
    return pred;
}

static Void local checkMems(Class c, List tyvars, Cell m)	/* check member function details   */
{
    Int  line = intOf(fst3(m));
    List vs   = snd3(m);
    Type t    = thd3(m);
    List sig  = NIL;
    List tvs  = NIL;
    List xtvs = NIL;

    if (isPolyType(t)) {
	xtvs = fst(snd(t));
	t    = monotypeOf(t);
    }

    tyvars    = typeVarsIn(t,NIL,xtvs,tyvars);
					/* Look for extra type vars.	   */
    checkOptQuantVars(line,xtvs,tyvars);

    if (isQualType(t)) {		/* Overloaded member signatures?   */
	map2Over(depPredExp,line,tyvars,fst(snd(t)));
    } else {
	t = ap(QUAL,pair(NIL,t));
    }

    fst(snd(t)) = cons(cclass(c).head,fst(snd(t)));/* Add main predicate   */
    snd(snd(t)) = depTopType(line,tyvars,snd(snd(t)));

    for (tvs=tyvars; nonNull(tvs); tvs=tl(tvs)){/* Quantify		   */
	sig = ap(NIL,sig);
    }
    if (nonNull(sig)) {
	t = mkPolyType(sig,t);
    }
    thd3(m) = t;				/* Save type		   */
    take(cclass(c).arity,tyvars);		/* Delete extra type vars  */
}

static Void local checkMems2(Class c, Cell m) /* check member function details   */
{
    Int  line = intOf(fst3(m));
    List vs   = snd3(m);
    Type t    = thd3(m);

    if (isAmbiguous(t)) {
	ambigError(line,"class declaration",hd(vs),t);
    }
}

static Void local addMembers(Class c)		/* Add definitions of member funs  */
{				/* and other parts of class struct.*/
    List ms  = fst(cclass(c).members);
    List fs  = snd(cclass(c).members);
    List ns  = NIL;			/* List of names		   */
    Int  mno;				/* Member function number	   */

    for (mno=0; mno<cclass(c).numSupers; mno++) {
	ns = cons(newDSel(c,mno),ns);
    }
    cclass(c).dsels = rev(ns);		/* Save dictionary selectors	   */

    for (mno=1, ns=NIL; nonNull(ms); ms=tl(ms)) {
	Int  line = intOf(fst3(hd(ms)));
	List vs   = rev(snd3(hd(ms)));
	Type t    = thd3(hd(ms));
	for (; nonNull(vs); vs=tl(vs)) {
	    ns = cons(newMember(line,mno++,hd(vs),t,c),ns);
	}
    }
    cclass(c).members    = rev(ns);	/* Save list of members		   */
    cclass(c).numMembers = length(cclass(c).members);

    for (; nonNull(fs); fs=tl(fs)) {	/* fixity declarations		   */
	Int    line = intOf(fst3(hd(fs)));
	List   ops  = snd3(hd(fs));
	Syntax s    = intOf(thd3(hd(fs)));
	for (; nonNull(ops); ops=tl(ops)) {
	    Name n = nameIsMember(textOf(hd(ops)),cclass(c).members);
	    if (isNull(n)) {
		missFixity(line,textOf(hd(ops)));
	    } else if (name(n).syntax!=NO_SYNTAX) {
		dupFixity(line,textOf(hd(ops)));
	    }
	    name(n).syntax = s;
	}
    }

/*  Not actually needed just yet; for the time being, dictionary code will
    not be passed through the type checker.

    cclass(c).dtycon   = addPrimTycon(generateText("Dict.%s",c),
				      NIL,
				      cclass(c).arity,
				      DATATYPE,
				      NIL);
*/

    mno		       = cclass(c).numSupers + cclass(c).numMembers;
    cclass(c).dcon     = addPrimCfun(generateText("Make.%s",c),mno,0,NIL);
    if (mno==1)	{			/* Single entry dicts use newtype  */
	name(cclass(c).dcon).defn = nameId;
	if (nonNull(cclass(c).members)) {
	    name(hd(cclass(c).members)).number = mfunNo(0);
	}
    }
    cclass(c).defaults = classBindings("class",c,cclass(c).defaults);
}

static Name local newMember(Int l, Int no, Cell v, Type t, Class parent)
            /* Make definition for member fn   */
{
    Name m = findName(textOf(v));

    if (isNull(m)) {
	m = newName(textOf(v),parent);
    } else if (name(m).defn!=PREDEFINED && name(m).mod == currentModule) {
	ERRMSG(l) "Multiple declarations for member function \"%s\"",
	  textToStr(name(m).text)
	EEND;
    } else if (name(m).defn!=PREDEFINED) {
	Name oldnm = m;
	removeName(m);
	m            = newName(textOf(v),parent);
	name(m).defn = PREDEFINED;
	name(m).clashes = cons(oldnm,name(m).clashes);
    } else if ( name(m).parent == NIL ) {
	/* (try) improving the parent info. */
	name(m).parent = parent;
    }
    name(m).line   = l;
    name(m).arity  = 1;
    name(m).number = mfunNo(no);
    name(m).type   = t;
    return m;
}

static Name local newDSel(Class c, Int no)		/* Make definition for dict selectr*/
{
    Name s;
    char buf[16];

    sprintf(buf,"_sc%d_%s",no,"%s");
    s              = newName(generateText(buf,c),c);
    name(s).line   = cclass(c).line;
    name(s).arity  = 1;
    name(s).number = DFUNNAME;
    return s;
}

#define MAX_GEN  128

static Text local generateText(String sk, Class c)	/* We need to generate names for   */
              /* certain objects corresponding   */
{				/* to each class.		   */
    String cname = textToStr(cclass(c).text);
    char buffer[MAX_GEN+1];

    if ((strlen(sk)+strlen(cname))>=MAX_GEN) {
	ERRMSG(0) "Please use a shorter name for class \"%s\"", cname
	EEND;
    }
    sprintf(buffer,sk,cname);
    return findText(buffer);
}

static Int local visitClass(Class c)		/* visit class defn to check that  */
{				/* class hierarchy is acyclic	   */
    if (cclass(c).level < 0) {		/* already visiting this class?	   */
	ERRMSG(cclass(c).line) "Superclass relation for \"%s\" is cyclic",
			       textToStr(cclass(c).text)
	EEND;
    } else if (cclass(c).level == 0) {	/* visiting class for first time   */
	List scs = cclass(c).supers;
	Int  lev = 0;
	cclass(c).level = (-1);
	for (; nonNull(scs); scs=tl(scs)) {
	    Int l = visitClass(getHead(hd(scs)));
	    if (l>lev) lev=l;
	}
	cclass(c).level = 1+lev;	/* level = 1 + max level of supers */
    }
    return cclass(c).level;
}

/* --------------------------------------------------------------------------
 * Process class and instance declaration binding groups:
 * ------------------------------------------------------------------------*/

static List local classBindings(String where, Class c, List bs)
                 /* Check validity of bindings bs   */
             /* for class c (or an inst of c)   */
{				/* sort into approp. member order  */
    List nbs = NIL;
    Text nm;

    for (; nonNull(bs); bs=tl(bs)) {
	Cell b    = hd(bs);
	Cell body = snd(snd(b));
	Name mnm;

	if ( !(isVar(fst(b))) ) { /* Only allow function bindings    */
	    ERRMSG(rhsLine(snd(body)))
		"Pattern binding illegal in %s declaration", where
	    EEND;
	}

	nm = textOf(fst(b));

	if (isNull(mnm=memberName(c,nm))) {
	    ERRMSG(rhsLine(snd(hd(body))))
		"No member \"%s\" in class \"%s\"",
		textToStr(nm), textToStr(cclass(c).text)
	    EEND;
	}
	snd(b) = body;
	nbs    = numInsert(mfunOf(mnm)-1,b,nbs);
    }
    return nbs;
}

static Name local memberName(Class c, Text t)	/* return name of member function  */
            /* with name t in class c	   */
{				/* return NIL if not a member	   */
    List ms = cclass(c).members;
    for (; nonNull(ms); ms=tl(ms)) {
	if (t==name(hd(ms)).text) {
	    return hd(ms);
	}
    }
    return NIL;
}

static List local numInsert(Int n, Cell x, List xs)    /* insert x at nth position in xs,  */
                                       /* filling gaps with NIL            */
{
    List start = isNull(xs) ? cons(NIL,NIL) : xs;

    for (xs=start; 0<n--; xs=tl(xs)) {
	if (isNull(tl(xs))) {
	    tl(xs) = cons(NIL,NIL);
	}
    }
    hd(xs) = x;
    return start;
}

/* --------------------------------------------------------------------------
 * Calculate set of variables appearing in a given type expression (possibly
 * qualified) as a list of distinct values.  The order in which variables
 * appear in the list is the same as the order in which those variables
 * occur in the type expression when read from left to right.
 * ------------------------------------------------------------------------*/

static List local typeVarsIn(Cell ty, List us, List ws, List vs)/*Calculate list of type variables*/
            /* used in type expression, reading*/
            /* from left to right ignoring any */
            /* listed in us.		   */
{				/* ws = explicitly quantified vars */
    switch (whatIs(ty)) {
	case AP        : return typeVarsIn(snd(ty),us,ws,
					   typeVarsIn(fst(ty),us,ws,vs));

	case VARIDCELL :
	case VAROPCELL : if ((nonNull(findBtyvs(textOf(ty)))
			      && !varIsMember(textOf(ty),ws))
			     || varIsMember(textOf(ty),us)) {
			     return vs;
			 } else {
			     return maybeAppendVar(ty,vs);
			 }

	case POLYTYPE  : return typeVarsIn(monotypeOf(ty),polySigOf(ty),ws,vs);

	case QUAL      : {   vs = typeVarsIn(fst(snd(ty)),us,ws,vs);
			     return typeVarsIn(snd(snd(ty)),us,ws,vs);
			 }

	case BANG      : return typeVarsIn(snd(ty),us,ws,vs);

	case LABC      : {   List fs = snd(snd(ty));
			     for (; nonNull(fs); fs=tl(fs)) {
				vs = typeVarsIn(snd(hd(fs)),us,ws,vs);
			     }
			     return vs;
			 }
	default        : break;
    }
    return vs;
}

static List local maybeAppendVar(Cell v, List vs) /* append variable to list if not   */
                  /* already included		   */
{
    Text t = textOf(v);
    List p = NIL;
    List c = vs;

    while (nonNull(c)) {
	if (textOf(hd(c))==t) {
	    return vs;
	}
	p = c;
	c = tl(c);
    }

    if (nonNull(p)) {
	tl(p) = cons(v,NIL);
    } else {
	vs    = cons(v,NIL);
    }

    return vs;
}

/* --------------------------------------------------------------------------
 * Static analysis for type expressions is required to:
 *   - ensure that each type constructor or class used has been defined.
 *   - replace type variables by offsets, constructor names by Tycons.
 *   - ensure that the type is well-kinded.
 * ------------------------------------------------------------------------*/

static Type local checkSigType(Int line, String where, Cell e, Type type)
                /* Check validity of type expr in  */
                 /* explicit type signature	   */
{
    List tvs  = NIL;
    List sunk = NIL;
    List xtvs = NIL;

    if (isPolyType(type)) {
	xtvs = fst(snd(type));
	type = monotypeOf(type);
    }
    tvs  = typeVarsIn(type,NIL,xtvs,NIL);
    sunk = unkindTypes;
    checkOptQuantVars(line,xtvs,tvs);

    if (isQualType(type)) {
	map2Over(depPredExp,line,tvs,fst(snd(type)));
	snd(snd(type)) = depTopType(line,tvs,snd(snd(type)));

	if (isAmbiguous(type)) {
	    ambigError(line,where,e,type);
	}
    } else {
	type = depTopType(line,tvs,type);
    }

    if (nonNull(tvs)) {
	if (length(tvs)>=NUM_OFFSETS) {
	    ERRMSG(line) "Too many type variables in %s\n", where
	    EEND;
	} else {
	    List ts = tvs;
	    for (; nonNull(ts); ts=tl(ts)) {
		hd(ts) = NIL;
	    }
	    type    = mkPolyType(tvs,type);
	}
    }

    unkindTypes = NIL;
    kindType(line,"type expression",type);
    fixKinds();
    unkindTypes = sunk;
    return type;
}

static Void local checkOptQuantVars(Int line, List xtvs, List tvs)
              /* Explicitly quantified vars	   */
{				/* Implicitly quantified vars	   */
    if (nonNull(xtvs)) {
	List vs = tvs;
	for (; nonNull(vs); vs=tl(vs)) {
	    if (!varIsMember(textOf(hd(vs)),xtvs)) {
		ERRMSG(line) "Quantifier does not mention type variable \"%s\"",
			     textToStr(textOf(hd(vs)))
		EEND;
	    }
	}
	for (vs=xtvs; nonNull(vs); vs=tl(vs)) {
	    if (!varIsMember(textOf(hd(vs)),tvs)) {
		ERRMSG(line) "Quantified type variable \"%s\" is not used",
			     textToStr(textOf(hd(vs)))
		EEND;
	    }
	    if (varIsMember(textOf(hd(vs)),tl(vs))) {
		ERRMSG(line) "Quantified type variable \"%s\" is repeated",
			     textToStr(textOf(hd(vs)))
		EEND;
	    }
	}
    }
}

static Type local depTopType(Int l, List tvs, Type t)	/* Check top-level of type sig     */
{
    Type prev = NIL;
    Type t1   = t;
    Int  nr2  = 0;
    Int  i    = 1;
    for (; getHead(t1)==typeArrow && argCount==2; ++i) {
	arg(fun(t1)) = depCompType(l,tvs,arg(fun(t1)));
	if (isPolyOrQualType(arg(fun(t1)))) {
	    nr2 = i;
	}
	prev = t1;
	t1   = arg(t1);
    }
    if (nonNull(prev)) {
	arg(prev) = depTypeExp(l,tvs,t1);
    } else {
	t = depTypeExp(l,tvs,t1);
    }

    if (nr2>0) {
	t = ap(RANK2,pair(mkInt(nr2),t));
    }

    return t;
}

static Type local depCompType(Int l, List tvs, Type t)	/* Check component type for constr */
{
    Int  ntvs = length(tvs);
    List nfr  = NIL;
    if (isPolyType(t)) {
	List vs = fst(snd(t));
	t	= monotypeOf(t);
	tvs	= checkQuantVars(l,vs,tvs,t);
	nfr	= replicate(length(vs),NIL);
    }

    if (isQualType(t)) {
	map2Over(depPredExp,l,tvs,fst(snd(t)));
	snd(snd(t)) = depTypeExp(l,tvs,snd(snd(t)));
	/* it's premature to judge ambiguity (e.g. given functional deps)
	if (isAmbiguous(t)) {
	    ambigError(l,"type component",NIL,t);
	}
	*/
    } else {
	t = depTypeExp(l,tvs,t);
    }

    if (isNull(nfr)) {
	return t;
    }

    take(ntvs,tvs);
    return mkPolyType(nfr,t);
}

static Type local depTypeExp(Int line, List tyvars, Type type)
{
    switch (whatIs(type)) {
	case AP		: fst(type) = depTypeExp(line,tyvars,fst(type));
			  snd(type) = depTypeExp(line,tyvars,snd(type));
			  break;

	case VARIDCELL	: return depTypeVar(line,tyvars,textOf(type));

	case QUALIDENT  : if (isQVar(type)) {
			      ERRMSG(line) "Qualified type variables not allowed"
			      EEND;
			  }
			  /* deliberate fall through */
	case CONIDCELL  : {   Tycon tc = findQualTycon(type);
			      if (isNull(tc)) {
				  ERRMSG(line)
				      "Undefined type constructor \"%s\"",
				      identToStr(type)
				  EEND;
			      }
			      if ( whatIs(type) != QUALIDENT ) {
				  checkTyconAmbig(line,tycon(tc).text,tc);
			      }
			      if (cellIsMember(tc,tyconDefns) &&
				  !cellIsMember(tc,tcDeps)) {
				  tcDeps = cons(tc,tcDeps);
			      }
			      return tc;
			  }

	case TYCON	:
	case TUPLE	: break;

	default		: internal("depTypeExp");
    }
    return type;
}

static Type local depTypeVar(Int line, List tyvars, Text tv)
{
    Int offset = 0;
    Int found  = (-1);

    for (; nonNull(tyvars); offset++) {
	if (tv==textOf(hd(tyvars))) {
	    found = offset;
	}
	tyvars = tl(tyvars);
    }
    if (found<0) {
	Cell vt = findBtyvs(tv);
	if (nonNull(vt)) {
	    return fst(vt);
	}
	ERRMSG(line) "Undefined type variable \"%s\"", textToStr(tv)
	EEND;
    }
    return mkOffset(found);
}

static List local checkQuantVars(Int line, List vs, List tvs, Cell body)
            /* variables to quantify over	   */
             /* variables already in scope	   */
{				/* type/constr for scope of vars   */
    if (nonNull(vs)) {
	List bvs = typeVarsIn(body,NIL,NIL,NIL);
	List us  = vs;
	for (; nonNull(us); us=tl(us)) {
	    Text u = textOf(hd(us));
	    if (varIsMember(u,tl(us))) {
		ERRMSG(line) "Repeated quantified variable %s",
			     textToStr(u)
		EEND;
	    }
	    if (!varIsMember(u,bvs)) {
		ERRMSG(line) "Locally quantified variable %s is not used",
			     textToStr(u)
		EEND;
	    }
	}
	tvs = appendOnto(tvs,vs);
    }
    return tvs;
}

/* --------------------------------------------------------------------------
 * Check for ambiguous types:
 * A type  Preds => type  is ambiguous if not (TV(P) `subset` TV(type))
 * ------------------------------------------------------------------------*/

static List local offsetTyvarsIn(Type t, List vs)	/* add list of offset tyvars in t  */
            /* to list vs			   */
{
    switch (whatIs(t)) {
	case AP	      : return offsetTyvarsIn(fun(t),
				offsetTyvarsIn(arg(t),vs));

	case OFFSET   : if (cellIsMember(t,vs))
			    return vs;
			else
			    return cons(t,vs);

	case QUAL     : return offsetTyvarsIn(snd(t),vs);

	case POLYTYPE : return offsetTyvarsIn(monotypeOf(t),vs);
			/* slightly inaccurate, but won't matter here	   */

	case EXIST    :
	case RANK2    : return offsetTyvarsIn(snd(snd(t)),vs);

	default	      : return vs;
    }
}

List zonkTyvarsIn(Type t, List vs)
{
    switch (whatIs(t)) {
	case AP	      : return zonkTyvarsIn(fun(t),
			         zonkTyvarsIn(arg(t),vs));

	case INTCELL  : if (cellIsMember(t,vs))
			    return vs;
			else
			    return cons(t,vs);

	/* this case will lead to a type error --
	   much better than reporting an internal error ;-) */
	/* case OFFSET   : internal("zonkTyvarsIn"); */

	default	      : return vs;
    }
}

static List local otvars(Cell pi, List os)		/* os is a list of offsets that	   */
            /* refer to the arguments of pi;   */
{				/* find list of offsets in those   */
    List us = NIL;			/* positions			   */
    for (; nonNull(os); os=tl(os)) {
	us = offsetTyvarsIn(nthArg(offsetOf(hd(os)),pi),us);
    }
    return us;
}

static List local otvarsZonk(Cell pi, List os, Int o)	/* same as above, but zonks	   */
{
    List us = NIL;
    for (; nonNull(os); os=tl(os)) {
        Type t = zonkType(nthArg(offsetOf(hd(os)),pi),o);
	us = zonkTyvarsIn(t,us);
    }
    return us;
}

static List local odiff(List us, List vs)
{
    while (nonNull(us) && cellIsMember(hd(us),vs)) {
	us = tl(us);
    }
    return us;
}

static Bool local osubset(List us, List vs)	/* Determine whether us is subset  */
{				/* of vs			   */
    while (nonNull(us) && cellIsMember(hd(us),vs)) {
	us = tl(us);
    }
    return isNull(us);
}

List oclose(List fds, List vs)	/* Compute closure of vs wrt to fds*/
{
    Bool changed = TRUE;
    while (changed) {
	List fds1 = NIL;
	changed = FALSE;
        while (nonNull(fds)) {
	    Cell fd   = hd(fds);
	    List next = tl(fds);
	    if (osubset(fst(fd),vs)) {	/* Test if fd applies		   */
		List os = snd(fd);
		for (; nonNull(os); os=tl(os)) {
		    if (!cellIsMember(hd(os),vs)) {
			vs      = cons(hd(os),vs);
			changed = TRUE;
		    }
		}
	    } else {			/* Didn't apply this time, so keep */
		tl(fds) = fds1;
		fds1    = fds;
	    }
	    fds = next;
	}
	fds = fds1;
    }
    return vs;
}

Bool isAmbiguous(Type type)			/* Determine whether type is	   */
{				/* ambiguous 			   */
    if (isPolyType(type)) {
	type = monotypeOf(type);
    }
    if (isQualType(type)) {		/* only qualified types can be	   */
	List ps   = fst(snd(type));	/* ambiguous			   */
	List tvps = offsetTyvarsIn(ps,NIL);
	List tvts = offsetTyvarsIn(snd(snd(type)),NIL);
    	List fds  = calcFunDeps(ps);

	tvts = oclose(fds,tvts);	/* Close tvts under fds		   */
	return !osubset(tvps,tvts);
    }
    return FALSE;
}

List calcFunDeps(List ps)
{
    List fds  = NIL;
    for (; nonNull(ps); ps=tl(ps)) {/* Calc functional dependencies	   */
	Cell pi = hd(ps);
	Cell c  = getHead(pi);
	if (isClass(c)) {
	    List xfs = cclass(c).xfds;
	    for (; nonNull(xfs); xfs=tl(xfs)) {
		List fs = snd(hd(xfs));
		for (; nonNull(fs); fs=tl(fs)) {
		    fds = cons(pair(otvars(pi,fst(hd(fs))),
				    otvars(pi,snd(hd(fs)))),fds);
		}
	    }
	}
    }
    return fds;
}

List calcFunDepsPreds(List ps)
{
    List fds  = NIL;
    for (; nonNull(ps); ps=tl(ps)) {/* Calc functional dependencies	   */
	Cell pi3 = hd(ps);
	Cell pi = fst3(pi3);
	Cell c  = getHead(pi);
	Int o = intOf(snd3(pi3));
	if (isClass(c)) {
	    List xfs = cclass(c).xfds;
	    for (; nonNull(xfs); xfs=tl(xfs)) {
		List fs = snd(hd(xfs));
		for (; nonNull(fs); fs=tl(fs)) {
		    fds = cons(pair(otvarsZonk(pi,fst(hd(fs)),o),
				    otvarsZonk(pi,snd(hd(fs)),o)),fds);
		}
	    }
	}
    }
    return fds;
}

Void ambigError(Int line, String where, Cell e, Type type)	/* produce error message for	   */
                /* ambiguity			   */
{
    ERRMSG(line) "Ambiguous type signature in %s", where ETHEN
    ERRTEXT      "\n*** ambiguous type : " ETHEN ERRTYPE(type);
    if (nonNull(e)) {
	ERRTEXT  "\n*** assigned to    : " ETHEN ERREXPR(e);
    }
    ERRTEXT      "\n"
    EEND;
}

/* --------------------------------------------------------------------------
 * Kind inference for simple types:
 * ------------------------------------------------------------------------*/

static Void local kindConstr(Int line, Int alpha, Int m, Cell c)
              /* Determine kind of constructor   */
{
    assertType(c);
    Cell h = getHead(c);
    Int  n = argCount;

    switch (whatIs(h)) {
	case POLYTYPE : if (n!=0) {
			    internal("kindConstr1");
			} else {
			    static String pt = "polymorphic type";
			    Type  t  = dropRank1(c,alpha,m);
			    Kinds ks = polySigOf(t);
			    Int   m1 = 0;
			    Int   beta;
			    for (; isAp(ks); ks=tl(ks)) {
				m1++;
			    }
			    beta	= newKindvars(m1);
			    unkindTypes = cons(pair(mkInt(beta),t),unkindTypes);
			    checkKind(line,beta,m1,monotypeOf(t),NIL,pt,STAR,0);
			}
			return;

	case CDICTS   :
	case QUAL     : if (n!=0) {
			    internal("kindConstr2");
			}
			map3Proc(kindPred,line,alpha,m,fst(snd(c)));
			kindConstr(line,alpha,m,snd(snd(c)));
			return;

	case EXIST    :
	case RANK2    : kindConstr(line,alpha,m,snd(snd(c)));
			return;


	case TYCON    : if (isSynonym(h) && n<tycon(h).arity) {
			    ERRMSG(line)
			      "Not enough arguments for type synonym \"%s\"",
			      textToStr(tycon(h).text)
			    EEND;
			}
			break;
	default       : break;
    }

    if (n==0) {				/* trivial case, no arguments	   */
	typeIs = kindAtom(alpha,c);
    } else {				/* non-trivial application	   */
	static String app = "constructor application";
	Cell   a = c;
	Int    i;
	Kind   k;
	Int    beta;

	varKind(n);
	beta   = typeOff;
	k      = typeIs;

	typeIs = kindAtom(alpha,h);	/* h  :: v1 -> ... -> vn -> w	   */
	shouldKind(line,h,c,app,k,beta);

	for (i=n; i>0; --i) {		/* ci :: vi for each 1 <- 1..n	   */
	    checkKind(line,alpha,m,arg(a),c,app,aVar,beta+i-1);
	    a = fun(a);
	}
	tyvarType(beta+n);		/* inferred kind is w		   */
    }
}

static Kind local kindAtom(Int alpha, Cell c)	/* Find kind of atomic constructor */
{
    switch (whatIs(c)) {
	case TUPLE     : return simpleKind(tupleOf(c));	/*(,)::* -> * -> * */
	case OFFSET    : return mkInt(alpha+offsetOf(c));
	case TYCON     : return tycon(c).kind;
	case INTCELL   : return c;
	case VARIDCELL :
	case VAROPCELL : {   Cell vt = findBtyvs(textOf(c));
			     if (nonNull(vt)) {
				 return snd(vt);
			     }
			 }
	default        : break;
    }
    internal("kindAtom");
    return STAR;/* not reached */
}

static Void local kindPred(Int l, Int alpha, Int m, Cell pi)/* Check kinds of arguments in pred*/
{
    assertPred(pi);
    {   static String predicate = "class constraint";
	Class c  = getHead(pi);
	List  as = getArgs(pi);
	Kinds ks = cclass(c).kinds;

	while (nonNull(ks)) {
	    checkKind(l,alpha,m,hd(as),NIL,predicate,hd(ks),0);
	    ks = tl(ks);
	    as = tl(as);
	}
    }
}

static Void local kindType(Int line, String wh, Type type)/* check that (poss qualified) type*/
                /* is well-kinded		   */
{
    assertType(type);
    checkKind(line,0,0,type,NIL,wh,STAR,0);
}

static Void local fixKinds() {		/* add kind annotations to types   */
    for (; nonNull(unkindTypes); unkindTypes=tl(unkindTypes)) {
	Pair pr   = hd(unkindTypes);
	Int  beta = intOf(fst(pr));
	Cell qts  = polySigOf(snd(pr));
	for (;;) {
	    if (isNull(hd(qts))) {
		hd(qts) = copyKindvar(beta++);
	    } else {
		internal("fixKinds");
	    }
	    if (nonNull(tl(qts))) {
		qts = tl(qts);
	    } else {
		tl(qts) = STAR;
		break;
	    }
	}
    }
}

/* --------------------------------------------------------------------------
 * Kind checking of groups of type constructors and classes:
 * ------------------------------------------------------------------------*/

static Void local kindTCGroup(List tcs)	/* find kinds for mutually rec. gp */
{				/* of tycons and classes	   */
    emptySubstitution();
    unkindTypes = NIL;
    mapProc(initTCKind,tcs);
    mapProc(kindTC,tcs);
    mapProc(genTC,tcs);
    fixKinds();
    emptySubstitution();
}

static Void local initTCKind(Cell c)		/* build initial kind/arity for	c  */
{
    if (isTycon(c)) {			/* Initial kind of tycon is:	   */
	Int beta = newKindvars(1);	/*    v1 -> ... -> vn -> vn+1	   */
	varKind(tycon(c).arity);	/* where n is the arity of c.	   */
	bindTv(beta,typeIs,typeOff);	/* For data definitions, vn+1 == * */
	switch (whatIs(tycon(c).what)) {
	    case NEWTYPE  :
	    case DATATYPE : bindTv(typeOff+tycon(c).arity,STAR,0);
	    default       : break;
	}
	tycon(c).kind = mkInt(beta);
    } else {
	Int n    = cclass(c).arity;
	Int beta = newKindvars(n);
	cclass(c).kinds = NIL;
	while (n>0) {
	    n--;
	    cclass(c).kinds = pair(mkInt(beta+n),cclass(c).kinds);
	}
    }
}

static Void local kindTC(Cell c)		/* check each part of a tycon/class*/
{				/* is well-kinded		   */
    if (isTycon(c)) {
	static String cfun = "data constructor";
	static String tsyn = "synonym declaration";
	Int line = tycon(c).line;
	Int beta = tyvar(intOf(tycon(c).kind))->offs;
	Int m    = tycon(c).arity;
	switch (whatIs(tycon(c).what)) {
	    case NEWTYPE     :
	    case DATATYPE    : {   List cs = tycon(c).defn;
				   if (isQualType(cs)) {
				       map3Proc(kindPred,line,beta,m,
								fst(snd(cs)));
				       tycon(c).defn = cs = snd(snd(cs));
				   }
				   for (; hasCfun(cs); cs=tl(cs)) {
				       kindType(line,cfun,name(hd(cs)).type);
				   }
				   break;
			       }

	    default          : checkKind(line,beta,m,tycon(c).defn,NIL,
							tsyn,aVar,beta+m);
	}
    }
    else {				/* scan type exprs in class defn to*/
	List ms   = fst(cclass(c).members);
	Int  m    = cclass(c).arity;	/* determine the class signature   */
	Int  beta = newKindvars(length(cclass(c).tyvars));
	kindPred(cclass(c).line,beta,m,cclass(c).head);
	map3Proc(kindPred,cclass(c).line,beta,m,cclass(c).supers);
	for (; nonNull(ms); ms=tl(ms)) {
	    Int  line = intOf(fst3(hd(ms)));
	    Type type = thd3(hd(ms));
	    kindType(line,"member function type signature",type);
	}
    }
}

static Void local genTC(Cell c)		/* generalise kind inferred for	   */
{				/* given tycon/class		   */
    if (isTycon(c)) {
	tycon(c).kind = copyKindvar(intOf(tycon(c).kind));
    } else {
	Kinds ks = cclass(c).kinds;
	for (; nonNull(ks); ks=tl(ks)) {
	    hd(ks) = copyKindvar(intOf(hd(ks)));
	}
    }
}

/* --------------------------------------------------------------------------
 * Static analysis of instance declarations:
 *
 * The first part of the static analysis is performed as the declarations
 * are read during parsing:
 * - make new entry in instance table
 * - record line number of declaration
 * - build list of instances defined in current script for use in later
 *   stages of static analysis.
 * ------------------------------------------------------------------------*/

Void instDefn(Int line, Cell head, List ms)	       /* process new instance definition  */
                    /* definition line number	   */
                    /* inst header :: (context,Class)   */
{			       /* instance members		   */
    Inst nw             = newInst();
    inst(nw).line       = line;
    inst(nw).specifics  = fst(head);
    inst(nw).head	= snd(head);
    inst(nw).implements = ms;
    instDefns           = cons(nw,instDefns);
}

/* --------------------------------------------------------------------------
 * Further static analysis of instance declarations:
 *
 * Makes the following checks:
 * - Class part of header has form C (T a1 ... an) where C is a known
 *   class, and T is a known datatype constructor (or restricted synonym),
 *   and there is no previous C-T instance, and (T a1 ... an) has a kind
 *   appropriate for the class C.
 * - Each element of context is a valid class expression, with type vars
 *   drawn from a1, ..., an.
 * - All bindings are function bindings
 * - All bindings define member functions for class C
 * - Arrange bindings into appropriate order for member list
 * - No top level type signature declarations
 * ------------------------------------------------------------------------*/

Bool allowOverlap = FALSE;		/* TRUE => allow overlapping insts */
Bool allowUnsafeOverlap = FALSE;	/* TRUE => in addition, allow      */
					/* potentially inconsistent        */
					/* overlapping instances           */
Name nameListMonad = NIL;		/* builder function for List Monad */

static Void local checkInstDefn(Inst in)	/* Validate instance declaration   */
{
    Int  line   = inst(in).line;
    List tyvars = typeVarsIn(inst(in).head,NIL,NIL,NIL);
    List tvps = NIL, tvts = NIL;
    List fds = NIL;

    /* add in the tyvars from the `specifics' so that we don't
       prematurely complain about undefined tyvars */
    tyvars = typeVarsIn(inst(in).specifics,NIL,NIL,tyvars);
    inst(in).head = depPredExp(line,tyvars,inst(in).head);


    map2Over(depPredExp,line,tyvars,inst(in).specifics);

    /* OK, now we start over, and test for ambiguity */
    tvts = offsetTyvarsIn(inst(in).head,NIL);
    tvps = offsetTyvarsIn(inst(in).specifics,NIL);
    fds  = calcFunDeps(inst(in).specifics);
    tvts = oclose(fds,tvts);
    tvts = odiff(tvps,tvts);
    if (!isNull(tvts)) {
	ERRMSG(line) "Ambiguous type variable \"%s\"",
	  textToStr(textOf(nth(offsetOf(hd(tvts)),tyvars)))
	EEND;
    }

    inst(in).numSpecifics = length(inst(in).specifics);
    inst(in).c            = getHead(inst(in).head);
    if (!isClass(inst(in).c)) {
	ERRMSG(line) "Illegal predicate in instance declaration"
	EEND;
    }

    /* should this be over xfds? */
    if (nonNull(cclass(inst(in).c).fds)) {
        List fds = cclass(inst(in).c).fds;
        for (; nonNull(fds); fds=tl(fds)) {
            List as = otvars(inst(in).head, fst(hd(fds)));
            List bs = otvars(inst(in).head, snd(hd(fds)));
	    List fs = calcFunDeps(inst(in).specifics);
	    as = oclose(fs,as);
            if (!osubset(bs,as)) {
		ERRMSG(inst(in).line)
		   "Instance is more general than a dependency allows"
		ETHEN
		ERRTEXT "\n*** Instance         : "
		ETHEN ERRPRED(inst(in).head);
		ERRTEXT "\n*** For class        : "
		ETHEN ERRPRED(cclass(inst(in).c).head);
		ERRTEXT "\n*** Under dependency : "
		ETHEN ERRFD(hd(fds));
		ERRTEXT "\n"
		EEND;
            }
        }
    }

    kindInst(in,length(tyvars));
    insertInst(in);

    if (nonNull(extractSigdecls(inst(in).implements))) {
	ERRMSG(line)
	  "Type signature declarations not permitted in instance declaration"
	EEND;
    }
    if (nonNull(extractFixdecls(inst(in).implements))) {
	ERRMSG(line)
	  "Fixity declarations not permitted in instance declaration"
	EEND;
    }
    inst(in).implements = classBindings("instance",
					inst(in).c,
					extractBindings(inst(in).implements));
    inst(in).builder    = newInstImp(in);
    if (!preludeLoaded && isNull(nameListMonad) && isAp(inst(in).head)
	&& fun(inst(in).head)==classMonad && arg(inst(in).head)==typeList) {
	nameListMonad = inst(in).builder;
    }
}

static Void local insertInst(Inst in)	/* Insert instance into class	   */
{
    Class c    = inst(in).c;
    List  ins  = cclass(c).instances;
    List  prev = NIL;

    if (nonNull(cclass(c).fds)) {	/* Check for conflicts with fds	   */
	List ins1 = cclass(c).instances;
	for (; nonNull(ins1); ins1=tl(ins1)) {
	    List fds = cclass(c).fds;
	    substitution(RESET);
	    for (; nonNull(fds); fds=tl(fds)) {
		Int  alpha = newKindedVars(inst(in).kinds);
		Int  beta  = newKindedVars(inst(hd(ins1)).kinds);
		List as    = fst(hd(fds));
		Bool same  = TRUE;
		for (; same && nonNull(as); as=tl(as)) {
		    Int n = offsetOf(hd(as));
		    same &= unify(nthArg(n,inst(in).head),alpha,
				  nthArg(n,inst(hd(ins1)).head),beta);
		}
		if (isNull(as) && same) {
		    for (as=snd(hd(fds)); same && nonNull(as); as=tl(as)) {
			Int n = offsetOf(hd(as));
			same &= sameType(nthArg(n,inst(in).head),alpha,
					 nthArg(n,inst(hd(ins1)).head),beta);
		    }
		    if (!same) {
			ERRMSG(inst(in).line)
			   "Instances are not consistent with dependencies"
			ETHEN
			ERRTEXT "\n*** This instance    : "
			ETHEN ERRPRED(inst(in).head);
			ERRTEXT "\n*** Conflicts with   : "
			ETHEN ERRPRED(inst(hd(ins1)).head);
			ERRTEXT "\n*** For class        : "
			ETHEN ERRPRED(cclass(c).head);
			ERRTEXT "\n*** Under dependency : "
			ETHEN ERRFD(hd(fds));
			ERRTEXT "\n"
			EEND;
		    }
		}
	    }
	}
    }

    substitution(RESET);
    while (nonNull(ins)) {		/* Look for overlap w/ other insts */
	Int alpha = newKindedVars(inst(in).kinds);
	Int beta  = newKindedVars(inst(hd(ins)).kinds);
	if (unifyPred(inst(in).head,alpha,inst(hd(ins)).head,beta)) {
	    Cell pi  = copyPred(inst(in).head,alpha);
	    if (allowOverlap || allowUnsafeOverlap) {
		Bool bef = instCompare(in,hd(ins));
		Bool aft = instCompare(hd(ins),in);
		if (bef && !aft) {	/* in comes strictly before hd(ins)*/
		    break;
		}
		if (aft && !bef) {	/* in comes strictly after hd(ins) */
		    prev = ins;
		    ins  = tl(ins);
		    continue;
		}
	    }
		ERRMSG(inst(in).line) "Overlapping instances for class \"%s\"",
				      textToStr(cclass(c).text)
		ETHEN
		ERRTEXT "\n*** This instance   : " ETHEN ERRPRED(inst(in).head);
		ERRTEXT "\n*** Overlaps with   : " ETHEN
						   ERRPRED(inst(hd(ins)).head);
		ERRTEXT "\n*** Common instance : " ETHEN
						   ERRPRED(pi);
		ERRTEXT "\n"
		EEND;
	}
	prev = ins;			/* No overlap detected, so move on */
	ins  = tl(ins);			/* to next instance		   */
    }
    substitution(RESET);

    if (nonNull(prev)) {		/* Insert instance at this point   */
	tl(prev) = cons(in,ins);
    } else {
	cclass(c).instances = cons(in,ins);
    }
}

static Bool local instCompare(Inst ia, Inst ib)	/* See if ia is an instance of ib  */
{
    Int alpha = newKindedVars(inst(ia).kinds);
    Int beta  = newKindedVars(inst(ib).kinds);
    return matchPred(inst(ia).head,alpha,inst(ib).head,beta);
}

static Name local newInstImp(Inst in)	/* Make definition for inst builder*/
{
    Name b         = newName(inventText(),in);
    name(b).line   = inst(in).line;
    name(b).arity  = inst(in).numSpecifics;
    name(b).number = DFUNNAME;
    return b;
}

/* --------------------------------------------------------------------------
 * Kind checking of instance declaration headers:
 * ------------------------------------------------------------------------*/

static Void local kindInst(Inst in, Int freedom)	/* check predicates in instance    */
{
    Int beta;

    emptySubstitution();
    beta = newKindvars(freedom);
    kindPred(inst(in).line,beta,freedom,inst(in).head);
    if (whatIs(inst(in).specifics)!=DERIVE) {
	map3Proc(kindPred,inst(in).line,beta,freedom,inst(in).specifics);
    }
    for (inst(in).kinds = NIL; 0<freedom--; ) {
	inst(in).kinds = cons(copyKindvar(beta+freedom),inst(in).kinds);
    }
    emptySubstitution();
}

/* --------------------------------------------------------------------------
 * Process derived instance requests:
 * ------------------------------------------------------------------------*/

static List derivedInsts;		/* list of derived instances	   */

static Void local checkDerive(Tycon t, List p, List ts, Cell ct)/* verify derived instance request */
            /* for tycon t, with explicit	   */
            /* context p, component types ts   */
             /* and named class ct		   */
{
    Int   line = tycon(t).line;
    Class c    = findQualClass(ct);
    if (isNull(c)) {
	ERRMSG(line) "Unknown class \"%s\" in derived instance",
		     identToStr(ct)
	EEND;
    }
    addDerInst(line,c,p,dupList(ts),t,tycon(t).arity);
}

static Void local addDerInst(Int line, Class c, List p, List cts, Type t, Int a)	/* Add a derived instance  */
{
    Inst in;
    Cell head = t;				/* Build instance head	   */
    Int  i    = 0;

    for (; i<a; i++) {
	head = ap(head,mkOffset(i));
    }
    head = ap(c,head);

    in		        = newInst();
    inst(in).c          = c;
    inst(in).line       = line;
    inst(in).head       = head;
    inst(in).specifics  = ap(DERIVE,pair(dupList(p),cts));
    inst(in).implements = NIL;
    inst(in).kinds      = mkInt(a);
    derivedInsts        = cons(in,derivedInsts);
}

Void addTupInst(Class c, Int n)			/* Request derived instance of c   */
            /* for mkTuple(n) constructor	   */
{
    Int  m   = n;
    List cts = NIL;
    while (0<m--) {
	cts = cons(mkOffset(m),cts);
    }
    cts = rev(cts);
    addDerInst(0,c,NIL,cts,mkTuple(n),n);
}


/* --------------------------------------------------------------------------
 * Calculation of contexts for derived instances:
 *
 * Allowing arbitrary types to appear in contexts makes it rather harder
 * to decide what the context for a derived instance should be.  For
 * example, given:
 *
 *    data T a = MkT [a] deriving Show,
 *
 * we could have either of the following:
 *
 *    instance (Show [a]) => Show (T a) where ...
 *    instance (Show a) => Show (T a) where ...
 *
 * (assuming, of course, that instance (Show a) => Show [a]).  For now, we
 * choose to reduce contexts in the hope of detecting errors at an earlier
 * stage---in contrast with value definitions, there is no way for a user
 * to provide something analogous to a `type signature' by which they might
 * be able to control this behaviour themselves.  We eliminate tautological
 * predicates, but only allow predicates to appear in the final result if
 * they have at least one argument with a variable at its head.
 *
 * In general, we have to deal with mutually recursive instance declarations.
 * We find a solution in the obvious way by iterating to find a fixed point.
 * Of course, without restrictions on the form of instance declarations, we
 * cannot be sure that this will always terminate!
 *
 * For each instance we maintain a pair of the form DERIVE (ctxt,ps).
 * Ctxt is a list giving the parts of the context that have been produced
 * so far in the form of predicate skeletons.  During the calculation of
 * derived instances, we attach a dummy NIL value to the end of the list
 * which acts as a kind of `variable': other parts of the system maintain
 * pointers to this variable, and use it to detect when the context has
 * been extended with new elements.  Meanwhile, ps is a list containing
 * predicates (pi,o) together with (delayed) substitutions of the form
 * (o,xs) where o is an offset and xs is one of the context variables
 * described above, which may have been partially instantiated.
 * ------------------------------------------------------------------------*/

static Bool instsChanged;

static Void local deriveContexts(List is)	/* Calc contexts for derived insts */
{
    emptySubstitution();
    mapProc(initDerInst,is);		/* Prepare derived instances	   */

    do {				/* Main calculation of contexts	   */
	instsChanged = FALSE;
	mapProc(calcInstPreds,is);
    } while (instsChanged);

    mapProc(tidyDerInst,is);		/* Tidy up results		   */
}

static Void local initDerInst(Inst in)	/* Prepare instance for calculation*/
{				/* of derived instance context	   */
    Cell spcs = inst(in).specifics;
    Int  beta = newKindedVars(inst(in).kinds);
    if (whatIs(spcs)!=DERIVE) {
	internal("initDerInst");
    }
    fst(snd(spcs)) = appendOnto(fst(snd(spcs)),singleton(NIL));
    for (spcs=snd(snd(spcs)); nonNull(spcs); spcs=tl(spcs)) {
	hd(spcs) = ap2(inst(in).c,hd(spcs),mkInt(beta));
    }
    inst(in).numSpecifics = beta;

}

static Void local calcInstPreds(Inst in)	/* Calculate next approximation	   */
{				/* of the context for a derived	   */
    List retain = NIL;			/* instance			   */
    List ps     = snd(snd(inst(in).specifics));
    List spcs   = fst(snd(inst(in).specifics));
    Int  beta   = inst(in).numSpecifics;
    Int  its    = 1;
    Int  factor = 1+length(ps);

    while (nonNull(ps)) {
	Cell p = hd(ps);
	ps     = tl(ps);

	if (its++ >= factor*cutoff) {
	    Cell bpi = inst(in).head;
	    ERRMSG(inst(in).line) "\n*** Cannot derive " ETHEN ERRPRED(bpi);
	    ERRTEXT " after %d iterations.", its-1   ETHEN
	    ERRTEXT
		"\n*** This may indicate that the problem is undecidable.  However,\n"
	    ETHEN ERRTEXT
		"*** you may still try to increase the cutoff limit using the -c\n"
	    ETHEN ERRTEXT
		"*** option and then try again.  (The current setting is -c%d)\n",
		cutoff
	    EEND;
	}
	if (isInt(fst(p))) {			/* Delayed substitution?   */
	    List qs = snd(p);
	    for (; nonNull(hd(qs)); qs=tl(qs)) {
		ps = cons(pair(hd(qs),fst(p)),ps);
	    }
	    retain = cons(pair(fst(p),qs),retain);
	}
	else {					/* Class predicate	   */
	    Cell pi  = fst(p);
	    Int  o   = intOf(snd(p));
	    Inst in1 = findInstFor(pi,o);
	    if (nonNull(in1)) {
		List qs  = inst(in1).specifics;
		Int  off = mkInt(typeOff);
		if (whatIs(qs)==DERIVE) {	/* Still being derived	   */
		    for (qs=fst(snd(qs)); nonNull(hd(qs)); qs=tl(qs)) {
			ps = cons(pair(hd(qs),off),ps);
		    }
		    retain = cons(pair(off,qs),retain);
		} else {			/* Previously def'd inst   */
		    for (; nonNull(qs); qs=tl(qs)) {
			ps = cons(pair(hd(qs),off),ps);
		    }
		}
	    } else {				/* No matching instance	   */
		Cell qi = pi;
		while (isAp(qi) && isOffset(getDerefHead(arg(qi),o))) {
		    qi = fun(qi);
		}
		if (isAp(qi)) {
		    Cell bpi = inst(in).head;
		    pi       = copyPred(pi,o);
		    ERRMSG(inst(in).line) "An instance of " ETHEN ERRPRED(pi);
		    ERRTEXT " is required to derive "       ETHEN ERRPRED(bpi);
		    ERRTEXT "\n"
		    EEND;
		} else {
		    maybeAddPred(pi,o,beta,spcs);
		}
	    }
	}
    }
    snd(snd(inst(in).specifics)) = retain;
}

static Void local maybeAddPred(Cell pi, Int o, Int beta, List ps)
            /* Add predicate pi to the list ps,*/
            /* setting the instsChanged flag if*/
              /* pi is not already a member and  */
{				/* using beta to adjust vars	   */
    Cell c = getHead(pi);
    for (; nonNull(ps); ps=tl(ps)) {
	if (isNull(hd(ps))) {		/* reached the `dummy' end of list?*/
	    hd(ps)       = copyAdj(pi,o,beta);
	    tl(ps)       = pair(NIL,NIL);
	    instsChanged = TRUE;
	    return;
	} else if (c==getHead(hd(ps)) && samePred(pi,o,hd(ps),beta)) {
	    return;
	}
    }
}

static Cell local copyAdj(Cell c, Int o, Int beta)	/* Copy (c,o), replacing vars with */
            /* offsets relative to beta.	   */
{
    switch (whatIs(c)) {
	case AP     : {   Cell l = copyAdj(fst(c),o,beta);
			  Cell r = copyAdj(snd(c),o,beta);
			  return ap(l,r);
		      }

	case OFFSET : {   Int   vn   = o+offsetOf(c);
			  Tyvar *tyv = tyvar(vn);
			  if (isBound(tyv)) {
			      return copyAdj(tyv->bound,tyv->offs,beta);
			  }
			  vn -= beta;
			  if (vn<0 || vn>=NUM_OFFSETS) {
			      internal("copyAdj");
			  }
			  return mkOffset(vn);
		      }
	default     : break;
    }
    return c;
}

static Void local tidyDerInst(Inst in)	/* Tidy up results of derived inst */
{				/* calculations			   */
    Int  o  = inst(in).numSpecifics;
    List ps = tl(rev(fst(snd(inst(in).specifics))));
    clearMarks();
    copyPred(inst(in).head,o);
    inst(in).specifics    = simpleContext(ps,o);
    inst(in).numSpecifics = length(inst(in).specifics);

}

/* --------------------------------------------------------------------------
 * Generate code for derived instances:
 * ------------------------------------------------------------------------*/

static Void local addDerivImp(Inst in)
{
    List  imp = NIL;
    Type  t   = getHead(arg(inst(in).head));
    Class c   = inst(in).c;
    if (c==classEq) {
	imp = deriveEq(t);
    } else if (c==classOrd) {
	imp = deriveOrd(t);
    } else if (c==classEnum) {
	imp = deriveEnum(t);
    } else if (c==classIx) {
	imp = deriveIx(t);
    } else if (c==classShow) {
	imp = deriveShow(t);
    } else if (c==classRead) {
	imp = deriveRead(t);
    } else if (c==classBounded) {
	imp = deriveBounded(t);
    } else {
	ERRMSG(inst(in).line) "Cannot derive instances of class \"%s\"",
			      textToStr(cclass(inst(in).c).text)
	EEND;
    }

    kindInst(in,intOf(inst(in).kinds));
    insertInst(in);
    inst(in).builder    = newInstImp(in);
    inst(in).implements = classBindings("derived instance",
					inst(in).c,
					imp);
}

static List diVars = NIL;		/* Acts as a cache of invented vars*/
static Int  diNum  = 0;

static List local getDiVars(Int n)		/* get list of at least n vars for */
{				/* derived instance generation	   */
    for (; diNum<n; diNum++) {
	diVars = cons(inventVar(),diVars);
    }
    return diVars;
}

static Cell local mkBind(String s, List alts)	/* make a binding for a variable   */
{
    return pair(mkVar(findText(s)),pair(NIL,alts));
}

static Cell local mkVarAlts(Int line, Cell r)	/* make alts for binding a var to  */
              /* a simple expression		   */
{
    return singleton(pair(NIL,pair(mkInt(line),r)));
}

/* --------------------------------------------------------------------------
 * Given a datatype:   data T a b = A a b | B Int | C  deriving (Eq, Ord)
 * The derived definitions of equality and ordering are given by:
 *
 *   A a b == A x y  =  a==x && b==y
 *   B a   == B x    =  a==x
 *   C     == C      =  True
 *   _     == _      =  False
 *
 *   compare (A a b) (A x y) =  primCompAux a x (compare b y)
 *   compare (B a)   (B x)   =  compare a x
 *   compare C       C       =  EQ
 *   compare a       x       =  cmpConstr a x
 *
 * In each case, the last line is only needed if there are multiple
 * constructors in the datatype definition.
 * ------------------------------------------------------------------------*/

#define ap2(f,x,y) ap(ap(f,x),y)

static List local deriveEq(Type t)		/* generate binding for derived == */
{				/* for some TUPLE or DATATYPE t	   */
    List alts = NIL;
    if (isTycon(t)) {			/* deal with type constrs	   */
	List cs = tycon(t).defn;
	for (; hasCfun(cs); cs=tl(cs)) {
	    alts = cons(mkAltEq(tycon(t).line,
				makeDPats2(hd(cs),userArity(hd(cs)))),
			alts);
	}
	if (cfunOf(hd(tycon(t).defn))!=0) {
	    alts = cons(pair(cons(WILDCARD,cons(WILDCARD,NIL)),
			     pair(mkInt(tycon(t).line),nameFalse)),alts);
	}
	alts = rev(alts);
    }
    else {				/* special case for tuples	   */
	alts = singleton(mkAltEq(0,makeDPats2(t,tupleOf(t))));
    }

    return singleton(mkBind("==",alts));
}

static Pair local mkAltEq(Int line, List pats)	/* make alt for an equation for == */
              /* using patterns in pats for lhs  */
{				/* arguments			   */
    Cell p = hd(pats);
    Cell q = hd(tl(pats));
    Cell e = nameTrue;

    if (isAp(p)) {
	e = ap2(nameEq,arg(p),arg(q));
	for (p=fun(p), q=fun(q); isAp(p); p=fun(p), q=fun(q)) {
	    e = ap2(nameAnd,ap2(nameEq,arg(p),arg(q)),e);
	}
    }
    return pair(pats,pair(mkInt(line),e));
}

static List local deriveOrd(Type t)		/* make binding for derived compare*/
{				/* for some TUPLE or DATATYPE t	   */
    List alts = NIL;
    if (isEnumType(t)) {		/* special case for enumerations   */
	alts = mkVarAlts(tycon(t).line,nameConCmp);
    } else if (isTycon(t)) {		/* deal with type constrs	   */
	List cs = tycon(t).defn;
	for (; hasCfun(cs); cs=tl(cs)) {
	    alts = cons(mkAltOrd(tycon(t).line,
				 makeDPats2(hd(cs),userArity(hd(cs)))),
			alts);
	}
	if (cfunOf(hd(tycon(t).defn))!=0) {
	    Cell u = inventVar();
	    Cell w = inventVar();
	    alts   = cons(pair(cons(u,singleton(w)),
			       pair(mkInt(tycon(t).line),
				    ap2(nameConCmp,u,w))),alts);
	}
	alts = rev(alts);
    } else {				/* special case for tuples	   */
	alts = singleton(mkAltOrd(0,makeDPats2(t,tupleOf(t))));
    }

    return singleton(mkBind("compare",alts));
}

static Pair local mkAltOrd(Int line, List pats)	/* make alt for eqn for compare	   */
              /* using patterns in pats for lhs  */
{				/* arguments			   */
    Cell p = hd(pats);
    Cell q = hd(tl(pats));
    Cell e = nameEQ;

    if (isAp(p)) {
	e = ap2(nameCompare,arg(p),arg(q));
	for (p=fun(p), q=fun(q); isAp(p); p=fun(p), q=fun(q)) {
	    e = ap(ap2(nameCompAux,arg(p),arg(q)),e);
	}
    }

    return pair(pats,pair(mkInt(line),e));
}

static List local makeDPats2(Cell h, Int n)	/* generate pattern list	   */
            /* by putting two new patterns with*/
{				/* head h and new var components   */
    List us = getDiVars(2*n);
    List vs = NIL;
    Cell p;
    Int  i;

    for (i=0, p=h; i<n; ++i) {		/* make first version of pattern   */
	p  = ap(p,hd(us));
	us = tl(us);
    }
    vs = cons(p,vs);

    for (i=0, p=h; i<n; ++i) {		/* make second version of pattern  */
	p  = ap(p,hd(us));
	us = tl(us);
    }
    return cons(p,vs);
}

/* --------------------------------------------------------------------------
 * Deriving Ix and Enum:
 * ------------------------------------------------------------------------*/

static List local deriveEnum(Tycon t)	/* Construct definition of enumeration	   */
{
    Int l = tycon(t).line;

    if (!isEnumType(t)) {
	ERRMSG(l) "Can only derive instances of Enum for enumeration types"
	EEND;
    }

    return cons(mkBind("toEnum",mkVarAlts(l,ap(nameEnToEn,hd(tycon(t).defn)))),
	    cons(mkBind("fromEnum",mkVarAlts(l,nameEnFrEn)),
	     cons(mkBind("enumFrom",mkVarAlts(l,nameEnFrom)),
	      cons(mkBind("enumFromTo",mkVarAlts(l,nameEnFrTo)),
	       cons(mkBind("enumFromThen",mkVarAlts(l,nameEnFrTh)),NIL)))));
}

static List local deriveIx(Tycon t)	/* Construct definition of indexing	   */
{
    if (isEnumType(t)) {	/* Definitions for enumerations		   */
	return cons(mkBind("range",mkVarAlts(tycon(t).line,nameEnRange)),
		cons(mkBind("index",mkVarAlts(tycon(t).line,nameEnIndex)),
		 cons(mkBind("inRange",mkVarAlts(tycon(t).line,nameEnInRng)),
		  NIL)));
    } else if (isTuple(t)) {	/* Definitions for product types	   */
	return mkIxBinds(0,t,tupleOf(t));
    } else if (isTycon(t) && cfunOf(hd(tycon(t).defn))==0) {
	return mkIxBinds(tycon(t).line,
			 hd(tycon(t).defn),
			 userArity(hd(tycon(t).defn)));
    }

    ERRMSG(tycon(t).line)
	"Can only derive instances of Ix for enumeration or product types"
    EEND;
    return NIL;/* NOTREACHED*/
}

static Bool local isEnumType(Tycon t)	/* Determine whether t is an enumeration   */
{			/* type (i.e. all constructors arity == 0) */
    if (isTycon(t) && (tycon(t).what==DATATYPE || tycon(t).what==NEWTYPE)) {
	List cs = tycon(t).defn;
	for (; hasCfun(cs); cs=tl(cs)) {
	    if (name(hd(cs)).arity!=0) {
		return FALSE;
	    }
	}
	addCfunTable(t);
	return TRUE;
    }
    return FALSE;
}

static List local mkIxBinds(Int line, Cell h, Int n)	/* build bindings for derived Ix on*/
              /* a product type		   */
{
    List vs   = getDiVars(3*n);
    Cell ls   = h;
    Cell us   = h;
    Cell is   = h;
    Cell js   = h;
    Cell pr   = NIL;
    Cell pats = NIL;
    Int  i;

    for (i=0; i<n; ++i, vs=tl(vs)) {	/* build three patterns for values */
	ls = ap(ls,hd(vs));		/* of the datatype concerned	   */
	us = ap(us,hd(vs=tl(vs)));
	is = ap(is,hd(vs=tl(vs)));
	js = ap(js,hd(vs));		/* ... and one expression	   */
    }
    pr   = ap2(mkTuple(2),ls,us);	/* Build (ls,us)		   */
    pats = cons(pr,cons(is,NIL));	/* Build [(ls,us),is]		   */

    return cons(prodRange(line,singleton(pr),ls,us,js),
		cons(prodIndex(line,pats,ls,us,is),
		     cons(prodInRange(line,pats,ls,us,is),NIL)));
}

static Cell local prodRange(Int line, List pats, Cell ls, Cell us, Cell is)
              /* Make definition of range for a  */
              /* product type			   */
{
    /* range :: (a,a) -> [a]
     * range (X a b c, X p q r)
     *   = [ X x y z | x <- range (a,p), y <- range (b,q), z <- range (c,r) ]
     */
    Cell is1 = is;
    List e   = NIL;
    for (; isAp(ls); ls=fun(ls), us=fun(us), is=fun(is)) {
	e = cons(ap(FROMQUAL,pair(arg(is),
				  ap(nameRange,ap2(mkTuple(2),
						   arg(ls),
						   arg(us))))),e);
    }
    e = ap(COMP,pair(is1,e));
    e = singleton(pair(pats,pair(mkInt(line),e)));
    return mkBind("range",e);
}

static Cell local prodIndex(Int line, List pats, Cell ls, Cell us, Cell is)
              /* Make definition of index for a  */
              /* product type			   */
{
    /* index :: (a,a) -> a -> Bool
     * index (X a b c, X p q r) (X x y z)
     *  = index (c,r) z + rangeSize (c,r) * (
     *     index (b,q) y + rangeSize (b,q) * (
     *      index (a,x) x))
     */
    List xs = NIL;
    Cell e  = NIL;
    for (; isAp(ls); ls=fun(ls), us=fun(us), is=fun(is)) {
	xs = cons(ap2(nameIndex,ap2(mkTuple(2),arg(ls),arg(us)),arg(is)),xs);
    }
    for (e=hd(xs); nonNull(xs=tl(xs));) {
	Cell x = hd(xs);
	e = ap2(namePlus,x,ap2(nameMult,ap(nameRangeSize,arg(fun(x))),e));
    }
    e = singleton(pair(pats,pair(mkInt(line),e)));
    return mkBind("index",e);
}

static Cell local prodInRange(Int line, List pats, Cell ls, Cell us, Cell is)
              /* Make definition of inRange for a*/
              /* product type			   */
{
    /* inRange :: (a,a) -> a -> Bool
     * inRange (X a b c, X p q r) (X x y z)
     *          = inRange (a,p) x && inRange (b,q) y && inRange (c,r) z
     */
    Cell e = ap2(nameInRange,ap2(mkTuple(2),arg(ls),arg(us)),arg(is));
    while (ls=fun(ls), us=fun(us), is=fun(is), isAp(ls)) {
	e = ap2(nameAnd,
		ap2(nameInRange,ap2(mkTuple(2),arg(ls),arg(us)),arg(is)),
		e);
    }
    e = singleton(pair(pats,pair(mkInt(line),e)));
    return mkBind("inRange",e);
}

/* --------------------------------------------------------------------------
 * Deriving Show:
 * ------------------------------------------------------------------------*/

static List local deriveShow(Tycon t)	/* Construct definition of text conversion */
{
    List alts = NIL;
    if (isTycon(t)) {			/* deal with type constrs	   */
	List cs = tycon(t).defn;
	for (; hasCfun(cs); cs=tl(cs)) {
	    alts = cons(mkAltShow(tycon(t).line,hd(cs),userArity(hd(cs))),
			alts);
	}
	alts = rev(alts);
    } else {				/* special case for tuples	   */
	alts = singleton(mkAltShow(0,t,tupleOf(t)));
    }

    return singleton(mkBind("showsPrec",alts));
}

static Cell local mkAltShow(Int line, Cell h, Int a)	/* make alt for showsPrec eqn	   */
{
    List vs   = getDiVars(a+1);
    Cell d    = hd(vs);
    Cell pat  = h;
    List pats = NIL;
    Int  i    = 0;
    for (vs=tl(vs); i<a; i++) {
	pat = ap(pat,hd(vs));
	vs  = tl(vs);
    }
    pats = cons(d,cons(pat,NIL));
    return pair(pats,pair(mkInt(line),showsPrecRhs(d,pat,a)));
}

#define APP_PREC  10 /* precedence of function application */
#define shows0    ap(nameShowsPrec,mkInt(0))
#define showsN(x) ap(nameShowsPrec,mkInt(x))
#define showsOP  ap(nameComp,consChar('('))
#define showsOB  ap(nameComp,consChar('{'))
#define showsCM  ap(nameComp,consChar(','))
#define showsSP  ap(nameComp,consChar(' '))
#define showsBQ  ap(nameComp,consChar('`'))
#define showsCP  consChar(')')
#define showsCB  consChar('}')

static Cell local showsPrecRhs(Cell d, Cell pat, Int a)	/* build a rhs for showsPrec for a */
                /* given pattern, pat		   */
{
    Cell h   = getHead(pat);
    List cfs = cfunSfuns;

    if (isTuple(h)) {
	/* To display a tuple:
	 *    showsPrec d (a,b,c,d) = showChar '(' . showsPrec 0 a .
	 *			      showChar ',' . showsPrec 0 b .
	 *			      showChar ',' . showsPrec 0 c .
	 *			      showChar ',' . showsPrec 0 d .
	 *			      showChar ')'
	 */
	Int  i   = tupleOf(h);
	Cell rhs = showsCP;
	for (; i>1; --i) {
	    rhs = ap(showsCM,ap2(nameComp,ap(shows0,arg(pat)),rhs));
	    pat = fun(pat);
	}
	return ap(showsOP,ap2(nameComp,ap(shows0,arg(pat)),rhs));
    }

    for (; nonNull(cfs) && h!=fst(hd(cfs)); cfs=tl(cfs)) {
    }
    if (nonNull(cfs)) {
	/* To display a value using record syntax:
	 *    showsPrec d C{x=e, y=f, z=g} =
	 *		showString "C"  . showString " {" .
	 *		showField "x" e . showString ", " .
	 *		showField "y" f . showString ", " .
	 *		showField "z" g . showChar '}'
	 *    showField lab val
	 *	= showString lab . showString " = " . shows val
	 */
	Cell rhs     = showsCB;
	List vs      = dupOnto(snd(hd(cfs)),NIL);
	if (isAp(pat)) {
	    for (;;) {
		rhs = ap2(nameComp,
			  ap2(nameShowField,
			      mkStr(textOf(hd(vs))),
			      arg(pat)),
			  rhs);
		pat = fun(pat);
		vs  = tl(vs);
		if (isAp(pat)) {
		    rhs = ap(showsCM,ap(showsSP,rhs));
		} else {
		    break;
		}
	    }
	}
	rhs = ap2(nameComp,ap(nameApp,mkStr(name(h).text)),
			ap(showsSP,ap(showsOB,rhs)));
	return rhs;
    }
    else if (a==0) {
	/* To display a nullary constructor:
	 *    showsPrec d Foo = showString "Foo"
	 */
	return ap(nameApp,mkStr(name(h).text));
    } else {
	Syntax s = syntaxOf(h);
	if (a==2 && assocOf(s)!=APPLIC) {
	    /* For a binary constructor with prec p:
	     * showsPrec d (a :* b) = showParen (d > p)
	     *				(showsPrec lp a . showChar ' ' .
	     *				 showsString s  . showChar ' ' .
	     *				 showsPrec rp b)
	     */
	    Int  p   = precOf(s);
	    Int  lp  = (p+1); /* how it was in Haskell from Day 1 until 11/4/2002:  (assocOf(s)==LEFT_ASS)  ? p : (p+1); */
	    Int  rp  = (p+1); /* ditto:  (assocOf(s)==RIGHT_ASS) ? p : (p+1); */
	    Cell rhs = ap(showsSP,ap2(nameShowsPrec,mkInt(rp),arg(pat)));
	    if (defaultSyntax(name(h).text)==APPLIC) {
		rhs = ap(showsBQ,
			 ap2(nameComp,
 			     ap(nameApp,mkStr(fixLitText(name(h).text))),
			     ap(showsBQ,rhs)));
	    } else {
		rhs = ap2(nameComp,
 			  ap(nameApp,mkStr(fixLitText(name(h).text))),rhs);
	    }

	    rhs = ap2(nameComp,
		      ap2(nameShowsPrec,mkInt(lp),arg(fun(pat))),
		      ap(showsSP,rhs));
	    rhs = ap2(nameShowParen,ap2(nameGt,d,mkInt(p)),rhs);
	    return rhs;
	}
	else {
	    /* To display a non-nullary constructor with applicative syntax:
	     *    showsPrec d (Foo x y) = showParen (d>APP_PREC)
	     *				   (showString "Foo" .
	     *				    showChar ' ' . showsPrec (APP_PREC+1) x .
	     *				    showChar ' ' . showsPrec (APP_PREC+1) y)
	     */
	    Cell rhs = ap(showsSP,ap(showsN(APP_PREC+1),arg(pat)));
	    for (pat=fun(pat); isAp(pat); pat=fun(pat)) {
		rhs = ap(showsSP,ap2(nameComp,ap(showsN(APP_PREC+1),arg(pat)),rhs));
	    }
	    rhs = ap2(nameComp,ap(nameApp,mkStr(name(h).text)),rhs);
	    rhs = ap2(nameShowParen,ap2(nameGt,d,mkInt(APP_PREC)),rhs);
	    return rhs;
	}
    }
}
#undef  showsN
#undef  shows0
#undef  showsOP
#undef  showsOB
#undef  showsCM
#undef  showsSP
#undef  showsBQ
#undef  showsCP
#undef  showsCB

/* --------------------------------------------------------------------------
 * Deriving Read:
 * ------------------------------------------------------------------------*/

#define Tuple2(f,s)      ap2(mkTuple(2),f,s)
#define Lex(r)           ap(nameLex,r)
#define ZFexp(h,q)       ap(FROMQUAL, pair(h,q))
#define ReadsPrec(n,e)   ap2(nameReadsPrec,n,e)
#define Lambda(v,e)      ap(LAMBDA,pair(v, pair(mkInt(0),e)))
#define ReadParen(a,b,c) ap(ap2(nameReadParen,a,b),c)
#define ReadField(f,s)   ap2(nameReadField,f,s)
#define GT(l,r)          ap2(nameGt,l,r)
#define Append(a,b)      ap2(nameApp,a,b)

/*  Construct the readsPrec function of the form:
 *
 *    readsPrec d r = (readParen (d>p1) (\r -> [ (C1 ...,s) | ... ]) r ++
 *                    (readParen (d>p2) (\r -> [ (C2 ...,s) | ... ]) r ++
 *                    ...
 *                    (readParen (d>pn) (\r -> [ (Cn ...,s) | ... ]) r) ... ))
 */
static List local deriveRead(Cell t)	/* construct definition of text reader	   */
{
    Cell alt  = NIL;
    Cell exp  = NIL;
    Cell d    = inventVar();
    Cell r    = inventVar();
    List pat  = cons(d,cons(r,NIL));
    Int  line = 0;

    if (isTycon(t)) {
	List cs = tycon(t).defn;
	List exps = NIL;
	for (; hasCfun(cs); cs=tl(cs)) {
	    exps = cons(mkReadCon(hd(cs),d,r),exps);
	}
	/* reverse concatenate list of subexpressions */
	exp = hd(exps);
	for (exps=tl(exps); nonNull(exps); exps=tl(exps)) {
	    exp = ap2(nameApp,hd(exps),exp);
	}
	line = tycon(t).line;
    }
    else { /* Tuples */
	exp = ap(mkReadTuple(t),r);
    }
    /* printExp(stdout,exp); putc('\n',stdout); */
    alt  = pair(pat,pair(mkInt(line),exp));
    return singleton(mkBind("readsPrec",singleton(alt)));
}

/* Generate an expression of the form:
 *
 *   readParen (d > p) <derived expression> r
 *
 * for a (non-tuple) constructor "con" of precedence "p".
 */

static Cell local mkReadCon(Name con, Cell d, Cell r) /* generate reader for a constructor */
{
    Cell exp = NIL;
    Int  p   = 0;
    Syntax s = syntaxOf(con);
    List cfs = cfunSfuns;

    for (; nonNull(cfs) && con!=fst(hd(cfs)); cfs=tl(cfs)) {
    }
    if (nonNull(cfs)) {
	exp = mkReadRecord(con,snd(hd(cfs)));
	return ReadParen(nameFalse, exp, r);
    }

    if (userArity(con)==2 && assocOf(s)!=APPLIC) {
	exp = mkReadInfix(con);
	p   = precOf(s);
    } else {
	exp = mkReadPrefix(con);
	p   = APP_PREC;
    }
    return ReadParen(userArity(con)==0 ? nameFalse : GT(d,mkInt(p)), exp, r);
}

/* Given an n-ary prefix constructor, generate a single lambda
 * expression, such that
 *
 *   data T ... = Constr a1 a2 .. an | ....
 *
 * derives
 *
 *   \ r -> [ (Constr t1 t2 ... tn, sn) | ("Constr",s0) <- lex r,
 *                                        (t1,s1) <- readsPrec (APP_PREC+1) s0,
 *                                        (t2,s2) <- readsPrec (APP_PREC+1) s1,
 *                                        ...,
 *                                        (tn,sn) <- readsPrec (APP_PREC+1) sn-1 ]
 *
 */
static Cell local mkReadPrefix(Cell con)    /* readsPrec for prefix constructor */
{
    Int  arity  = userArity(con);
    Cell cn     = mkStr(name(con).text);
    Cell r      = inventVar();
    Cell prev_s = inventVar();
    Cell exp    = con;
    List quals  = NIL;
    Int  i;

    /* build (reversed) list of qualifiers and constructor */
    quals = cons(ZFexp(Tuple2(cn,prev_s),Lex(r)),quals);
    for(i=0; i<arity; i++) {
	Cell t = inventVar();
	Cell s = inventVar();
	quals  = cons(ZFexp(Tuple2(t,s),ReadsPrec(mkInt(APP_PREC+1),prev_s)), quals);
	exp    = ap(exp,t);
	prev_s = s;
    }

    /* \r -> [ (exp, prev_s) | quals ] */
    return Lambda(singleton(r),ap(COMP,pair(Tuple2(exp, prev_s), rev(quals))));
}

/* Given a binary infix constructor of precedence p
 *
 *   ... | T1 `con` T2 | ...
 *
 * generate the lambda expression
 *
 *   \ r -> [ (u `con` v, s2) | (u,s0)     <- readsPrec lp r,
 *                              ("con",s1) <- lex s0,
 *                              (v,s2)     <- readsPrec rp s1 ]
 *
 * where lp and rp are either p or p+1 depending on associativity
 */
static Cell local mkReadInfix(Cell con)
{
    Syntax s  = syntaxOf(con);
    Int    p  = precOf(s);
    Int    lp = (p+1); /* how it was in Haskell from Day 1 until 11/4/2002: assocOf(s)==LEFT_ASS  ? p : (p+1); */
    Int    rp = (p+1); /* ditto: assocOf(s)==RIGHT_ASS ? p : (p+1); */
    Cell   cn = mkStr(name(con).text);
    Cell   r  = inventVar();
    Cell   s0 = inventVar();
    Cell   s1 = inventVar();
    Cell   s2 = inventVar();
    Cell   u  = inventVar();
    Cell   v  = inventVar();
    List quals = NIL;

    quals = cons(ZFexp(Tuple2(u, s0), ReadsPrec(mkInt(lp),r)),  quals);
    quals = cons(ZFexp(Tuple2(cn,s1), Lex(s0)),                 quals);
    quals = cons(ZFexp(Tuple2(v, s2), ReadsPrec(mkInt(rp),s1)), quals);

    return Lambda(singleton(r),
		  ap(COMP,pair(Tuple2(ap2(con,u,v),s2),rev(quals))));
}

/* Given the n-ary tuple constructor return a lambda expression:
 *
 *   \ r -> [ ((t1,t2,...tn),s(2n+1)) | ("(",s0)      <- lex r,
 *  					(t1, s1)      <- readsPrec 0 s0,
 *  					...
 *  					(",",s(2n-1)) <- lex s(2n-2),
 *  					(tn, s(2n))   <- readsPrec 0 s(2n-1),
 *  					(")",s(2n+1)) <- lex s(2n) ]
 */
static Cell local mkReadTuple(Cell tup) /* readsPrec for n-tuple */
{
    Int  arity  = tupleOf(tup);
    Cell lp     = mkStr(findText("("));
    Cell rp     = mkStr(findText(")"));
    Cell co     = mkStr(findText(","));
    Cell sep    = lp;
    Cell r      = inventVar();
    Cell prev_s = r;
    Cell s      = inventVar();
    Cell exp    = tup;
    List quals  = NIL;
    Int  i;

    /* build (reversed) list of qualifiers and constructor */
    for(i=0; i<arity; i++) {
	Cell t  = inventVar();
	Cell si = inventVar();
	Cell sj = inventVar();
	quals  = cons(ZFexp(Tuple2(sep,si),Lex(prev_s)),quals);
	quals  = cons(ZFexp(Tuple2(t,sj),ReadsPrec(mkInt(0),si)), quals);
	exp    = ap(exp,t);
	prev_s = sj;
	sep    = co;
    }
    quals = cons(ZFexp(Tuple2(rp,s),Lex(prev_s)),quals);

    /* \ r -> [ (exp,s) | quals ] */
    return Lambda(singleton(r),ap(COMP,pair(Tuple2(exp,s),rev(quals))));
}

/* Given a record constructor
 *
 *   ... | C { f1 :: T1, ... fn :: Tn } | ...
 *
 * generate the expression:
 *
 *   \ r -> [(C t1 t2 ... tn,s(2n+1)) | ("C", s0)    <- lex r,
 *            				("{", s1)    <- lex s0,
 *            				(t1,  s2)    <- readField "f1" s1,
 *  	      				...
 *  	      				(",", s(2n-1)) <- lex s(2n),
 *            				(tn,  s(2n)) <- readField "fn" s(2n+1),
 *  	      				("}", s(2n+1)) <- lex s(2n+2) ]
 *
 * where
 *
 *   readField    :: Read a => String -> ReadS a
 *   readField m s0 = [ r | (t,  s1) <- lex s0, t == m,
 *   			    ("=",s2) <- lex s1,
 *   			    r        <- reads s2 ]
 */
static Cell local mkReadRecord(Cell con, List fs) /* readsPrec for record constructor */
{
    Cell cn     = mkStr(name(con).text);
    Cell lb     = mkStr(findText("{"));
    Cell rb     = mkStr(findText("}"));
    Cell co     = mkStr(findText(","));
    Cell sep    = lb;
    Cell r      = inventVar();
    Cell s0     = inventVar();
    Cell prev_s = s0;
    Cell s      = inventVar();
    Cell exp    = con;
    List quals  = NIL;

    /* build (reversed) list of qualifiers and constructor */
    quals  = cons(ZFexp(Tuple2(cn,s0),Lex(r)), quals);
    for(; nonNull(fs); fs=tl(fs)) {
	Cell f  = mkStr(textOf(hd(fs)));
	Cell t  = inventVar();
	Cell si = inventVar();
	Cell sj = inventVar();
	quals  = cons(ZFexp(Tuple2(sep,si),Lex(prev_s)),     quals);
	quals  = cons(ZFexp(Tuple2(t,  sj),ReadField(f,si)), quals);
	exp    = ap(exp,t);
	prev_s = sj;
	sep    = co;
    }
    quals = cons(ZFexp(Tuple2(rb,s),Lex(prev_s)),quals);

    /* \ r -> [ (exp,s) | quals ] */
    return Lambda(singleton(r),ap(COMP,pair(Tuple2(exp,s),rev(quals))));
}

#undef Tuple2
#undef Lex
#undef ZFexp
#undef ReadsPrec
#undef Lambda
#undef ReadParen
#undef ReadField
#undef GT
#undef Append

/* --------------------------------------------------------------------------
 * Deriving Bounded:
 * ------------------------------------------------------------------------*/

static List local deriveBounded(Tycon t)/* construct definition of bounds	   */
{
    if (isEnumType(t)) {
	Cell last  = tycon(t).defn;
	Cell first = hd(last);
	while (hasCfun(tl(last))) {
	    last = tl(last);
	}
	return cons(mkBind("minBound",mkVarAlts(tycon(t).line,first)),
		cons(mkBind("maxBound",mkVarAlts(tycon(t).line,hd(last))),
		 NIL));
    } else if (isTuple(t)) {	/* Definitions for product types	   */
	return mkBndBinds(0,t,tupleOf(t));
    } else if (isTycon(t) && cfunOf(hd(tycon(t).defn))==0) {
	return mkBndBinds(tycon(t).line,
			  hd(tycon(t).defn),
			  userArity(hd(tycon(t).defn)));
    }

    ERRMSG(tycon(t).line)
     "Can only derive instances of Bounded for enumeration and product types"
    EEND;
    return NIL;
}

static List local mkBndBinds(Int line, Cell h, Int n)	/* build bindings for derived	   */
              /* Bounded on a product type	   */
{
    Cell minB = h;
    Cell maxB = h;
    while (n-- > 0) {
	minB = ap(minB,nameMinBnd);
	maxB = ap(maxB,nameMaxBnd);
    }
    return cons(mkBind("minBound",mkVarAlts(line,minB)),
	    cons(mkBind("maxBound",mkVarAlts(line,maxB)),
	     NIL));
}

/* --------------------------------------------------------------------------
 * Default definitions; only one default definition is permitted in a
 * given script file.  If no default is supplied, then a standard system
 * default will be used where necessary.
 * ------------------------------------------------------------------------*/

Void defaultDefn(Int line, List defs)		/* Handle default types definition */
{
    if (defaultLine!=0) {
	ERRMSG(line) "Multiple default declarations are not permitted in" ETHEN
	ERRTEXT      " a single script file.\n"
	EEND;
    }
    defaultDefns = defs;
    defaultLine  = line;
}

static Void local checkDefaultDefns() {	/* check that default types are	   */
    List ds = NIL;			/* well-kinded instances of Num	   */

    if (defaultLine!=0) {
	map2Over(depTypeExp,defaultLine,NIL,defaultDefns);
	emptySubstitution();
	unkindTypes = NIL;
	map2Proc(kindType,defaultLine,"default type",defaultDefns);
	fixKinds();
	emptySubstitution();
	mapOver(fullExpand,defaultDefns);
    } else {
	defaultDefns = stdDefaults;
    }

    if (isNull(classNum)) {
	classNum = findClass(findText("Num"));
    }

    for (ds=defaultDefns; nonNull(ds); ds=tl(ds)) {
	if (isNull(provePred(NIL,NIL,ap(classNum,hd(ds))))) {
	    ERRMSG(defaultLine)
		"Default types must be instances of the Num class"
	    EEND;
	}
    }
}

/* --------------------------------------------------------------------------
 * Primitive definitions are usually only included in the first script
 * file read - the prelude.  A primitive definition associates a variable
 * name with a string (which identifies a built-in primitive) and a type.
 * ------------------------------------------------------------------------*/

Void primDefn(Cell line, List prims, Cell type)		/* Handle primitive definitions	   */
{
    primDefns = cons(triple(line,prims,type),primDefns);
}

static List local checkPrimDefn(Triple pd)	/* Check primitive definition	   */
{
    Int  line  = intOf(fst3(pd));
    List prims = snd3(pd);
    Type type  = thd3(pd);
    emptySubstitution();
    type = checkSigType(line,"primitive declaration",fst(hd(prims)),type);
    for (; nonNull(prims); prims=tl(prims)) {
	Cell   p    = hd(prims);
	Bool   same = isVar(p);
	Text   pt   = textOf(same ? p : fst(p));
	String pr   = textToStr(textOf(same ? p : snd(p)));
	hd(prims)   = addNewPrim(line,pt,pr,type);
    }
    return snd3(pd);
}

static Name local addNewPrim(Int l, Text vn, String s, Cell t)	/* make binding of variable vn to  */
             /* primitive function referred	   */
              /* to by s, with given type t	   */
{
    Name n = findName(vn);

    if (isNull(n)) {
	n = newName(vn,NIL);
    } else if (name(n).defn!=PREDEFINED) {
	duplicateError(l,name(n).mod,vn,"primitive");
    }

    addPrim(l,n,s,currentModule,t);
    return n;
}

/* --------------------------------------------------------------------------
 * Foreign import declarations are Cuddles' equivalent of GHC's ccall mechanism.
 * They are used to "import" C functions into a module.
 * Foreign export declarations allow Cuddles functions to be called from C.
 * Foreign export declarations provide the address of a C symbol.
 * ------------------------------------------------------------------------*/

/* When using the FFI, you first run Cuddles with generateFFI == TRUE
 * to generate C files for any modules which contain foreign import/export
 * declarations.  You then compile and partially link the C files and run
 * Cuddles with generateFFI == FALSE to load the object files and lookup
 * the appropriate helper functions in the object files.
 * Only when you run Cuddles in the second mode can you actually execute code.
 */
Bool generateFFI = FALSE;

Bool generate_ffi = FALSE;	/* generate FFI for the current module? */

static Int checkCallConv  (Int,Text);

/* Checking if the calling convention is supported */
static Int checkCallConv(Int line, Text t)
{
  if (t == textCCall ) return FFI_CCONV_CCALL;
#if STDCALL_SUPPORTED
  /* also support the stdcall calling convention */
  if (t == textStdcall) return FFI_CCONV_STDCALL;
#endif
  return FFI_CCONV_UNKNOWN;
}

/* Tricky naming detail:
 *
 * When we generate C function names, we need to make sure they
 * are unique within each module.  This is done using the foreignCount
 * variable.
 */
Void foreignImport(Cell l, Cell callconv, Cell safety, Cell ext, Cell intName, Cell type)
                                              /* Handle foreign imports    */
{
    Text t   = textOf(intName);
    Name n   = findName(t);
    Int sfty;
    Int cconv;
    Int line = intOf(l);

    if (isNull(n)) {
        n = newName(t,NIL);
    } else if (name(n).defn!=PREDEFINED) {
        ERRMSG(line) "Redeclaration of foreign \"%s\"", textToStr(t)
        EEND;
    }
    cconv = checkCallConv(line,textOf(callconv));
    if ( cconv == FFI_CCONV_UNKNOWN ) {
      ERRMSG(line) "Foreign import calling convention \"%s\" not supported",
  	           textToStr(textOf(callconv))
      EEND;
    }

    if (isNull(safety) || textOf(safety) == textSafe) {
        sfty = FFI_SAFE;
    } else if (textOf(safety) == textUnsafe) {
        sfty = FFI_UNSAFE;
    } else if (textOf(safety) == textThreadsafe) {
        sfty = FFI_THREADSAFE;
    } else {
        ERRMSG(line) "Foreign import safety level \"%s\" not supported", textToStr(textOf(safety))
        EEND;
    }
    name(n).line         = line;
    name(n).type         = type;
    name(n).extFun       = textOf(ext);
    name(n).foreignId    = foreignCount++;
    name(n).foreignFlags = sfty | cconv;
    foreignImports       = cons(n,foreignImports);
}

Void foreignExport(Cell l, Cell v, Cell callconv, Cell ext, Cell intName, Cell type)
                                              /* Handle foreign exports    */
{
    /*
     * Export attaches to an existing name in the symbol table.
     * We generate a new name whose definition is
     *   newName :: type
     *   newName = intName;
     * and pass the whole thing through the typechecker.
     *
     * This lets us have multiple exports of each symbol and nicely
     * deals with the fact that the exported signature might not match
     * the signature of the definition and might even be overloaded.
     *
     * The only problem is that the text of the name has to be
     * shared between the generated code and Cuddles.  Since these
     * are two separate invocations of Cuddles, we cannot use an
     * invented Text so we generate the text by adding a prefix
     * to the start which cannot conflict with normal identifiers.
     */
    Int  line    = intOf(l);
    Int  cconv;
    Text t   = concatText("--FFI_",textToStr(textOf(ext)));
    Name n   = newName(t,NIL);

    if (textOf(v) != textExport) {
        ERRMSG(line) "Foreign declarations must be either import or export not \"%s\"", textToStr(textOf(v))
        EEND;
    }
    cconv = checkCallConv(line,textOf(callconv));
    if ( cconv == FFI_CCONV_UNKNOWN ) {
      ERRMSG(line) "Foreign export calling convention \"%s\" not supported",
  	           textToStr(textOf(callconv))
      EEND;
    }

    name(n).line     = line;
    name(n).type     = type;
    name(n).extFun   = textOf(ext);
    name(n).defn     = intName;
    name(n).foreignId = foreignCount++;
    name(n).foreignFlags = FFI_NOSAFETY | cconv;
    foreignExports   = cons(n,foreignExports);
}

static String skipSpaces  (String);
static String matchToken  (String,String);
static String skipToSpace (String);
static String skipToChar  (String,int);
static String matchFname  (String);

static String skipSpaces(String s)
{
    while (isspace(*s)) {
        ++s;
    }
    return s;
}

static String skipToSpace(String s)
{
    while (*s != '\0' && !isspace(*s)) {
        ++s;
    }
    return s;
}

static String skipToChar(String s, int c)
{
    while (*s != '\0' && *s != c) {
        ++s;
    }
    return s;
}

static String matchToken(String t, String s)
{
    while (*t != '\0' && *t == *s) {
        ++t;
        ++s;
    }
    if (*t == '\0') {
        return s;
    } else {
        return 0;
    }
}

static String matchFname(String s)
{
    String t = skipToSpace(s);
    if (t-s >= 3 && t[-2] == '.' && t[-1] == 'h') {
        return t;
    } else {
        return 0;
    }
}

static Void local checkForeignImport(Name p)   /* Check foreign import          */
{
    Int  line      = name(p).line;
    String ext     = textToStr(name(p).extFun);
    String e       = 0;
    Type t         = NIL;
    List argTys    = NIL;

    Bool generate_stub = generate_ffi
      ;

    emptySubstitution();
    name(p).type = checkSigType(name(p).line,
                                "foreign import declaration",
                                p,
                                name(p).type);

    t = name(p).type;
    if (isPolyType(t)) {
        t = monotypeOf(t);
    }
    t = fullerExpand(t);
    while (getHead(t)==typeArrow && argCount==2) {
        Type ta = fullerExpand(arg(fun(t)));
        Type tr = fullerExpand(arg(t));
        argTys = cons(ta,argTys);
        t = tr;
    }
    argTys = rev(argTys);
    /* argTys now holds the argument tys and t holds result type */

    /* What kind of import is this? */
    ext = skipSpaces(ext);
    if ((e = matchToken("dynamic",ext))) { /* dynamic import */
        Type ta = NIL;
        Bool isIO = FALSE;

        e = skipSpaces(e);
        if (*e != '\0') goto cantparse;

        /* type must be of the form:
         *
         *    (FunPtr fty) -> fty
         *
         */

        if (length(argTys) < 1) goto dynerr;
        ta = hd(argTys);
        argTys = tl(argTys);

        if (getHead(ta) != typeFunPtr || argCount!=1) goto dynerr;
        ta = hd(getArgs(ta));

        /* ToDo: check that ta == argTys -> t and check it's a valid type */

        if (getHead(t) == typeIO && argCount==1) {
            isIO = TRUE;
            t = fullerExpand(hd(getArgs(t)));
        }

        if (generate_stub) {
            name(p).arity = 1 + length(argTys) + (isIO ? IOArity : 0);
            name(p).extFun = inventText();
            implementForeignImportDynamic(line,name(p).foreignId,name(p).extFun,argTys,isIO,t);
        }

    } else if ((e = matchToken("wrapper",ext))) { /* thunk builder */
        Bool isIO = FALSE;
        Type ta = NIL;

        e = skipSpaces(e);
        if (*e != '\0') goto cantparse;

        if (length(argTys) != 1) goto wraperr;
        ta = hd(argTys);

        if (getHead(t) != typeIO || argCount!=1) goto wraperr;
        t = fullerExpand(hd(getArgs(t)));

        if (getHead(t) != typeFunPtr) goto wraperr;
        t = fullerExpand(hd(getArgs(t)));

        /* ToDo: check that ta == t */

	t = fullerExpand(t);
        argTys = NIL;
        while (getHead(t)==typeArrow && argCount==2) {
            Type ta = fullerExpand(arg(fun(t)));
            Type tr = fullerExpand(arg(t));
            argTys = cons(ta,argTys);
            t = tr;
        }
        argTys = rev(argTys);
        /* argTys now holds the argument tys and t holds result type */

        if (getHead(t) == typeIO && argCount==1) {
            isIO = TRUE;
            t = fullerExpand(hd(getArgs(t)));
        }
	t = fullerExpand(t);

        if (generate_stub) {
            name(p).arity = 1+IOArity;
            name(p).extFun = inventText();
            implementForeignImportWrapper(line,name(p).foreignId,name(p).extFun,argTys,isIO,t);
        }

    } else {
        /* static function or address:
         *
         *  ['static'] [fname] ['[' lib ']'] [&] [cid]
	 *
         * or, for .NET bindings:
         *
	 *  ['static'] ['ctor' | 'field' | 'method' ] ['[' lib ']'] [cid]
         *
         */
	Bool isStatic = FALSE;
        Text fn       = -1;
        Text cid      = -1;
        Bool isLabel  = FALSE;

        if ((e = matchToken("static",ext))) {
	    isStatic = TRUE;
            ext = skipSpaces(e);
        }
        if ((e = matchFname(ext))) {
            fn = subText(ext,e-ext);
            ext = skipSpaces(e);
        }

        if ((e = matchToken("&",ext))) {
            isLabel = TRUE;
            ext = skipSpaces(e);
        }

        if ((e = matchToken("[",ext))) {
            ext = skipToChar(e,']');
            if (*ext != ']' || ext == e) goto cantparse;
            ERRMSG(line) "Cuddles doesn't use library specifications."
            EEND;
        }
        if (*ext != '\0') {
            e = skipToSpace(ext);
            cid = subText(ext,e-ext);
            ext = skipSpaces(e);
        } else {
            cid = name(p).text;
        }

        if (*ext != '\0') goto cantparse;

        if (isLabel) {
            if (!isNull(argTys)) goto labelerr;
            if (!(  (getHead(t) == typePtr && argCount == 1)
		 || (getHead(t) == typeFunPtr && argCount == 1)
		 )) {
                goto labelerr;
            }
            if (generate_stub) {
                name(p).arity = 0;
                implementForeignImportLabel(line,name(p).foreignId,fn,cid,name(p).text,t);
                name(p).extFun = cid;
            }
        } else {
            Bool isIO = FALSE;

            if (getHead(t) == typeIO && argCount==1) {
                isIO = TRUE;
                t = fullerExpand(hd(getArgs(t)));
            }

            if (generate_stub) {
                name(p).arity
                    = length(argTys)
                    + (isIO ? IOArity : 0);
                implementForeignImport(line,p,name(p).foreignId,
				       fn,
				       cid,isStatic,
				       -1, argTys,isIO,t);
                name(p).extFun = cid;
            }
        }
    }

    return;

  cantparse:
     ERRMSG(line) "Can't parse external entity '"
     ETHEN ERRTEXT ext
     ETHEN ERRTEXT "'\n"
     EEND;

  dynerr:
    ERRMSG(line) "foreign import dynamic must have type '(FunPtr ft) -> ft'"
    EEND;

  wraperr:
    ERRMSG(line) "foreign import wrapper must have type 'ft -> IO (FunPtr ft)'"
    EEND;

  labelerr:
    ERRMSG(line) "foreign import & must have type 'Ptr a' or 'FunPtr a'"
    EEND;

}

static Void local checkForeignExport(Name p)       /* Check foreign export      */
{
    Int  line  = name(p).line;
    Text ext   = name(p).extFun;
    Type t;
    List argTys = NIL;
    Bool isIO   = FALSE;

    emptySubstitution();
    name(p).type = checkSigType(line,
                                "foreign export declaration",
                                p,
                                name(p).type);
    t = name(p).type;
    t = fullerExpand(t);

    while (getHead(t)==typeArrow && argCount==2) {
        Type ta = fullerExpand(arg(fun(t)));
        Type tr = fullerExpand(arg(t));
        argTys = cons(ta,argTys);
        t = tr;
    }
    argTys = rev(argTys);

    if (getHead(t) == typeIO && argCount==1) {
        t = fullerExpand(hd(getArgs(t)));
        isIO = TRUE;
    }

    if (generate_ffi) {
        name(p).arity
          = length(argTys)
          + (isIO ? IOArity : 0);
        implementForeignExport(line,name(p).foreignId,ext,argTys,isIO,t);
    }
}

static Void local linkForeign(Name p)        /* Link an ffi-generated primitive */
{
    addPrim(name(p).line,p,textToStr(name(p).text),name(p).mod,name(p).type);
}

/* --------------------------------------------------------------------------
 * Static analysis of patterns:
 *
 * Patterns are parsed as ordinary (atomic) expressions.  Static analysis
 * makes the following checks:
 *  - Patterns are well formed (according to pattern syntax), including the
 *    special case of (n+k) patterns.
 *  - All constructor functions have been defined and are used with the
 *    correct number of arguments.
 *  - No variable name is used more than once in a pattern.
 *
 * The list of pattern variables occuring in each pattern is accumulated in
 * a global list `patVars', which must be initialised to NIL at appropriate
 * points before using these routines to check for valid patterns.  This
 * mechanism enables the pattern checking routine to be mapped over a list
 * of patterns, ensuring that no variable occurs more than once in the
 * complete pattern list (as is required on the lhs of a function defn).
 * ------------------------------------------------------------------------*/

static List patVars;		       /* List of vars bound in pattern    */

static Cell local checkPat(Int line, Cell p)     /* Check valid pattern syntax	   */
{
    STACK_CHECK
    switch (whatIs(p)) {
	case VARIDCELL :
	case VAROPCELL : addToPatVars(line,p);
			 break;

	case INFIX     : return checkPat(line,tidyInfix(line,snd(p)));

	case AP        : return checkMaybeCnkPat(line,p);

	case NAME      :
	case QUALIDENT :
	case CONIDCELL :
	case CONOPCELL : return checkApPat(line,0,p);

	case ZERONUM   :
	case POSNUM    :
	case NEGNUM    :
	case WILDCARD  :
	case STRCELL   :
	case CHARCELL  :
	case DOUBLECELL:
	case INTCELL   : break;

	case ASPAT     : addToPatVars(line,fst(snd(p)));
			 snd(snd(p)) = checkPat(line,snd(snd(p)));
			 break;

	case LAZYPAT   : snd(p) = checkPat(line,snd(p));
			 break;

	case FINLIST   : map1Over(checkPat,line,snd(p));
			 break;

	case CONFLDS   : depConFlds(line,p,TRUE);
			 break;

	case ESIGN     : snd(snd(p)) = checkPatType(line,
						    "pattern",
						    fst(snd(p)),
						    snd(snd(p)));
			 fst(snd(p)) = checkPat(line,fst(snd(p)));
			 break;

	default        : ERRMSG(line) "Illegal pattern syntax"
			 EEND;
    }
    return p;
}

static Cell local checkMaybeCnkPat(Int l, Cell p)/* Check applicative pattern with   */
                  /* the possibility of n+k pattern   */
{
    return checkApPat(l,0,p);
}

static Cell local checkApPat(Int line, Int args, Cell p)
                    /* check validity of application    */
                    /* of constructor to arguments	   */
{
    switch (whatIs(p)) {
	case AP        : fun(p) = checkApPat(line,args+1,fun(p));
			 arg(p) = checkPat(line,arg(p));
			 break;

	case TUPLE     : if (tupleOf(p)!=args) {
			     ERRMSG(line) "Illegal tuple pattern"
			     EEND;
			 }
			 break;


	case QUALIDENT : if (!isQCon(p)) {
			    ERRMSG(line)
				"Illegal use of qualified variable in pattern"
			    EEND;
			 }
			 /* deliberate fall through */
	case CONIDCELL :
	case CONOPCELL : p = conDefined(line,p,TRUE);
			 checkCfunArgs(line,p,args);
			 break;

	case NAME      : checkIsCfun(line,p);
			 checkCfunArgs(line,p,args);
			 break;

	default        : ERRMSG(line) "Illegal pattern syntax"
			 EEND;
    }
    return p;
}

static Void local addToPatVars(Int line, Cell v)	/* Add variable v to list of vars  */
              /* in current pattern, checking	   */
{				/* for repeated variables.	   */
    Text t = textOf(v);
    List p = NIL;
    List n = patVars;

    for (; nonNull(n); p=n, n=tl(n)) {
	if (textOf(hd(n))==t) {
	    ERRMSG(line) "Repeated variable \"%s\" in pattern",
			 textToStr(t)
	    EEND;
	}
    }

    if (isNull(p)) {
	 patVars = cons(v,NIL);
    } else {
	 tl(p)	 = cons(v,NIL);
    }
}

static Name local conDefined(Int line, Cell nm, Bool check)
             /* check that nm is the name of a  */
                         /* previously defined constructor  */
{		 /* function (and only one.)        */
    Name n = findQualName(nm);
    if (isNull(n)) {
	ERRMSG(line) "Undefined data constructor \"%s\"", identToStr(nm)
	EEND;
    }
    checkIsCfun(line,n);
    if (check) {
      if (isQualIdent(nm)) {
	depQVar(line,nm,FALSE);
      } else {
	checkNameAmbigName(line,n,FALSE);
      }
    }
    return n;
}

static Void local checkIsCfun(Int line, Name c)  /* Check that c is a constructor fn */
{
    if (!isCfun(c)) {
	ERRMSG(line) "\"%s\" is not a data constructor",
		     textToStr(name(c).text)
	EEND;
    }
}

static Void local checkCfunArgs(Int line, Cell c, Int args)
                    /* Check constructor applied with   */
                  /* correct number of arguments	   */
{
    Int a = userArity(c);
    if (a!=args) {
	ERRMSG(line)
	  "Constructor \"%s\" must have exactly %d argument%s in pattern",
	  textToStr(name(c).text), a, ((a==1)?"":"s")
	EEND;
    }
}

static Cell local checkPatType(Int l, String wh, Cell e, Type t)/* Check type appearing in pattern */
{
    List tvs = typeVarsIn(t,NIL,NIL,NIL);
    for (; nonNull(tvs); tvs=tl(tvs)) {
	Int beta    = newKindvars(1);
	hd(btyvars) = cons(pair(hd(tvs),mkInt(beta)), hd(btyvars));
    }
    t = checkSigType(l,"pattern type",e,t);
    if (isPolyOrQualType(t) || whatIs(t)==RANK2) {
	ERRMSG(l) "Illegal syntax in %s type annotation", wh
	EEND;
    }
    return t;
}

static Cell local applyBtyvs(Cell pat)	/* Record bound type vars in pat   */
{
    List bts = hd(btyvars);
    leaveBtyvs();
    if (nonNull(bts)) {
	pat = ap(BIGLAM,pair(bts,pat));
	for (; nonNull(bts); bts=tl(bts)) {
	    snd(hd(bts)) = copyKindvar(intOf(snd(hd(bts))));
	}
    }
    return pat;
}

/* --------------------------------------------------------------------------
 * Maintaining lists of bound variables and local definitions, for
 * dependency and scope analysis.
 * ------------------------------------------------------------------------*/

static List bounds;		       /* list of lists of bound vars	   */
static List bindings;		       /* list of lists of binds in scope  */
static List depends;		       /* list of lists of dependents	   */

/* bounds   :: [[Var]]        -- var equality used on Vars     */
/* bindings :: [[([Var],?)]]  -- var equality used on Vars     */
/* depends  :: [[Var]]        -- pointer equality used on Vars */


#define saveBvars()	 hd(bounds)    /* list of bvars in current scope   */
#define restoreBvars(bs) hd(bounds)=bs /* restore list of bound variables  */

static Cell local bindPat(Int line, Cell p)      /* add new bound vars for pattern   */
{
    patVars    = NIL;
    p	       = checkPat(line,p);
    hd(bounds) = dupOnto(patVars,hd(bounds));
    return p;
}

static Void local bindPats(Int line, List ps)    /* add new bound vars for patterns  */
{
    patVars    = NIL;
    map1Over(checkPat,line,ps);
    hd(bounds) = revOnto(patVars,hd(bounds));
}

/* --------------------------------------------------------------------------
 * Before processing value and type signature declarations, all data and
 * type definitions have been processed so that:
 * - all valid type constructors (with their arities) are known.
 * - all valid constructor functions (with their arities and types) are
 *   known.
 *
 * The result of parsing a list of value declarations is a list of Eqns:
 *	 Eqn ::= (SIGDECL,(Line,[Var],type))
 *	      |  (FIXDECL,(Line,[Op],SyntaxInt))
 *	      |  (Expr,Rhs)
 * The ordering of the equations in this list is the reverse of the original
 * ordering in the script parsed.  This is a consequence of the structure of
 * the parser ... but also turns out to be most convenient for the static
 * analysis.
 *
 * As the first stage of the static analysis of value declarations, each
 * list of Eqns is converted to a list of Bindings.  As part of this
 * process:
 * - The ordering of the list of Bindings produced is the same as in the
 *   original script.
 * - When a variable (function) is defined over a number of lines, all
 *   of the definitions should appear together and each should give the
 *   same arity to the variable being defined.
 * - No variable can have more than one definition.
 * - For pattern bindings:
 *   - Each lhs is a valid pattern/function lhs, all constructor functions
 *     have been defined and are used with the correct number of arguments.
 *   - Each lhs contains no repeated pattern variables.
 *   - Each equation defines at least one variable (e.g. True = False is
 *     not allowed).
 * - Types appearing in type signatures are well formed:
 *    - Type constructors used are defined and used with correct number
 *	of arguments.
 *    - type variables are replaced by offsets, type constructor names
 *	by Tycons.
 * - Every variable named in a type signature declaration is defined by
 *   one or more equations elsewhere in the script.
 * - No variable has more than one type declaration.
 * - Similar properties for fixity declarations.
 *
 * ------------------------------------------------------------------------*/

#define bindingAttr(b) fst(snd(b))     /* type(s)/fixity(ies) for binding  */
#define fbindAlts(b)   snd(snd(b))     /* alternatives for function binding*/

static List local extractSigdecls(List es)  /* Extract the SIGDECLS from list   */
{			       /* of equations			   */
    List sigdecls = NIL;	       /* :: [(Line,[Var],Type)]	   */

    for(; nonNull(es); es=tl(es)) {
	if (fst(hd(es))==SIGDECL) {		     /* type-declaration?  */
	    Pair sig  = snd(hd(es));
	    Int  line = intOf(fst3(sig));
	    List vs   = snd3(sig);
	    for(; nonNull(vs); vs=tl(vs)) {
		if (isQualIdent(hd(vs))) {
		    ERRMSG(line) "Type signature for qualified variable \"%s\" is not allowed",
				 identToStr(hd(vs))
		    EEND;
		}
	    }
	    sigdecls = cons(sig,sigdecls);           /* discard SIGDECL tag*/
	}
    }
    return sigdecls;
}

static List local extractFixdecls(List es)	/* Extract the FIXDECLS from list  */
{				/* of equations			   */
    List fixdecls = NIL;		/* :: [(Line,SyntaxInt,[Op])]	   */

    for(; nonNull(es); es=tl(es)) {
	if (fst(hd(es))==FIXDECL) {		     /* fixity declaration?*/
	    fixdecls = cons(snd(hd(es)),fixdecls);   /* discard FIXDECL tag*/
	}
    }
    return fixdecls;
}

static List local extractBindings(List ds)	/* extract untyped bindings from   */
{				/* given list of equations	   */
    Cell lastVar   = NIL;		/* = var def'd in last eqn (if any)*/
    Int  lastArity = 0;			/* = number of args in last defn   */
    List bs	   = NIL;		/* :: [Binding]			   */

    for(; nonNull(ds); ds=tl(ds)) {
	Cell d = hd(ds);
	if (fst(d)==FUNBIND) {		/* Function bindings		   */
	    Cell rhs	= snd(snd(d));
	    Int  line	= rhsLine(rhs);
	    Cell lhs	= fst(snd(d));
	    Cell v	= getHead(lhs);
	    Cell newAlt = pair(getArgs(lhs),rhs);
	    if ( !isVar(v) ) {
		internal("FUNBIND");
	    }
	    if (nonNull(lastVar) && (textOf(v))==textOf(lastVar)) {
		if (argCount!=lastArity) {
		    ERRMSG(line) "Equations give different arities for \"%s\"",
				 textToStr(textOf(v))
		    EEND;
		}
		fbindAlts(hd(bs)) = cons(newAlt,fbindAlts(hd(bs)));
	    }
	    else {
		lastVar   = v;
		lastArity = argCount;
		notDefined(line,bs,v);
		bs	  = cons(pair(v,pair(NIL,singleton(newAlt))),bs);
	    }

	} else if (fst(d)==PATBIND) {	/* Pattern bindings		   */
	    Cell rhs  = snd(snd(d));
	    Int  line = rhsLine(rhs);
	    Cell pat  = fst(snd(d));
	    while (whatIs(pat)==ESIGN) {/* Move type annotations to rhs   */
		Cell p	      = fst(snd(pat));
		fst(snd(pat)) = rhs;
		snd(snd(d))   = rhs = pat;
		fst(snd(d))   = pat = p;
		/* Lift out the line number, i.e.,
		 * (ESIGN,((location, expr),ty)) ~=> (location,(ESIGN,(expr,ty)))
		 */
		p = snd(rhs);
		fst(rhs) = fst(fst(p));
		snd(rhs) = ap(ESIGN,ap(snd(fst(p)),snd(p)));
	    }
	    if (isVar(pat)) {		/* Convert simple pattern bind to */
		notDefined(line,bs,pat);/* a function binding		  */
		bs = cons(pair(pat,pair(NIL,singleton(pair(NIL,rhs)))),bs);
	    } else {
		List vs	= getPatVars(line,pat,NIL);
		map2Proc(notDefined,line,bs,vs);
		bs	= cons(pair(vs,pair(NIL,snd(d))),bs);
	    }
	    lastVar = NIL;
	} else
	    lastVar = NIL;
    }
    return bs;
}

static List local getPatVars(Int line, Cell p, List vs)	/* Find list of variables bound in */
              /* pattern p			   */
{
    switch (whatIs(p)) {
	case AP		: do {
			      vs = getPatVars(line,arg(p),vs);
			      p  = fun(p);
			  } while (isAp(p));
			  return vs;	/* Ignore head of application	   */

	case CONFLDS	: {   List pfs = snd(snd(p));
			      for (; nonNull(pfs); pfs=tl(pfs)) {
				  if (isVar(hd(pfs))) {
				      vs = addPatVar(line,hd(pfs),vs);
				  } else {
				      vs = getPatVars(line,snd(hd(pfs)),vs);
				  }
			      }
			  }
			  return vs;

	case FINLIST	: {   List ps = snd(p);
			      for (; nonNull(ps); ps=tl(ps)) {
				  vs = getPatVars(line,hd(ps),vs);
			      }
			  }
			  return vs;

	case ESIGN	: return getPatVars(line,fst(snd(p)),vs);

	case LAZYPAT	:
	case NEG	:
	case ONLY	:
	case INFIX	: return getPatVars(line,snd(p),vs);

	case ASPAT	: return addPatVar(line,fst(snd(p)),
					     getPatVars(line,snd(snd(p)),vs));

	case VARIDCELL	:
	case VAROPCELL	: return addPatVar(line,p,vs);

	case CONIDCELL	:
	case CONOPCELL	:
	case QUALIDENT 	:
	case INTCELL	:
	case DOUBLECELL	:
	case CHARCELL	:
	case STRCELL	:
	case NAME	:
	case WILDCARD	: return vs;

	default		: internal("getPatVars");
    }
    return vs;
}

static List local addPatVar(Int line, Cell v, List vs)	/* Add var to list of previously   */
              /* encountered variables	   */
{
    if (varIsMember(textOf(v),vs)) {
	ERRMSG(line) "Repeated use of variable \"%s\" in pattern binding",
		     textToStr(textOf(v))
	EEND;
    }
    return cons(v,vs);
}

static List local eqnsToBindings(List es, List ts, List cs, List ps)
            /* Convert list of equations to	   */
            /* list of typed bindings	   */
{
    List bs = extractBindings(es);
    map1Proc(addSigdecl,bs,extractSigdecls(es));
    map4Proc(addFixdecl,bs,ts,cs,ps,extractFixdecls(es));
    return bs;
}

static Void local notDefined(Int line, List bs, Cell v)/* check if name already defined in */
                    /* list of bindings		   */
{
    if (nonNull(findBinding(textOf(v),bs))) {
	ERRMSG(line) "\"%s\" multiply defined", textToStr(textOf(v))
	EEND;
    }
}

static Cell local findBinding(Text t, List bs)    /* look for binding for variable t  */
                  /* in list of bindings bs	   */
{
    for (; nonNull(bs); bs=tl(bs)) {
	if (isVar(fst(hd(bs)))) {		      /* function-binding? */
	    if (textOf(fst(hd(bs)))==t) {
		return hd(bs);
	    }
	} else if (nonNull(varIsMember(t,fst(hd(bs))))){/* pattern-binding?*/
	    return hd(bs);
	}
    }
    return NIL;
}

static Cell local getAttr(List bs, Cell v)		/* Locate type/fixity attribute    */
            /* for variable v in bindings bs   */
{
    Text t = textOf(v);
    Cell b = findBinding(t,bs);

    if (isNull(b)) {					/* No binding	   */
	return NIL;
    } else if (isVar(fst(b))) {				/* func binding?   */
	if (isNull(bindingAttr(b))) {
	    bindingAttr(b) = pair(NIL,NIL);
	}
	return bindingAttr(b);
    } else {						/* pat binding?	   */
	List vs = fst(b);
	List as = bindingAttr(b);

	if (isNull(as)) {
	    bindingAttr(b) = as = replicate(length(vs),NIL);
	}

	while (nonNull(vs) && t!=textOf(hd(vs))) {
	    vs = tl(vs);
	    as = tl(as);
	}

	if (isNull(vs)) {
	    internal("getAttr");
	} else if (isNull(hd(as))) {
	    hd(as) = pair(NIL,NIL);
	}
	return hd(as);
    }
}

static Void local addSigdecl(List bs, Cell sigdecl)/* add type information to bindings*/
                  /* :: [Binding]			   */
{ 		       /* :: (Line,[Var],Type)		   */
    Int  l    = intOf(fst3(sigdecl));
    List vs   = snd3(sigdecl);
    Type type = checkSigType(l,"type declaration",hd(vs),thd3(sigdecl));

    for (; nonNull(vs); vs=tl(vs)) {
	Cell v    = hd(vs);
	Pair attr = getAttr(bs,v);
	if (isNull(attr)) {
	    ERRMSG(l) "Missing binding for variable \"%s\" in type signature",
		      textToStr(textOf(v))
	    EEND;
	} else if (nonNull(fst(attr))) {
	    ERRMSG(l) "Multiple type signatures for \"%s\"",
		      textToStr(textOf(v))
	    EEND;
	}
	fst(attr) = type;
    }
}

static Void local addFixdecl(List bs, List ts, List cs, List ps, Triple fixdecl)
{
    Int  line = intOf(fst3(fixdecl));
    List ops  = snd3(fixdecl);
    Cell sy   = thd3(fixdecl);

    for (; nonNull(ops); ops=tl(ops)) {
	Cell op   = hd(ops);
	Text t    = textOf(op);
	Cell attr = getAttr(bs,op);
	if (nonNull(attr)) {		/* Found name in binding?	   */
	    if (nonNull(snd(attr))) {
		dupFixity(line,t);
	    }
	    snd(attr) = sy;
	} else {			/* Look in tycons, classes, prims  */
	    Name n   = NIL;
	    List ts1 = ts;
	    List cs1 = cs;
	    List ps1 = ps;
	    for (; isNull(n) && nonNull(ts1); ts1=tl(ts1)) {	/* tycons  */
		Tycon tc = hd(ts1);
		if (tycon(tc).what==DATATYPE || tycon(tc).what==NEWTYPE) {
		    n = nameIsMember(t,tycon(tc).defn);
		}
	    }
	    for (; isNull(n) && nonNull(cs1); cs1=tl(cs1)) {	/* classes */
		n = nameIsMember(t,cclass(hd(cs1)).members);
	    }
	    for (; isNull(n) && nonNull(ps1); ps1=tl(ps1)) {	/* prims   */
		n = nameIsMember(t,hd(ps1));
	    }

	    if (isNull(n)) {
		missFixity(line,t);
	    } else if (name(n).syntax!=NO_SYNTAX) {
		dupFixity(line,t);
	    }
	    name(n).syntax = intOf(sy);
	}
    }
}

static Void local dupFixity(Int line, Text t)	/* Report repeated fixity decl	   */
{
    ERRMSG(line)
	"Multiple fixity declarations for operator \"%s\"", textToStr(t)
    EEND;
}

static Void local missFixity(Int line, Text t)	/* Report missing op for fixity	   */
{
    ERRMSG(line)
	"Cannot find binding for operator \"%s\" in fixity declaration",
	textToStr(t)
    EEND;
}

/* --------------------------------------------------------------------------
 * Dealing with infix operators:
 *
 * Expressions involving infix operators or unary minus are parsed as
 * elements of the following type:
 *
 *     data InfixExp = Only Exp | Neg InfixExp | Infix InfixExp Op Exp
 *
 * (The algorithms here do not assume that negation can be applied only once,
 * i.e., that - - x is a syntax error, as required by the Haskell report.
 * Instead, that restriction is captured by the grammar itself, given above.)
 *
 * There are rules of precedence and grouping, expressed by two functions:
 *
 *     prec :: Op -> Int;   assoc :: Op -> Assoc    (Assoc = {L, N, R})
 *
 * InfixExp values are rearranged accordingly when a complete expression
 * has been read using a simple shift-reduce parser whose result may be taken
 * to be a value of the following type:
 *
 *     data Exp = Atom Int | Negate Exp | Apply Op Exp Exp | Error String
 *
 * The machine on which this parser is based can be defined as follows:
 *
 *     tidy                         :: InfixExp -> [(Op,Exp)] -> Exp
 *     tidy (Only a)      []         = a
 *     tidy (Only a)      ((o,b):ss) = tidy (Only (Apply o a b)) ss
 *     tidy (Infix a o b) []         = tidy a [(o,b)]
 *     tidy (Infix a o b) ((p,c):ss)
 *                      | shift  o p = tidy a ((o,b):(p,c):ss)
 *                      | red    o p = tidy (Infix a o (Apply p b c)) ss
 *                      | ambig  o p = Error "ambiguous use of operators"
 *     tidy (Neg e)       []         = tidy (tidyNeg e) []
 *     tidy (Neg e)       ((o,b):ss)
 *                      | nshift o   = tidy (Neg (underNeg o b e)) ss
 *                      | nred   o   = tidy (tidyNeg e) ((o,b):ss)
 *                      | nambig o   = Error "illegal use of negation"
 *
 * At each stage, the parser can either shift, reduce, accept, or error.
 * The transitions when dealing with juxtaposed operators o and p are
 * determined by the following rules:
 *
 *     shift o p  = (prec o > prec p)
 *               || (prec o == prec p && assoc o == L && assoc p == L)
 *
 *     red o p    = (prec o < prec p)
 *               || (prec o == prec p && assoc o == R && assoc p == R)
 *
 *     ambig o p  = (prec o == prec p)
 *               && (assoc o == N || assoc p == N || assoc o /= assoc p)
 *
 * The transitions when dealing with juxtaposed unary minus and infix
 * operators are as follows.  The precedence of unary minus (infixl 6) is
 * hardwired in to these definitions, as it is to the definitions of the
 * Haskell grammar in the official report.
 *
 *     nshift o   = (prec o > 6)
 *     nred   o   = (prec o < 6) || (prec o == 6 && assoc o == L)
 *     nambig o   = prec o == 6 && (assoc o == R || assoc o == N)
 *
 * An InfixExp of the form (Neg e) means negate the last thing in
 * the InfixExp e; we can force this negation using:
 *
 *     tidyNeg              :: OpExp -> OpExp
 *     tidyNeg (Only e)      = Only (Negate e)
 *     tidyNeg (Infix a o b) = Infix a o (Negate b)
 *     tidyNeg (Neg e)       = tidyNeg (tidyNeg e)
 *
 * On the other hand, if we want to sneak application of an infix operator
 * under a negation, then we use:
 *
 *     underNeg                  :: Op -> Exp -> OpExp -> OpExp
 *     underNeg o b (Only e)      = Only (Apply o e b)
 *     underNeg o b (Neg e)       = Neg (underNeg o b e)
 *     underNeg o b (Infix e p f) = Infix e p (Apply o f b)
 *
 * As a concession to efficiency, we lower the number of calls to syntaxOf
 * by keeping track of the values of sye, sys throughout the process.  The
 * value APPLIC is used to indicate that the syntax value is unknown.
 * ------------------------------------------------------------------------*/

static Cell local tidyInfix(Int line, Cell e)	/* Convert infixExp to Exp	   */
{				/* :: OpExp			   */
    Cell   s   = NIL;			/* :: [(Op,Exp)]		   */
    Syntax sye = APPLIC;		/* Syntax of op in e (init unknown)*/
    Syntax sys = APPLIC;		/* Syntax of op in s (init unknown)*/
    Cell   d   = e;

    while (fst(d)!=ONLY) {		/* Attach fixities to operators    */
	if (fst(d)==NEG) {
	    d = snd(d);
	} else {
	    fun(fun(d)) = attachFixity(line,fun(fun(d)));
	    d		= arg(fun(d));
	}
    }

    for (;;)
	switch (whatIs(e)) {
	    case ONLY : e = snd(e);
                        while (nonNull(s)) {
                            Cell next   = arg(fun(s));
                            arg(fun(s)) = e;
			    fun(fun(s)) = snd(snd(fun(fun(s))));
                            e           = s;
                            s           = next;
			}
		        return e;

	    case NEG  : if (nonNull(s)) {
			    if (sys==APPLIC) {	/* calculate sys	   */
				sys = intOf(fst(fun(fun(s))));
			    }

			    if (precOf(sys)==UMINUS_PREC &&	/* nambig  */
				assocOf(sys)!=UMINUS_ASSOC) {
				ERRMSG(line)
				 "Ambiguous use of unary minus with \""
				ETHEN ERREXPR(fst(snd(fun(fun(s)))));
				ERRTEXT "\""
				EEND;
			    }

			    if (precOf(sys)>UMINUS_PREC) {	/* nshift  */
				Cell e1    = snd(e);
				Cell t     = s;
                                s          = arg(fun(s));
				while (whatIs(e1)==NEG)
				    e1 = snd(e1);
				arg(fun(t)) = arg(e1);
				fun(fun(t)) = snd(snd(fun(fun(t))));
                                arg(e1)     = t;
				sys         = APPLIC;
				continue;
			    }
			}

			/* Intentional fall-thru for nreduce and isNull(s) */

			{   Cell prev = e;		/* e := tidyNeg e  */
			    Cell temp = arg(prev);
			    Int  nneg = 1;
			    for (; whatIs(temp)==NEG; nneg++) {
				fun(prev) = nameNegate;
				prev	  = temp;
				temp	  = arg(prev);
			    }
			    if (isInt(arg(temp))) {	/* special cases   */
				if (nneg&1)		/* for literals    */
				    arg(temp) = mkInt(-intOf(arg(temp)));
			    }
			    else if (isBignum(arg(temp))) {
				if (nneg&1)
				    arg(temp) = bigNeg(arg(temp));
			    }
			    else if (isDouble(arg(temp))) {
				if (nneg&1)
				    arg(temp) = mkDouble(-doubleOf(arg(temp)));
			    }
			    else {
				fun(prev) = nameNegate;
				arg(prev) = arg(temp);
				arg(temp) = e;
			    }
			    e = temp;
			}
			continue;

	    default   : if (isNull(s)) {/* Move operation onto empty stack */
			    Cell next   = arg(fun(e));
                            s           = e;
			    arg(fun(s)) = NIL;
                            e           = next;
                            sys         = sye;
                            sye         = APPLIC;
			}
                        else {		/* deal with pair of operators	   */

			    if (sye==APPLIC) {	/* calculate sys and sye   */
				sye = intOf(fst(fun(fun(e))));
			    }
			    if (sys==APPLIC) {
				sys = intOf(fst(fun(fun(s))));
			    }

			    if (precOf(sye)==precOf(sys) &&	/* ambig   */
				(assocOf(sye)!=assocOf(sys) ||
				 assocOf(sye)==NON_ASS)) {
				ERRMSG(line) "Ambiguous use of operator \""
				ETHEN ERREXPR(fst(snd(fun(fun(e)))));
				ERRTEXT "\" with \""
				ETHEN ERREXPR(fst(snd(fun(fun(s)))));
				ERRTEXT "\""
				EEND;
			    }

			    if (precOf(sye)>precOf(sys) ||	/* shift   */
				(precOf(sye)==precOf(sys) &&
				 assocOf(sye)==LEFT_ASS &&
				 assocOf(sys)==LEFT_ASS)) {
				Cell next   = arg(fun(e));
				arg(fun(e)) = s;
				s	    = e;
				e	    = next;
				sys	    = sye;
				sye	    = APPLIC;
                            }
			    else {				/* reduce  */
				Cell next   = arg(fun(s));
				arg(fun(s)) = arg(e);
				fun(fun(s)) = snd(snd(fun(fun(s))));
				arg(e)      = s;
				s	    = next;
				sys	    = APPLIC;
				/* sye unchanged */
			    }
                        }
			continue;
	}
}

static Pair local attachFixity(Int line, Cell op)	/* Attach fixity to operator in an */
              /* infix expression		   */
{
    Syntax sy   = DEF_OPSYNTAX;
    Cell   trop = op;

    switch (whatIs(op)) {
	case VAROPCELL :
	case VARIDCELL : if ((sy=lookupSyntax(textOf(op)))==NO_SYNTAX) {
			     Name n = findName(textOf(op));
			     if (isNull(n)) {
				ERRMSG(line) "Undefined variable \"%s\"",
					     textToStr(textOf(op))
				EEND;
			     }
			     sy = syntaxOf(n);
			     trop = n;
			 }
			 break;

	case CONOPCELL :
	case CONIDCELL : sy = syntaxOf(trop = conDefined(line,op,FALSE));
			 break;

	case QUALIDENT : {   Name n = findQualName(op);
			     if (nonNull(n)) {
				 trop = n;
				 sy = syntaxOf(n);
			     } else {
				 ERRMSG(line)
				   "Undefined qualified variable \"%s\"",
				   identToStr(op)
				 EEND;
			     }
			 }
			 break;
	default        : break;
    }
    if (sy==APPLIC) {
	sy = DEF_OPSYNTAX;
    }
    return pair(mkInt(sy),pair(trop,op)); /* Pair fixity with (possibly) */
					  /* translated operator	 */
}

static Syntax local lookupSyntax(Text t)	/* Try to find fixity for var in  */
{				/* enclosing bindings		  */
    List bounds1   = bounds;
    List bindings1 = bindings;

    while (nonNull(bindings1)) {
	if (nonNull(varIsMember(t,hd(bounds1)))) {
	    return DEF_OPSYNTAX;
	} else {
	    Cell b = findBinding(t,hd(bindings1));
	    if (nonNull(b)) {
		Cell a = fst(snd(b));
		if (isVar(fst(b))) {	/* Function binding		   */
		    if (nonNull(a) && nonNull(snd(a))) {
			return intOf(snd(a));
		    }
		} else {		/* Pattern binding		   */
		    List vs = fst(b);
		    while (nonNull(vs) && nonNull(a)) {
			if (t==textOf(hd(vs))) {
			    if (nonNull(hd(a)) && isInt(snd(hd(a)))) {
				return intOf(snd(hd(a)));
			    }
			    break;
			}
			vs = tl(vs);
			a  = tl(a);
		    }
		}
		return DEF_OPSYNTAX;
	    }
	}
	bounds1   = tl(bounds1);
	bindings1 = tl(bindings1);
    }
    return NO_SYNTAX;
}

/* --------------------------------------------------------------------------
 * To facilitate dependency analysis, lists of bindings are temporarily
 * augmented with an additional field, which is used in two ways:
 * - to build the `adjacency lists' for the dependency graph. Represented by
 *   a list of pointers to other bindings in the same list of bindings.
 * - to hold strictly positive integer values (depth first search numbers) of
 *   elements `on the stack' during the strongly connected components search
 *   algorithm, or a special value mkInt(0), once the binding has been added
 *   to a particular strongly connected component.
 *
 * Using this extra field, the type of each list of declarations during
 * dependency analysis is [Binding'] where:
 *
 *    Binding' ::= (Var, (Attr, (Dep, [Alt])))	       -- function binding
 *		|  ([Var], ([Attr], (Dep, (Pat,Rhs)))) -- pattern binding
 *
 * ------------------------------------------------------------------------*/

#define depVal(d) (fst(snd(snd(d))))	/* Access to dependency information*/

static List local dependencyAnal(List bs)	/* Separate lists of bindings into */
{				/* mutually recursive groups in	   */
					/* order of dependency		   */

    mapProc(addDepField,bs);		/* add extra field for dependents  */
    mapProc(depBinding,bs);		/* find dependents of each binding */
    bs = bscc(bs);			/* sort to strongly connected comps*/
    mapProc(remDepField,bs);		/* remove dependency info field	   */
    return bs;
}

static List local topDependAnal(List bs)	/* Like dependencyAnal(), but at   */
{				/* top level, reporting on progress*/
    List xs;
    Int  i = 0;

    setGoal("Dependency analysis",(Target)(length(bs)));
    mapProc(addDepField,bs);	       /* add extra field for dependents   */
    for (xs=bs; nonNull(xs); xs=tl(xs)) {
	emptySubstitution();
	depBinding(hd(xs));
	soFar((Target)(i++));
    }
    bs = bscc(bs);		       /* sort to strongly connected comps */
    mapProc(remDepField,bs);	       /* remove dependency info field	   */
    done();
    return bs;
}

static Void local addDepField(Cell b)       /* add extra field to binding to    */
{			       /* hold list of dependents	   */
    snd(snd(b)) = pair(NIL,snd(snd(b)));
}

static Void local remDepField(List bs)      /* remove dependency field from	   */
{			       /* list of bindings		   */
    mapProc(remDepField1,bs);
}

static Void local remDepField1(Cell b)      /* remove dependency field from	   */
{			       /* single binding		   */
    snd(snd(b)) = snd(snd(snd(b)));
}

static Void local clearScope() {       /* initialise dependency scoping    */
    bounds   = NIL;
    bindings = NIL;
    depends  = NIL;
}

static Void local withinScope(List bs)	/* Enter scope of bindings bs	   */
{
    bounds   = cons(NIL,bounds);
    bindings = cons(bs,bindings);
    depends  = cons(NIL,depends);
}

static Void local leaveScope() {	/* Leave scope of last withinScope */
    List bs	  = hd(bindings);	/* Remove fixity info from binds   */
    Bool toplevel = isNull(tl(bindings));
    for (; nonNull(bs); bs=tl(bs)) {
	Cell b = hd(bs);
	if (isVar(fst(b))) {		/* Variable binding		   */
	    Cell a = fst(snd(b));
	    dropNameClash(fst(b));
	    if (isPair(a)) {
		if (toplevel) {
		    saveSyntax(fst(b),snd(a));
		}
		fst(snd(b)) = fst(a);
	    }
	} else {			/* Pattern binding		   */
	    List vs = fst(b);
	    List as = fst(snd(b));
	    while (nonNull(vs) && nonNull(as)) {
		if (isPair(hd(as))) {
		    dropNameClash(hd(vs));
		    if (toplevel) {
			saveSyntax(hd(vs),snd(hd(as)));
		    }
		    hd(as) = fst(hd(as));
		}
		vs = tl(vs);
		as = tl(as);
	    }
	}
    }
    bounds   = tl(bounds);
    bindings = tl(bindings);
    depends  = tl(depends);
}

static Void local dropNameClash(Cell v)
{
    Name n = findName(textOf(v));
    if ( !isNull(n) && nonNull(name(n).clashes) ) {
	name(n).clashes = tl(name(n).clashes);
    }
}

static Void local saveSyntax(Cell v, Cell sy)	/* Save syntax of top-level var	   */
            /* in corresponding Name	   */
{
    Name n = findName(textOf(v));
    if (isNull(n) || name(n).syntax!=NO_SYNTAX) {
	internal("saveSyntax");
    }
    if (nonNull(sy)) {
	name(n).syntax = intOf(sy);
    }
}


/* --------------------------------------------------------------------------
 * As a side effect of the dependency analysis we also make the following
 * checks:
 * - Each lhs is a valid pattern/function lhs, all constructor functions
 *   have been defined and are used with the correct number of arguments.
 * - No lhs contains repeated pattern variables.
 * - Expressions used on the rhs of an eqn should be well formed.  This
 *   includes:
 *   - Checking for valid patterns (including repeated vars) in lambda,
 *     case, and list comprehension expressions.
 *   - Recursively checking local lists of equations.
 * - No free (i.e. unbound) variables are used in the declaration list.
 * ------------------------------------------------------------------------*/

static Void local depBinding(Cell b)        /* find dependents of binding	   */
{
    Cell defpart = snd(snd(snd(b)));   /* definition part of binding	   */

    hd(depends) = NIL;

    if (isVar(fst(b))) {	       /* function-binding?		   */
	mapProc(depAlt,defpart);
	if (isNull(fst(snd(b)))) {	/* Save dep info if no type sig	   */
	    fst(snd(b)) = pair(ap(IMPDEPS,hd(depends)),NIL);
	} else if (isNull(fst(fst(snd(b))))) {
	    fst(fst(snd(b))) = ap(IMPDEPS,hd(depends));
	}
    } else {			       /* pattern-binding?		   */
	Int line = rhsLine(snd(defpart));
	enterBtyvs();
	patVars = NIL;
	fst(defpart) = checkPat(line,fst(defpart));
	depRhs(snd(defpart));
	fst(defpart) = applyBtyvs(fst(defpart));
    }
    depVal(b) = hd(depends);
}

static Void local depDefaults(Class c)       /* dependency analysis on defaults  */
{                             /* from class definition            */
    depClassBindings(cclass(c).defaults);
}

static Void local depInsts(Inst in)         /* dependency analysis on instance  */
{                             /* bindings                         */
    depClassBindings(inst(in).implements);
}

static Void local depClassBindings(List bs) /* dependency analysis on list of   */
{                             /* bindings, possibly containing    */
    for (; nonNull(bs); bs=tl(bs)) {   /* NIL bindings ...                 */
	if (nonNull(hd(bs))) {         /* No need to add extra field for   */
	   mapProc(depAlt,snd(hd(bs)));/* dependency information...	   */
	}
    }
}

static Cell local depLetRec(Bool isRhs, Int line, Cell e) /* dependency analysis on a letrec    */
{                                 /* expr, containing a set of bindings */

    fst(snd(e)) = eqnsToBindings(fst(snd(e)),NIL,NIL,NIL);
    withinScope(fst(snd(e)));
    fst(snd(e)) = dependencyAnal(fst(snd(e)));
    hd(depends) = fst(snd(e));
    if (isRhs) {
	depRhs(snd(snd(e)));
    } else {
	snd(snd(e)) = depExpr(line,snd(snd(e)));
    }
    leaveScope();
    return e;
}

static Void local depAlt(Cell a)		/* Find dependents of alternative  */
{
    List obvs = saveBvars();		/* Save list of bound variables	   */
    enterBtyvs();
    bindPats(rhsLine(snd(a)),fst(a));	/* add new bound vars for patterns */
    depRhs(snd(a));			/* find dependents of rhs	   */
    fst(a)    = applyBtyvs(fst(a));
    restoreBvars(obvs);			/* restore original list of bvars  */
}

static Void local depRhs(Cell r)		/* Find dependents of rhs	   */
{
    switch (whatIs(r)) {
	case GUARDED : mapProc(depGuard,snd(r));
		       break;

        case LETREC  : r = depLetRec(TRUE, rhsLine(snd(snd(r))),r);
	               break;

	case RSIGN   : snd(snd(r)) = checkPatType(rhsLine(fst(snd(r))),
						  "result",
						  rhsExpr(fst(snd(r))),
						  snd(snd(r)));
		       depRhs(fst(snd(r)));
		       break;

	default      : snd(r) = depExpr(intOf(fst(r)),snd(r));
		       break;
    }
}

static Void local depGuard(Cell g)	       /* find dependents of single guarded*/
{			       /* expression			   */
    depPair(intOf(fst(g)),snd(g));
}

Cell depExpr(Int line, Cell e)      /* find dependents of expression    */
{
    STACK_CHECK
    switch (whatIs(e)) {

	case VARIDCELL	:
	case VAROPCELL	: return depVar(line,e,TRUE);

	case CONIDCELL	:
	case CONOPCELL	: return conDefined(line,e,TRUE);

	case QUALIDENT	: if (isQVar(e)) {
			      return depQVar(line,e,TRUE);
			  } else { /* QConOrConOp */
			      return conDefined(line,e,TRUE);
			  }

	case INFIX     : return depExpr(line,tidyInfix(line,snd(e)));

	case AP 	: depPair(line,e);
			  break;

	case ZERONUM	:
	case POSNUM	:
	case NEGNUM	:
	case NAME	:
	case TUPLE	:
	case STRCELL	:
	case CHARCELL	:
	case DOUBLECELL :
	case INTCELL	: break;

	case COND	: depTriple(line,snd(e));
			  break;

	case FINLIST	: map1Over(depExpr,line,snd(e));
			  break;

        case LETREC	: e = depLetRec(FALSE,line,e);
	                  break;

	case LAMBDA	: depAlt(snd(e));
			  break;


	case DOCOMP	: /* fall-thru */
	case COMP	: depComp(line,snd(e),snd(snd(e)));
			  break;


	case ESIGN	: fst(snd(e)) = depExpr(line,fst(snd(e)));
			  snd(snd(e)) = checkSigType(line,
						     "expression",
						     fst(snd(e)),
						     snd(snd(e)));
			  break;

	case CASE	: fst(snd(e)) = depExpr(line,fst(snd(e)));
			  map1Proc(depCaseAlt,line,snd(snd(e)));
			  break;

	case CONFLDS	: depConFlds(line,e,FALSE);
			  break;

	case UPDFLDS	: depUpdFlds(line,e);
			  break;


	case ASPAT	: ERRMSG(line) "Illegal `@' in expression"
			  EEND;

	case LAZYPAT	: ERRMSG(line) "Illegal `~' in expression"
			  EEND;

	case WILDCARD	: ERRMSG(line) "Illegal `_' in expression"
			  EEND;


	default 	: internal("depExpr");
   }
   return e;
}

static Void local depPair(Int line, Cell e)	/* find dependents of pair of exprs*/
{
    fst(e) = depExpr(line,fst(e));
    snd(e) = depExpr(line,snd(e));
}

static Void local depTriple(Int line, Cell e)	/* find dependents of triple exprs */
{
    fst3(e) = depExpr(line,fst3(e));
    snd3(e) = depExpr(line,snd3(e));
    thd3(e) = depExpr(line,thd3(e));
}

static Void local depComp(Int l, Cell e, List qs)	/* find dependents of comprehension*/
{
    if (isNull(qs)) {
	fst(e) = depExpr(l,fst(e));
    } else {
	Cell q   = hd(qs);
	List qs1 = tl(qs);
	switch (whatIs(q)) {
	    case FROMQUAL : {   List obvs   = saveBvars();
				snd(snd(q)) = depExpr(l,snd(snd(q)));
				enterBtyvs();
				fst(snd(q)) = bindPat(l,fst(snd(q)));
				depComp(l,e,qs1);
				fst(snd(q)) = applyBtyvs(fst(snd(q)));
				restoreBvars(obvs);
			    }
			    break;

	    case QWHERE   :
		    snd(q)      = eqnsToBindings(snd(q),NIL,NIL,NIL);
		    withinScope(snd(q));
		    snd(q)      = dependencyAnal(snd(q));
		    hd(depends) = snd(q);
		    depComp(l,e,qs1);
		    leaveScope();
			    break;

	    case DOQUAL	  : /* fall-thru */
	    case BOOLQUAL : snd(q) = depExpr(l,snd(q));
			    depComp(l,e,qs1);
			    break;
	    default       : break;
	}
    }
}



static Void local depCaseAlt(Int line, Cell a)	/* Find dependents of case altern. */
{
    List obvs = saveBvars();		/* Save list of bound variables	   */
    enterBtyvs();
    fst(a)    = bindPat(line,fst(a));	/* Add new bound vars for pats	   */
    depRhs(snd(a));			/* Find dependents of rhs	   */
    fst(a)    = applyBtyvs(fst(a));
    restoreBvars(obvs);			/* Restore original list of bvars  */
}

static Void local checkNameAmbigName(Int line, Cell n, Bool isV)
{
    String kind = (isV ? "variable" : "data constructor");
    if (!isNull(n) && nonNull(name(n).clashes)) {
	Text t = name(n).text;
	List ls = name(n).clashes;
	ERRMSG(line) "Ambiguous %s occurrence \"%s\"", kind, textToStr(t) ETHEN
	    ERRTEXT "\n*** Could refer to: " ETHEN
	    ERRTEXT "%s.%s ", textToStr(module(name(n).mod).text), textToStr(name(n).text) ETHEN
	    for(;nonNull(ls);ls=tl(ls)) {
		ERRTEXT "%s.%s ", textToStr(module(name(hd(ls)).mod).text), textToStr(name(hd(ls)).text)
		ETHEN
	    }
	    ERRTEXT "\n" EEND;
    }
}


static Void local checkNameAmbig(Int line, Text t, Cell e)
{
    Name n;

    if (isName(e)) {
	n = e;
    } else {
	n = findName(t);
    }
    checkNameAmbigName(line,n,TRUE);
}

static Cell local checkTyconAmbig(Int line, Text t, Cell e)
{
    Tycon tc;

    if (isTycon(e)) {
	tc = e;
    } else {
	tc = findTycon(t);
    }

    if (!isNull(tc) && nonNull(tycon(tc).clashes)) {
	Text t = tycon(tc).text;
	List ls = tycon(tc).clashes;
	ERRMSG(line) "Ambiguous type constructor occurrence \"%s\"", textToStr(t) ETHEN
	    ERRTEXT "\n*** Could refer to: " ETHEN
	    ERRTEXT "%s.%s ", textToStr(module(tycon(tc).mod).text), textToStr(tycon(tc).text) ETHEN
	    for(;nonNull(ls);ls=tl(ls)) {
		ERRTEXT "%s.%s ", textToStr(module(tycon(hd(ls)).mod).text), textToStr(tycon(hd(ls)).text)
		ETHEN
	    }
	    ERRTEXT "\n" EEND;
    }
    return e;
}

static Cell local depVar(Int line, Cell e, Bool check)	/* Register occurrence of variable */
{
    List bounds1   = bounds;
    List bindings1 = bindings;
    List depends1  = depends;
    Text t	   = textOf(e);
    Cell n;

    while (nonNull(bindings1)) {
	n = varIsMember(t,hd(bounds1));   /* look for t in bound variables */
	if (nonNull(n)) {
	    return (n);
	}

	n = findBinding(t,hd(bindings1)); /* look for t in var bindings    */
	if (nonNull(n)) {
	    if (!cellIsMember(n,hd(depends1))) {
		hd(depends1) = cons(n,hd(depends1));
	    }

	    if (check) {
	      checkNameAmbig(line,t, (isVar(fst(n)) ? fst(n) : e));
	    }
	    return (isVar(fst(n)) ? fst(n) : e);
	}

	bounds1   = tl(bounds1);
	bindings1 = tl(bindings1);
	depends1  = tl(depends1);
    }

    if (isNull(n=findName(t))) {	       /* check global definitions */
	ERRMSG(line) "Undefined variable \"%s\"", textToStr(t)
	EEND;
    }
    /* Check whether there's no ambiguity about which global entity */
    if (check) {
	checkNameAmbig(line,t,e);
    }

    if (!moduleThisScript(name(n).mod)) {
	return n;
    }
    /* Later phases of the system cannot cope if we resolve references
     * to unprocessed objects too early.  This is the main reason that
     * we cannot cope with recursive modules at the moment.
     */
    return e;
}

static Cell local depQVar(Int line, Cell e, Bool isV)/* register occurrence of qualified variable */
{
    List ns = findQualNames(e);
    String kind = (isV ? "variable" : "data constructor");
    Name n;
    if (isNull(ns)) {	                        /* check global definitions */
	ERRMSG(line) "Undefined qualified %s \"%s\"", kind, identToStr(e)
	EEND;
    }
    if (!isNull(tl(ns))) {
        List ls = ns;
	ERRMSG(line) "Ambiguous qualified %s occurrence \"%s\"", kind, identToStr(e) ETHEN
        ERRTEXT "\n*** Could refer to: " ETHEN
	for(;nonNull(ls);ls=tl(ls)) {
	  ERRTEXT "%s.%s ", textToStr(module(name(hd(ls)).mod).text), textToStr(name(hd(ls)).text)
	  ETHEN
	}
	ERRTEXT "\n" EEND;
    }

    n = hd(ns);
    if (name(n).mod != currentModule) {
	return n;
    }
    if (fst(e) == VARIDCELL) {
	e = mkVar(qtextOf(e));
    } else {
	e = mkVarop(qtextOf(e));
    }
    return depVar(line,e,FALSE);
}

static Void local depConFlds(Int line, Cell e, Bool isP)/* check construction using fields */
{
    Name c = conDefined(line,fst(snd(e)),TRUE);
    if (isNull(snd(snd(e))) ||
	nonNull(cellIsMember(c,depFields(line,e,snd(snd(e)),isP)))) {
	fst(snd(e)) = c;
    } else {
	ERRMSG(line) "Constructor \"%s\" does not have selected fields in ",
		     textToStr(name(c).text)
	ETHEN ERREXPR(e);
	ERRTEXT "\n"
	EEND;
    }
    if (!isP && isPair(name(c).defn)) {	/* Check that banged fields defined*/
	List scs = fst(name(c).defn);	/* List of strict components	   */
	Type t   = name(c).type;
	Int  a   = userArity(c);
	List fs  = snd(snd(e));
	List ss;
	if (isPolyType(t)) {		/* Find tycon that c belongs to	   */
	    t = monotypeOf(t);
	}
	if (isQualType(t)) {
	    t = snd(snd(t));
	}
	if (whatIs(t)==CDICTS) {
	    t = snd(snd(t));
	}
	while (0<a--) {
	    t = arg(t);
	}
	while (isAp(t)) {
	    t = fun(t);
	}
	for (ss=tycon(t).defn; hasCfun(ss); ss=tl(ss)) {
	}
	/* Now we know the tycon t that c belongs to, and the corresponding
	 * list of selectors for that type, ss.  Now we have to check that
	 * each of the fields identified by scs appears in fs, using ss to
	 * cross reference, and convert integers to selector names.
	 */
	for (; nonNull(scs); scs=tl(scs)) {
	    Int  i   = intOf(hd(scs));
	    List ss1 = ss;
	    for (; nonNull(ss1); ss1=tl(ss1)) {
		List cns = name(hd(ss1)).defn;
		for (; nonNull(cns); cns=tl(cns)) {
		    if (fst(hd(cns))==c) {
			break;
		    }
		}
		if (nonNull(cns) && intOf(snd(hd(cns)))==i) {
		    break;
		}
	    }
	    if (isNull(ss1)) {
		internal("depConFlds");
	    } else {
		Name s   = hd(ss1);
		List fs1 = fs;
		for (; nonNull(fs1) && s!=fst(hd(fs1)); fs1=tl(fs1)) {
		}
		if (isNull(fs1)) {
		    ERRMSG(line) "Construction does not define strict field"
		    ETHEN
		    ERRTEXT      "\nExpression : " ETHEN ERREXPR(e);
		    ERRTEXT	 "\nField      : " ETHEN ERREXPR(s);
		    ERRTEXT      "\n"
		    EEND;
		}
	    }
	}
    }
}

static Void local depUpdFlds(Int line, Cell e)	/* check update using fields	   */
{
    if (isNull(thd3(snd(e)))) {
	ERRMSG(line) "Empty field list in update"
	EEND;
    }
    fst3(snd(e)) = depExpr(line,fst3(snd(e)));
    snd3(snd(e)) = depFields(line,e,thd3(snd(e)),FALSE);
}

static List local depFields(Int l, Cell e, List fs, Bool isP)	/* check field binding list	   */
{
    List cs = NIL;
    List ss = NIL;

    for (; nonNull(fs); fs=tl(fs)) {	/* for each field binding	   */
	Cell fb = hd(fs);
	Name s;

	if (isVar(fb)) {		/* expand  var  to  var = var	   */
	    fb = hd(fs) = pair(fb,fb);
	}

	s = findQualName(fst(fb));	/* check for selector		   */
	if (nonNull(s) && isSfun(s)) {
	    fst(fb) = s;
	} else {
	  if (isQualIdent(fst(fb))) {
	    ERRMSG(l) "\"%s.%s\" is not a selector function/field name",
	      textToStr(qmodOf(fst(fb))),textToStr(qtextOf(fst(fb)))
            EEND;
	  } else {
	    ERRMSG(l) "\"%s\" is not a selector function/field name",
		      textToStr(textOf(fst(fb)))
	    EEND;
	  }
	}

	if (isNull(ss)) {		/* for first named selector	   */
	    List scs = name(s).defn;	/* calculate list of constructors  */
	    for (; nonNull(scs); scs=tl(scs)) {
		cs = cons(fst(hd(scs)),cs);
	    }
	    ss = singleton(s);		/* initialize selector list	   */
	} else {			/* for subsequent selectors	   */
	    List ds = cs;		/* intersect constructor lists	   */
	    for (cs=NIL; nonNull(ds); ) {
		List scs = name(s).defn;
		while (nonNull(scs) && fst(hd(scs))!=hd(ds)) {
		    scs = tl(scs);
		}
		if (isNull(scs)) {
		    ds = tl(ds);
		} else {
		    List next = tl(ds);
		    tl(ds)    = cs;
		    cs	      = ds;
		    ds        = next;
		}
	    }

	    if (cellIsMember(s,ss)) {	/* check for repeated uses	   */
		ERRMSG(l) "Repeated field name \"%s\" in field list",
			  textToStr(name(s).text)
		EEND;
	    }
	    ss = cons(s,ss);
	}

	if (isNull(cs)) {		/* Are there any matching constrs? */
	    ERRMSG(l) "No constructor has all of the fields specified in "
	    ETHEN ERREXPR(e);
	    ERRTEXT "\n"
	    EEND;
	}

	snd(fb) = (isP ? checkPat(l,snd(fb)) : depExpr(l,snd(fb)));
    }
    return cs;
}





/* --------------------------------------------------------------------------
 * Dependency ordering, using the strongly connected components sort in
 * scc.c.  Type constructors and classes keep their dependency list in the
 * kind field until kinds are inferred; bindings keep theirs in depVal.
 * ------------------------------------------------------------------------*/

static Cell local tcDepends(Cell c)
{
    return isTycon(c) ? tycon(c).kind : cclass(c).kinds;
}

static Void local tcSetDepends(Cell c, Cell v)
{
    if (isTycon(c))
	tycon(c).kind = v;
    else
	cclass(c).kinds = v;
}

static List local tcscc(List ts, List cs)	/* scc for tycons and classes	   */
{
    return sccSort(ts,cs,tcDepends,tcSetDepends);
}

static Cell local bDepends(Cell b)
{
    return depVal(b);
}

static Void local bSetDepends(Cell b, Cell v)
{
    depVal(b) = v;
}

static List local bscc(List bs)			/* scc for bindings		   */
{
    return sccSort(bs,NIL,bDepends,bSetDepends);
}

/* --------------------------------------------------------------------------
 * Main static analysis:
 * ------------------------------------------------------------------------*/

Void checkExp() {			/* Top level static check on Expr  */
    staticAnalysis(RESET);
    clearScope();			/* Analyse expression in the scope */
    withinScope(NIL);			/* of no local bindings		   */
    inputExpr = depExpr(0,inputExpr);
    leaveScope();
    staticAnalysis(RESET);
}

#if EXPLAIN_INSTANCE_RESOLUTION
Void checkContext() {			/* Top level static check on Expr  */
    List vs, qs;

    staticAnalysis(RESET);
    clearScope();			/* Analyse expression in the scope */
    withinScope(NIL);			/* of no local bindings		   */
    qs = inputContext;
    for (vs = NIL; nonNull(qs); qs=tl(qs)) {
	vs = typeVarsIn(hd(qs),NIL,NIL,vs);
    }
    map2Proc(depPredExp,0,vs,inputContext);
    leaveScope();
    staticAnalysis(RESET);
}
#endif

Void checkDefns() {			/* Top level static analysis	   */
    List tcs;
    Module thisModule = lastModule();
    staticAnalysis(RESET);

    setCurrModule(thisModule);

    /* Resolve module references */
    mapProc(checkQualImport,  module(thisModule).modAliases);
    mapProc(checkUnqualImport,unqualImports);
    /* Add "import Prelude" if there's no explicit import */
    if ((thisModule!=modulePrelude && thisModule!=moduleUserPrelude)
	&& isNull(cellAssoc(moduleUserPrelude,unqualImports))
	&& isNull(cellRevAssoc(moduleUserPrelude,module(thisModule).modAliases)) ) {
	addUnqualImport(moduleUserPrelude,mkCon(textUserPrelude),DOTDOT);
    }
    map1Proc(checkImportList, FALSE, unqualImports);
    mapProc(checkQualImportList, module(thisModule).qualImports);

    /* And,  finally, fix-up the effective import lists. */
    fixupImportExports(module(currentModule).modImports);

    /* Note: there's a lot of side-effecting going on here, so
       don't monkey about with the order of operations here unless
       you know what you are doing */
    linkPreludeTC();			/* Get prelude tycons and classes  */
    mapProc(checkTyconDefn,tyconDefns);	/* validate tycon definitions	   */
    checkSynonyms(tyconDefns);		/* check synonym definitions	   */
    mapProc(checkClassDefn,classDefns);	/* process class definitions	   */
    tcs = tcscc(tyconDefns,classDefns); /* calc dependencies for type      */
					/* constructors and classes        */
    mapProc(kindTCGroup,tcs);		/* attach kinds	   		   */
    mapProc(visitClass,classDefns);	/* check class hierarchy	   */
    mapProc(extendFundeps,classDefns);  /* finish class definitions	   */
					/* (convenient if we do this after */
					/* calling `visitClass' so that we */
					/* know the class hierarchy is     */
					/* acyclic)                        */
    mapProc(checkClassDefn2_,tcs);	/* process class definitions again */
    mapProc(addMembers,classDefns);	/* add definitions for member funs */
    linkPreludeCM();			/* Get prelude cfuns and mfuns	   */

    mapOver(checkPrimDefn,primDefns);	/* check primitive declarations	   */

    instDefns = rev(instDefns);		/* process instance definitions	   */
    mapProc(checkInstDefn,instDefns);

    setCurrModule(thisModule);
    mapProc(addRSsigdecls,typeInDefns);	/* add sigdecls for RESTRICTSYN	   */
    valDefns   = eqnsToBindings(valDefns,tyconDefns,classDefns,primDefns);
    mapProc(allNoPrevDef,valDefns);	/* check against previous defns	   */
    mapProc(addDerivImp,derivedInsts);	/* Add impls for derived instances */
    deriveContexts(derivedInsts);	/* Calculate derived inst contexts */
    instDefns  = appendOnto(instDefns,derivedInsts);
    checkDefaultDefns();		/* validate default definitions	   */

    tyconDefns = NIL;
    primDefns  = NIL;

    if (nonNull(foreignImports) || nonNull(foreignExports)) {
        /* If generate_ffi is set, we generate a C file which defines
         * appropriate primitives.
         * Otherwise, we try to load a C file and look for those
         * primitives.
         */
        Bool need_stubs = foreignNeedStubs(foreignImports, foreignExports);

        if (generate_ffi && need_stubs) {
            foreignHeader(scriptFile);
        }
        mapProc(checkForeignImport,foreignImports);
        mapProc(checkForeignExport,foreignExports);
        if (generate_ffi && need_stubs) {
            foreignFooter(scriptFile, module(thisModule).text,
			  foreignImports, foreignExports);
        }
	if (need_stubs && (generate_ffi || !generateFFI)) {
	  needPrims(0, NULL);
	  mapProc(linkForeign,foreignImports);
	}

        /* We are now finished with foreign import declarations but
         * foreign export declarations need to pass through the
         * typechecker so that we can check that the exported type is
         * an instance of the actual type and so that we can gnerate
         * code which inserts any dictionaries we might need.
         */
        foreignImports = NIL;
    }

    /* Every top-level name has now been created - so we can build the     */
    /* export list.  Note that this has to happen before dependency        */
    /* analysis so that references to Prelude.foo will be resolved         */
    /* when compiling the prelude.                                         */
    module(thisModule).exports = checkExports(module(thisModule).exports);

    mapProc(checkTypeIn,typeInDefns);	/* check restricted synonym defns  */

    clearScope();
    withinScope(valDefns);
    valDefns = topDependAnal(valDefns); /* top level dependency ordering   */
    linkPreludeFuns();			/* Get prelude funs 	   */
    mapProc(depDefaults,classDefns);    /* dep. analysis on class defaults */
    mapProc(depInsts,instDefns);        /* dep. analysis on inst defns	   */
    leaveScope();

    /* ToDo: evalDefaults should match current evaluation module */
    evalDefaults = defaultDefns;	/* Set defaults for evaluator	   */

    /* A module's 'modImports' list is only used to construct a precise export
     * list in the presence of module re-exportation. We've now finished
     * computing the export list, so 'modImports' can now be stubbed out.
     *
     * 10/02 refinement: hold on to the 'modImports' a little bit longer;
     * until we switch to another module. This lets the user have access
     * to the effective imports when eval/querying from the read-eval-print
     * loop. However, once we switch to another module, the info is gone.
     * (i.e., there's potential for confusion wrt. the :m command.)
     *
     * This is an approximation to simply holding on to a module's
     * effective imports until it is unloaded. This isn't done due to the
     * storage overhead imposed.
     */
    staticAnalysis(RESET);
}

static Void local addRSsigdecls(Pair pr)	/* add sigdecls from TYPE ... IN ..*/
{
    List vs = snd(pr);			/* get list of variables	   */
    for (; nonNull(vs); vs=tl(vs)) {
	if (fst(hd(vs))==SIGDECL) {	/* find a sigdecl		   */
	    valDefns = cons(hd(vs),valDefns);	/* add to valDefns	   */
	    hd(vs)   = hd(snd3(snd(hd(vs))));	/* and replace with var	   */
	}
    }
}

static Void local allNoPrevDef(Cell b)	 /* ensure no previous bindings for*/
{				 /* variables in new binding	   */
    if (isVar(fst(b))) {
	noPrevDef(rhsLine(snd(hd(snd(snd(b))))),fst(b));
    } else {
	Int line = rhsLine(snd(snd(snd(b))));
	map1Proc(noPrevDef,line,fst(b));
    }
}

static Void local noPrevDef(Int line, Cell v)	 /* ensure no previous binding for */
               /* new variable		   */
{
    Name n = findName(textOf(v));

    if (isNull(n)) {
	n            = newName(textOf(v),NIL);
	name(n).defn = PREDEFINED;
    } else if (name(n).defn!=PREDEFINED && name(n).mod == currentModule) {
	/* A local repeated definition */
	duplicateError(line,name(n).mod,name(n).text,"variable");
    } else if (name(n).defn!=PREDEFINED) {
	Name oldnm = n;
	removeName(n);
	n            = newName(textOf(v),NIL);
	name(n).defn = PREDEFINED;
	name(n).clashes = cons(oldnm,name(n).clashes);
    }
    name(n).line = line;
}

static Void local duplicateError(Int line, Module mod, Text t, String kind)/* report duplicate defn */
{
    if (mod == currentModule) {
	ERRMSG(line) "Multiple declarations for %s \"%s\"", kind,
		     textToStr(t)
	EEND;
    } else {
	ERRMSG(line) "Declaration of %s \"%s\" clashes with import", kind,
		     textToStr(t)
	EEND;
    }
}

static Void local checkTypeIn(Pair cvs)	/* Check that vars in restricted   */
{				/* synonym are defined             */
    Tycon c  = fst(cvs);
    List  vs = snd(cvs);

    for (; nonNull(vs); vs=tl(vs)) {
	if (isNull(findName(textOf(hd(vs))))) {
	    ERRMSG(tycon(c).line)
		"No top level binding of \"%s\" for restricted synonym \"%s\"",
		textToStr(textOf(hd(vs))), textToStr(tycon(c).text)
	    EEND;
	}
    }
}

/* --------------------------------------------------------------------------
 * Static Analysis control:
 * ------------------------------------------------------------------------*/

/* The statics that hold cells: registered as roots at INSTALL and marked
 * by the collector on every collection */
static Cell *const staticRoots[] = {
    &patVars, &bounds, &bindings, &depends,
    &tcDeps, &derivedInsts, &diVars, &cfunSfuns,
    &unkindTypes,
};

Void staticAnalysis(Int what)
{
    switch (what) {
	case RESET   : cfunSfuns    = NIL;
		       patVars	    = NIL;
		       bounds	    = NIL;
		       bindings	    = NIL;
		       depends      = NIL;
		       tcDeps	    = NIL;
		       derivedInsts = NIL;
		       diVars	    = NIL;
		       diNum	    = 0;
		       unkindTypes  = NIL;
		       break;


	case INSTALL : registerRoots(staticRoots);
		       staticAnalysis(RESET);
		       break;
    }
}

/*-------------------------------------------------------------------------*/
