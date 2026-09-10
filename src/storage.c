/* --------------------------------------------------------------------------
 * Primitives for manipulating global data structures
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: storage.c,v $
 * $Revision: 1.86 $
 * $Date: 2004/01/06 19:45:11 $
 * ------------------------------------------------------------------------*/

#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "opts.h" /* IWYU pragma: keep -- declares profiling and profInterval, defined here under PROFILING */
#include "errors.h"
#include "machdep.h"
#include "evaluator.h" /* everybody() proto only */
#include "strutil.h"
#include "output.h" /* IWYU pragma: keep -- printPred, under PROFILING */
#include <setjmp.h>

/*#define DEBUG_SHOWUSE*/

/* --------------------------------------------------------------------------
 * local function prototypes:
 * ------------------------------------------------------------------------*/

static Int local hash(String);
static Int local saveText(Text);
static Void local hashTycon(Tycon);
static List local insertTycon(Tycon, List);
static Void local hashName(Name);
static List local insertName(Name, List);
static Void local patternError(String);
static Bool local stringMatch(String, String);
static Bool local typeInvolves(Type, Type);
static Cell local markCell(Cell);
static Cell local markSnd(Cell);
static Cell local indirectChain(Cell);
static Bool local isMarked(Cell);
static Cell local lowLevelLastIn(Cell);
static Cell local lowLevelLastOut(Cell);
static Void local freeHandle(Int);
static Void local resetStablePtrs(Void);

/* --------------------------------------------------------------------------
 * Text storage:
 *
 * provides storage for the characters making up identifier and symbol
 * names, string literals, character constants etc...
 *
 * All character strings are stored in a large character array, with textHw
 * pointing to the next free position.  Lookup in the array is improved using
 * a hash table.  Internally, text strings are represented by integer offsets
 * from the beginning of the array to the string in question.
 *
 * Where memory permits, the use of multiple hashtables gives a significant
 * increase in performance, particularly when large source files are used.
 *
 * Each string in the array is terminated by a zero byte.  No string is
 * stored more than once, so that it is safe to test equality of strings by
 * comparing the corresponding offsets.
 *
 * Special text values (beyond the range of the text array table) are used
 * to generate unique `new variable names' as required.
 *
 * The same text storage is also used to hold text values stored in a saved
 * expression.  This grows downwards from the top of the text table (and is
 * not included in the hash table).
 * ------------------------------------------------------------------------*/

#define TEXTHSZ 512                       /* Size of Text hash table         */
#define NOTEXT ((Text)(~0))               /* Empty bucket in Text hash table */
static Text textHw;                       /* Next unused position            */
static Text savedText = NUM_TEXT;         /* Start of saved portion of text  */
static Text nextNewText;                  /* Next new text value             */
static Text nextNewDText;                 /* Next new dict text value        */
static char DEFTABLE(text, NUM_TEXT);     /* Storage of character strings    */
static Text textHash[TEXTHSZ][NUM_TEXTH]; /* Hash table storage        */

String textToStr(Text t) /* find string corresp to given Text*/
{
    static char newVar[16];

    if (0 <= t && t < NUM_TEXT) /* standard char string    */
        return text + t;
    if (t < 0)
        sprintf(newVar, "d%d", -t); /* dictionary variable     */
    else
        sprintf(newVar, "v%d", t - NUM_TEXT); /* normal variable         */
    return newVar;
}

String
identToStr(Cell v) /*find string corresp to given ident or qualified name*/
{
    if (!isPair(v)) {
        internal("identToStr");
    }
    switch (fst(v)) {
    case VARIDCELL:
    case VAROPCELL:
    case CONIDCELL:
    case CONOPCELL:
        return text + textOf(v);

    case QUALIDENT: {
        Text pos = textHw;
        Text t = qmodOf(v);
        while (pos + 1 < savedText && text[t] != 0) {
            text[pos++] = text[t++];
        }
        if (pos + 1 < savedText) {
            text[pos++] = '.';
        }
        t = qtextOf(v);
        while (pos + 1 < savedText && text[t] != 0) {
            text[pos++] = text[t++];
        }
        text[pos] = '\0';
        return text + textHw;
    }
    default:
        break;
    }
    internal("identToStr2");
    return 0; /*NOTUSED*/
}

Text inventText() { /* return new unused variable name */
    return nextNewText++;
}

Text inventDictText() { /* return new unused dictvar name  */
    return nextNewDText--;
}

Bool inventedText(Text t) /* Signal TRUE if text has been    */
{                         /* generated internally            */
    return (t < 0 || t >= NUM_TEXT);
}

#define MAX_FIXLIT 100
Text fixLitText(Text t) /* fix literal text that might include \ */
{
    String s = textToStr(t);
    char p[MAX_FIXLIT];
    Int i;
    for (i = 0; i < MAX_FIXLIT - 2 && *s; s++) {
        p[i++] = *s;
        if (*s == '\\') {
            p[i++] = '\\';
        }
    }
    if (i < MAX_FIXLIT - 2) {
        p[i] = 0;
    } else {
        errHead(0);
        errPrintf("storage space exhausted for internal literal string");
        errFail();
    }
    return (findText(p));
}
#undef MAX_FIXLIT

#define MAX_TEXTLEN 4000 /* cf. MAX_TOKEN in input.c */
Text concatText(String s1, String s2) {
    char s[MAX_TEXTLEN];
    int n = snprintf(s, MAX_TEXTLEN, "%s%s", s1, s2);

    /* snprintf returns the length the string would have had, not -1, when
     * the output is truncated; only a genuine encoding error returns -1. */
    if (n < 0 || n >= MAX_TEXTLEN) {
        errHead(0);
        errPrintf("Generated name '%s%s' exceeds limit of %d", s1, s2,
                  MAX_TEXTLEN);
        errFail();
    }
    return findText(s);
}
#undef MAX_TEXTLEN

Text subText(String s, Int l) /* extract a substring and make it a Text */
{
    /* 
     * This used to insert '\0' at s[l], do a lookup and then change s[l]
     * back to its old value.  This fails if the String happens to be the
     * result of textToStr so instead we make a copy, do the lookup and 
     * forget the copy.
     */
    String t = strnCopy(s, l);
    Text r = findText(t);
    free(t);
    return r;
}

static Int local hash(String s) /* Simple hash function on strings */
{
    int v, j = 3;

    for (v = ((int)(*s)) * 8; *s; s++)
        v += ((int)(*s)) * (j++);
    if (v < 0)
        v = (-v);
    return (v % TEXTHSZ);
}

Text findText(String s) /* Locate string in Text array      */
{
    int h = hash(s);
    int hashno = 0;
    Text textPos = textHash[h][hashno];

#define TryMatch                                                               \
    {                                                                          \
        Text originalTextPos = textPos;                                        \
        String t;                                                              \
        for (t = s; *t == text[textPos]; textPos++, t++)                       \
            if (*t == '\0')                                                    \
                return originalTextPos;                                        \
    }
#define Skip                                                                   \
    while (text[textPos++])                                                    \
        ;

    while (textPos != NOTEXT) {
        TryMatch if (++hashno < NUM_TEXTH) /* look in next hashtable entry    */
            textPos = textHash[h][hashno];
        else {
            Skip while (textPos < textHw) {
                TryMatch Skip
            }
            break;
        }
    }

#undef TryMatch
#undef Skip

    textPos = textHw; /* if not found, save in array      */
    if (textHw + (Int)strlen(s) + 1 > savedText) {
        errHead(0);
        errPrintf("Character string storage space exhausted");
        errFail();
    }
    while ((text[textHw++] = *s++) != 0) {
    }
    if (hashno < NUM_TEXTH) { /* updating hash table as necessary */
        textHash[h][hashno] = textPos;
        if (hashno < NUM_TEXTH - 1)
            textHash[h][hashno + 1] = NOTEXT;
    }

    return textPos;
}

static Int local saveText(Text t) /* Save text value in buffer       */
{                                 /* at top of text table            */
    String s = textToStr(t);
    Int l = (Int)strlen(s);

    if (textHw + l + 1 > savedText) {
        errHead(0);
        errPrintf("Character string storage space exhausted");
        errFail();
    }
    savedText -= l + 1;
    strcpy(text + savedText, s);
    return savedText;
}

/* --------------------------------------------------------------------------
 * Addr storage: records `next unused program location'
 * ------------------------------------------------------------------------*/

static Addr addrHw; /* next unused program location     */

Addr getMem(Int n) /* Get some more memory             */
{
    Addr newAddr = addrHw;
    addrHw += n;
    while (addrHw >= (Int)(dynMemory->maxIdx)) {
        growMemory();
    }
    return newAddr;
}

Void nextInstr(Addr a) /* Reset point to next instruction */
{                      /* Currently does NO CHECKING      */
    addrHw = a;
}

/* --------------------------------------------------------------------------
 * Ext storage:
 *
 * Currently, the only attributes that we store for each Ext value is the
 * corresponding Text label.  At some later stage, we may decide to cache
 * types, predicates, etc. here as a space saving gesture.  Given that Text
 * comparison is cheap, and that this is an experimental implementation, we
 * will use a straightforward linear search to locate Ext values from their
 * corresponding Text labels; a hashing scheme can be introduced later if
 * this turns out to be a problem.
 * ------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
 * Tycon storage:
 *
 * A Tycon represents a user defined type constructor.  Tycons are indexed
 * by Text values ... a very simple hash function is used to improve lookup
 * times.  Tycon entries with the same hash code are chained together, with
 * the most recent entry at the front of the list.
 * ------------------------------------------------------------------------*/

#define TYCONHSZ 256                           /* Size of Tycon hash table*/
#define tHash(x) ((x) % TYCONHSZ)              /* Tycon hash function     */
static Tycon tyconHw;                          /* next unused Tycon       */
static Tycon DEFTABLE(tyconHash, TYCONHSZ);    /* Hash table storage      */
struct strTycon DEFTABLE(tabTycon, NUM_TYCON); /* Tycon storage           */

Tycon newTycon(Text t) /* add new tycon to tycon table    */
{
    Int h = tHash(t);

    if (tyconHw - TYCMIN >= NUM_TYCON) {
        errHead(0);
        errPrintf("Type constructor storage space exhausted");
        errFail();
    }
    tyconRec(tyconHw)->text = t; /* clear new tycon record          */
    tyconRec(tyconHw)->mod = currentModule;
    tyconRec(tyconHw)->kind = NIL;
    tyconRec(tyconHw)->what = NIL;
    tyconRec(tyconHw)->defn = NIL;
    moduleRec(currentModule)->tycons =
        cons(tyconHw, moduleRec(currentModule)->tycons);
    tyconRec(tyconHw)->nextTyconHash = tyconHash[h];
    tyconRec(tyconHw)->clashes = NIL;
    tyconHash[h] = tyconHw;

    return tyconHw++;
}

Tycon findTycon(Text t) /* locate Tycon in tycon table     */
{
    Tycon tc = tyconHash[tHash(t)];

    while (nonNull(tc) && tyconRec(tc)->text != t)
        tc = tyconRec(tc)->nextTyconHash;
    return tc;
}

Tycon addTycon(
    Tycon tc) /* Insert Tycon in tycon table - if no clash is caused */
{
    Tycon oldtc = findTycon(tyconRec(tc)->text);
    if (isNull(oldtc)) {
        hashTycon(tc);
        moduleRec(currentModule)->tycons =
            cons(tc, moduleRec(currentModule)->tycons);
        return tc;
    } else
        return oldtc;
}

static Void local hashTycon(Tycon tc) /* Insert Tycon into hash table    */
{
    Text t = tyconRec(tc)->text;
    Int h = tHash(t);
    tyconRec(tc)->nextTyconHash = tyconHash[h];
    tyconRec(tc)->clashes = NIL;
    tyconHash[h] = tc;
}

Tycon findQualTycon(
    Cell id) /*locate (possibly qualified) Tycon in tycon table */
{
    if (!isPair(id))
        internal("findQualTycon");
    switch (fst(id)) {
    case CONIDCELL:
    case CONOPCELL:
        return findTycon(textOf(id));
    case QUALIDENT: {
        Text t = qtextOf(id);
        List ms = findQualifiers(qmodOf(id));
        if (isNull(ms))
            return NIL;
        while (nonNull(ms)) {
            Module m = hd(ms);
            List es = NIL;
            ms = tl(ms);
            if (m == currentModule) {
                es = moduleRec(m)->tycons;
            } else {
                es = getModuleImports(m);
            }
            for (; nonNull(es); es = tl(es)) {
                Cell e = hd(es);
                if (isTycon(e) && tyconRec(e)->text == t)
                    return e;
                if (isPair(e) && isTycon(fst(e)) && tyconRec(fst(e))->text == t)
                    return fst(e);
            }
        }
        return NIL;
    }
    default:
        internal("findQualTycon2");
    }
    return NIL; /*NOTUSED*/
}

Tycon addPrimTycon(Text t, Kind kind, Int ar, Cell what,
                   Cell defn) /* add new primitive type constr   */
{
    Tycon tc = newTycon(t);
    tyconRec(tc)->line = 0;
    tyconRec(tc)->kind = kind;
    tyconRec(tc)->what = what;
    tyconRec(tc)->defn = defn;
    tyconRec(tc)->arity = ar;
    tyconRec(tc)->mod = currentModule;

    return tc;
}

static List local insertTycon(Tycon tc,
                              List ts) /* insert tycon tc into sorted list*/
                                       /* ts                              */
{
    Cell prev = NIL;
    Cell curr = ts;
    String s = textToStr(tyconRec(tc)->text);

    while (nonNull(curr) &&
           strCompare(s, textToStr(tyconRec(hd(curr))->text)) >= 0) {
        if (hd(curr) == tc) /* just in case we get duplicates! */
            return ts;
        prev = curr;
        curr = tl(curr);
    }
    if (nonNull(prev)) {
        tl(prev) = cons(tc, curr);
        return ts;
    } else
        return cons(tc, curr);
}

List addTyconsMatching(String pat,
                       List ts) /* Add tycons matching pattern pat */
                                /* to list of Tycons ts            */
{                               /* Null pattern matches every tycon*/
    Tycon tc;                   /* (Tycons with NIL kind excluded) */
    for (tc = TYCMIN; tc < tyconHw; ++tc)
        if (!pat || stringMatch(pat, textToStr(tyconRec(tc)->text)))
            if (nonNull(tyconRec(tc)->kind))
                ts = insertTycon(tc, ts);
    return ts;
}

/*
 * Remove 'tc' from a module's 'tycons' list; used to implement 
 * local overrides of imported decls.
 */
Void removeTycon(Tycon tc) {
    List ls = moduleRec(currentModule)->tycons;
    List *prev = &(moduleRec(currentModule)->tycons);

    for (; nonNull(ls); ls = tl(ls)) {
        if (hd(ls) == tc) {
            *prev = tl(ls);
            break;
        }
        prev = &(tl(ls));
    }
}

/* --------------------------------------------------------------------------
 * Name storage:
 *
 * A Name represents a top level binding of a value to an identifier.
 * Such values may be a constructor function, a member function in a
 * class, a user-defined or primitive value/function.
 *
 * Names are indexed by Text values ... a very simple hash functions speeds
 * access to the table of Names and Name entries with the same hash value
 * are chained together, with the most recent entry at the front of the
 * list.
 * ------------------------------------------------------------------------*/

#define NAMEHSZ 256                         /* Size of Name hash table */
#define nHash(x) ((x) % NAMEHSZ)            /* hash fn :: Text->Int    */
static Name nameHw;                         /* next unused name        */
static Name DEFTABLE(nameHash, NAMEHSZ);    /* Hash table storage      */
struct strName DEFTABLE(tabName, NUM_NAME); /* Name table storage      */

Name newName(Text t, Cell parent) /* Add new name to name table      */
{
    Int h = nHash(t);

    if (nameHw - NAMEMIN >= NUM_NAME) {
        errHead(0);
        errPrintf("Name storage space exhausted");
        errFail();
    }
    nameRec(nameHw)->text = t; /* clear new name record           */
    nameRec(nameHw)->line = 0;
    nameRec(nameHw)->syntax = (unsigned int)NO_SYNTAX;
    nameRec(nameHw)->parent = parent;
    nameRec(nameHw)->arity = 0;
    nameRec(nameHw)->number = EXECNAME;
    nameRec(nameHw)->defn = NIL;
    nameRec(nameHw)->type = NIL;
    nameRec(nameHw)->extFun = 0;
    nameRec(nameHw)->foreignId = -1;
    nameRec(nameHw)->foreignFlags = FFI_NOSAFETY | FFI_CCONV_UNKNOWN;
    nameRec(nameHw)->primDef = 0;
    nameRec(nameHw)->code = 0;
    nameRec(nameHw)->mod = currentModule;
    nameRec(nameHw)->clashes = NIL;

    moduleRec(currentModule)->names =
        cons(nameHw, moduleRec(currentModule)->names);
    nameRec(nameHw)->nextNameHash = nameHash[h];
    nameHash[h] = nameHw;
    return nameHw++;
}

Name findName(Text t) /* Locate name in name table       */
{
    Name n = nameHash[nHash(t)];

    while (nonNull(n) && nameRec(n)->text != t)
        n = nameRec(n)->nextNameHash;
    return n;
}

Name addName(Name nm) /* Insert Name in name table - if  */
{                     /* no clash is caused                */
    Name oldnm = findName(nameRec(nm)->text);
    if (isNull(oldnm)) {
        hashName(nm);
        moduleRec(currentModule)->names =
            cons(nm, moduleRec(currentModule)->names);
        return nm;
    } else
        return oldnm;
}

static Void local hashName(Name nm) /* Insert Name into hash table         */
{
    Text t = nameRec(nm)->text;
    Int h = nHash(t);

    nameRec(nm)->nextNameHash = nameHash[h];
    nameRec(nm)->clashes = NIL;
    nameHash[h] = nm;
}

/*
 * Remove 'n' from a module's 'names' list; used to implement 
 * local overrides of imported decls.
 */
Void removeName(Name n) {
    List ls = moduleRec(currentModule)->names;
    List *prev = &(moduleRec(currentModule)->names);

    for (; nonNull(ls); ls = tl(ls)) {
        if (hd(ls) == n) {
            *prev = tl(ls);
            break;
        }
        prev = &(tl(ls));
    }
}

Name findQualName(Cell id) /* Locate (possibly qualified) name*/
{                          /* in name table                */
    if (!isPair(id))
        internal("findQualName");
    switch (fst(id)) {
    case VARIDCELL:
    case VAROPCELL:
    case CONIDCELL:
    case CONOPCELL:
        return findName(textOf(id));
    case QUALIDENT: {
        Text t = qtextOf(id);
        List ms = findQualifiers(qmodOf(id));
        if (isNull(ms))
            return NIL;
        while (nonNull(ms)) {
            Module m = hd(ms);
            List es = NIL;
            ms = tl(ms);
            if (m == currentModule) {
                es = moduleRec(m)->names;
            } else {
                es = getModuleImports(m);
            }
            for (; nonNull(es); es = tl(es)) {
                Cell e = hd(es);
                if (isName(e) && nameRec(e)->text == t)
                    return e;
                if (isTycon(e) && tyconRec(e)->text == t)
                    return e;
                else if (isPair(e)) {
                    List subentities = NIL;
                    Cell c = fst(e);
                    if (snd(e) == DOTDOT) {
                        if (isTycon(c) && (tyconRec(c)->what == DATATYPE ||
                                           tyconRec(c)->what == NEWTYPE)) {
                            subentities = tyconRec(c)->defn;
                        } else if (isClass(c))
                            subentities = cclassRec(c)->members;
                    } else {
                        subentities = snd(e);
                    }
                    for (; nonNull(subentities);
                         subentities = tl(subentities)) {
                        if (nameRec(hd(subentities))->text == t)
                            return hd(subentities);
                    }
                }
            }
        }
        return NIL;
    }
    default:
        internal("findQualName2");
    }
    return NIL; /*NOTUSED*/
}

List findQualNames(Cell id) /* Locate (possibly qualified) names */
{                           /* in name table                 */
    if (!isPair(id))
        internal("findQualNames");
    switch (fst(id)) {
    case VARIDCELL:
    case VAROPCELL:
    case CONIDCELL:
    case CONOPCELL:
        return singleton(findName(textOf(id)));
    case QUALIDENT: {
        Text t = qtextOf(id);
        Text aliasMod = qmodOf(id);
        List ms = findQualifiers(aliasMod);
        List res = NIL;
        Bool fromHome = FALSE;
        if (isNull(ms))
            return NIL;
        while (nonNull(ms)) {
            Module m = hd(ms);
            List es = NIL;
            ms = tl(ms);
            /* For each module with alias 'aliasMod', get at
               * what 'currentModule' imports from it.
               */
            if (m == currentModule) {
                es = moduleRec(m)->names;
                fromHome = TRUE;
            } else {
                es = getModuleImports(m);
                fromHome = FALSE;
            }
            /* Chase down list looking for _unqual_ entity. */
            for (; nonNull(es); es = tl(es)) {
                Cell e = hd(es);
                if (isName(e) && nameRec(e)->text == t &&
                    !cellIsMember(e, res)) {
                    if (fromHome && nameRec(e)->mod != currentModule) {
                        /* If we're processing local names, only interested in
                       * names that were actually declared there.
                       */
                        continue;
                    }
                    res = cons(e, res);
                } else if (isPair(e)) {
                    List subentities = NIL;
                    Cell c = fst(e);
                    if (DOTDOT == snd(e)) {
                        if (isTycon(c) && (tyconRec(c)->what == DATATYPE ||
                                           tyconRec(c)->what == NEWTYPE))
                            subentities = tyconRec(c)->defn;
                        else if (isClass(c))
                            subentities = cclassRec(c)->members;
                    } else {
                        subentities = snd(e);
                    }
                    for (; nonNull(subentities);
                         subentities = tl(subentities)) {
                        if (nameRec(hd(subentities))->text == t &&
                            !cellIsMember(hd(subentities), res)) {
                            if (fromHome && nameRec(hd(subentities))->mod !=
                                                currentModule) {
                                continue;
                            }
                            res = cons(hd(subentities), res);
                        }
                    }
                }
            }
        }
        return res;
    }
    default:
        internal("findQualName2");
    }
    return NIL; /*NOTUSED*/
}

Name findQualFun(Text m, Text v) /* Locate name in name table       */
{
    Module mod = findModule(m);
    List ns;
    if (isNull(mod)) {
        return NIL;
    }
    for (ns = moduleRec(mod)->names; nonNull(ns); ns = tl(ns)) {
        Name n = hd(ns);
        if (nameRec(n)->text == v) {
            return n;
        }
    }
    return NIL;
}

/* --------------------------------------------------------------------------
 * Primitive functions:
 * ------------------------------------------------------------------------*/

struct primInfoDef {
    Module prim_module; /* module that defined prim-table */
    Bool prim_oldIO;    /* backwards compat: does the DLL's primop
                                        assume old IO rep? */
    void *prim_dll;     /* if in a dll, handle to it. */
    struct primInfo *p_info;
    struct primInfoDef
        *nextPrimInfoDef; /* subsumes nextPrimInfo (ToDo: nuke it) */
};

static Void local freePrimInfo(struct primInfoDef *);

static struct primInfoDef *firstPrimInfo = 0; /* queue of primInfo structures */
static struct primInfoDef *lastPrimInfo = 0;

/* Nasty global flag - set prior to hoisting in a DLL */
static Bool oldIO_dll = FALSE;

Bool setOldDLLFlag(Bool flg) {
    Bool res = oldIO_dll;
    oldIO_dll = flg;
    return res;
}

Void registerPrims(struct primInfo *info) /* register new primitives         */
{
    struct primInfoDef *new_entry;
    /* ToDo: this is in all likelihood not the Approved Way of asking for mem. from the OS.
     *       Figure out what's the local idiom.      
     */
    if ((new_entry = malloc(sizeof(struct primInfoDef))) == NULL) {
        /* It's going to break pretty soon anyway...*/
        return;
    } else {
        new_entry->prim_module = currentModule;
        new_entry->prim_oldIO = oldIO_dll;
        new_entry->p_info = info;
        new_entry->prim_dll = NULL;
    }
    if (0 == firstPrimInfo) /* first entry in queue                */
        firstPrimInfo = lastPrimInfo = new_entry;
    else
        lastPrimInfo = lastPrimInfo->nextPrimInfoDef = new_entry;

    lastPrimInfo->nextPrimInfoDef = 0;
}

struct primInfoDef *setPrimInfoDll(void *dll) {
    /* After a module has registered its primitives, we set
     its primInfoDef (==lastPrimInfo) to point to the DLL
     the prims are located in, so that we can later on release
     the DLL upon module unload.
  */
    lastPrimInfo->prim_dll = dll;
    return lastPrimInfo;
}

static Void local freePrimInfo(struct primInfoDef *p) {
    struct primInfoDef *info = firstPrimInfo;
    struct primInfoDef *prev = 0;

    if (p && p->prim_dll) {
        freeDLL(p->prim_dll);
    }
    while (info) {
        if (info == p) {
            /* Remove it from the list */
            if (prev) {
                prev->nextPrimInfoDef = info->nextPrimInfoDef;
            } else {
                firstPrimInfo = info->nextPrimInfoDef;
                prev = firstPrimInfo;
            }
            if (lastPrimInfo == info) {
                lastPrimInfo = prev;
            }
            free(p);
            return;
        }
        prev = info;
        info = info->nextPrimInfoDef;
    }
}

Void addPrim(Int l, Name n, String s, Module mod,
             Type ty) /* Add primitive function value    */
{
    struct primInfoDef *info_def;
    Bool lookInPrelude = TRUE;
    nameRec(n)->line = l;
    nameRec(n)->defn = NIL;
    nameRec(n)->type = ty;
    nameRec(n)->mod = mod;

    /*
     * (mini) sigh - the primitives for the Prelude'ish modules
     * are special in the sense that each such module doesn't
     * have its own prim-table (cf. Int). So, we make two passes
     * over the prim-table list, first time around we demand
     * that the defining Module for the primitive we're adding
     * matches that of the Module pinned onto the prim-table we're
     * currently looking at. If that didn't produce any matches,
     * we have another go, this time scanning through the misc
     * builtin prim-tables.
     *
     * => If a module's prim-table doesn't have an entry for
     *    the prim-decl in question, we try to fall back on
     *    any of the builtin ones. 
     *
     */
    do {
        info_def = firstPrimInfo;
        lookInPrelude = !lookInPrelude;
        for (; 0 != info_def; info_def = info_def->nextPrimInfoDef) {
            if ((lookInPrelude && isPrelude(info_def->prim_module)) ||
                mod == info_def->prim_module) {
                struct primitive *prims = info_def->p_info->primFuns;
                Int i = 0;
                for (; prims[i].ref; ++i)
                    if (strcmp(s, prims[i].ref) == 0) {
                        nameRec(n)->arity = prims[i].arity;
                        if (info_def->prim_oldIO && hasIOResultType(ty)) {
                            /* The primitive has IO type and its impl assumes the old
                        * IO rep which took a failure and success continuation. 
                        * The failure continuation is assumed unused within the
                        * DLL's primitives (reasonable assumption), so interoperating
                        * with the new representation is simply a case of adjusting
                        * the arity.
                        */
                            nameRec(n)->arity--;
                        }
                        nameRec(n)->number = EXECNAME;
                        nameRec(n)->primDef = prims[i].imp;
                        return;
                    }
            }
        }
    } while (!lookInPrelude);

    errHead(nameRec(n)->line);
    errPrintf("Unknown primitive reference \"%s\"", s);
    errFail();
}

Name addPrimCfun(Text t, Int arity, Int no,
                 Cell type) /* add primitive constructor func  */
{
    Name n = newName(t, NIL);
    nameRec(n)->arity = arity;
    nameRec(n)->number = cfunNo(no);
    nameRec(n)->type = type;
    nameRec(n)->mod = currentModule;

    return n;
}

Int sfunPos(Name s, Name c) /* Find position of field with     */
                            /* selector s in constructor c.    */
{
    List cns;
    cns = nameRec(s)->defn;
    for (; nonNull(cns); cns = tl(cns))
        if (fst(hd(cns)) == c)
            return intOf(snd(hd(cns)));
    internal("sfunPos");
    return 0; /*NOTREACHED*/
}

static List local insertName(Name nm,
                             List ns) /* insert name nm into sorted list */
                                      /* ns                              */
{
    Cell prev = NIL;
    Cell curr = ns;
    String s = textToStr(nameRec(nm)->text);

    while (nonNull(curr) &&
           strCompare(s, textToStr(nameRec(hd(curr))->text)) >= 0) {
        if (hd(curr) == nm) /* just in case we get duplicates! */
            return ns;
        prev = curr;
        curr = tl(curr);
    }
    if (nonNull(prev)) {
        tl(prev) = cons(nm, curr);
        return ns;
    } else
        return cons(nm, curr);
}

List addNamesMatching(String pat, List ns) /* Add names matching pattern pat  */
                                           /* to list of names ns             */
{                                          /* Null pattern matches every name */
    Name nm;                               /* (Names with NIL type, or hidden */
    for (nm = NAMEMIN; nm < nameHw; ++nm)  /* or invented names are excluded) */
        if (!inventedText(nameRec(nm)->text) && nonNull(nameRec(nm)->type)) {
            String str = textToStr(nameRec(nm)->text);
            if (str[0] != '_' && (!pat || stringMatch(pat, str)))
                ns = insertName(nm, ns);
        }
    return ns;
}

/* --------------------------------------------------------------------------
 * A simple string matching routine
 *     `*'    matches any sequence of zero or more characters
 *     `?'    matches any single character exactly 
 *     `@str' matches the string str exactly (ignoring any special chars)
 *     `\c'   matches the character c only (ignoring special chars)
 *     c      matches the character c only
 * ------------------------------------------------------------------------*/

static Void local patternError(String s) /* report error in pattern         */
{
    errHead(0);
    errPrintf("%s in pattern", s);
    errFail();
}

static Bool local stringMatch(String pat,
                              String str) /* match string against pattern    */
{

    for (;;)
        switch (*pat) {
        case '\0':
            return (*str == '\0');

        case '*':
            do {
                if (stringMatch(pat + 1, str))
                    return TRUE;
            } while (*str++);
            return FALSE;

        case '?':
            if (*str++ == '\0')
                return FALSE;
            pat++;
            break;

        case '[': {
            Bool found = FALSE;
            while (*++pat != '\0' && *pat != ']')
                if (!found &&
                    (pat[0] == *str ||
                     (pat[1] == '-' && pat[2] != ']' && pat[2] != '\0' &&
                      pat[0] <= *str && pat[2] >= *str)))

                    found = TRUE;
            if (*pat != ']')
                patternError("missing `]'");
            if (!found)
                return FALSE;
            pat++;
            str++;
        } break;

        case '\\':
            if (*++pat == '\0')
                patternError("extra trailing `\\'");
            /*fallthru!*/
        default:
            if (*pat++ != *str++)
                return FALSE;
            break;
        }
}

/* --------------------------------------------------------------------------
 * Storage of type classes, instances etc...:
 * ------------------------------------------------------------------------*/

static Class classHw; /* next unused class                */
static List classes;  /* list of classes in current scope */
static Inst instHw;   /* next unused instance record      */

struct strClass DEFTABLE(tabClass, NUM_CLASSES); /* Class storage        */
struct strInst DEFTABLE(tabInst, NUM_INSTS);     /* Instance storage     */

Class newClass(Text t) /* add new class to class table     */
{
    if (classHw - CLASSMIN >= NUM_CLASSES) {
        errHead(0);
        errPrintf("Class storage space exhausted");
        errFail();
    }

    cclassRec(classHw)->text = t;
    cclassRec(classHw)->line = 0;
    cclassRec(classHw)->arity = 0;
    cclassRec(classHw)->tyvars = NIL;
    cclassRec(classHw)->kinds = NIL;
    cclassRec(classHw)->head = NIL;
    cclassRec(classHw)->fds = NIL;
    cclassRec(classHw)->xfds = NIL;
    cclassRec(classHw)->dcon = NIL;
    cclassRec(classHw)->supers = NIL;
    cclassRec(classHw)->numSupers = 0;
    cclassRec(classHw)->dsels = NIL;
    cclassRec(classHw)->members = NIL;
    cclassRec(classHw)->numMembers = 0;
    cclassRec(classHw)->defaults = NIL;
    cclassRec(classHw)->instances = NIL;
    cclassRec(classHw)->clashes = NIL;
    classes = cons(classHw, classes);
    cclassRec(classHw)->mod = currentModule;
    moduleRec(currentModule)->classes =
        cons(classHw, moduleRec(currentModule)->classes);

    return classHw++;
}

Class classMax() {  /* Return max Class in use ...     */
    return classHw; /* This is a bit ugly, but it's not*/
} /* worth a lot of effort right now */

Class findClass(Text t) /* look for named class in table    */
{
    Class cl;
    List cs;
    for (cs = classes; nonNull(cs); cs = tl(cs)) {
        cl = hd(cs);
        if (cclassRec(cl)->text == t)
            return cl;
    }
    return NIL;
}

Class addClass(Class c) /* Insert Class in class list      */
{                       /*  - if no clash caused           */
    Class oldc = findClass(cclassRec(c)->text);
    if (isNull(oldc)) {
        classes = cons(c, classes);
        moduleRec(currentModule)->classes =
            cons(c, moduleRec(currentModule)->classes);
        return c;
    } else
        return oldc;
}

Class findQualClass(Cell c) /* Look for (possibly qualified)   */
{                           /* class in class list                 */
    if (!isQualIdent(c)) {
        return findClass(textOf(c));
    } else {
        Text t = qtextOf(c);
        List ms = findQualifiers(qmodOf(c));
        if (isNull(ms))
            return NIL;
        while (nonNull(ms)) {
            Module m = hd(ms);
            List es = NIL;
            ms = tl(ms);
            if (m == currentModule) {
                es = moduleRec(m)->classes;
            } else {
                es = getModuleImports(m);
            }
            for (; nonNull(es); es = tl(es)) {
                Cell e = hd(es);
                if (isClass(e) && cclassRec(e)->text == t)
                    return e;
                if (isPair(e) && isClass(fst(e)) &&
                    cclassRec(fst(e))->text == t)
                    return fst(e);
            }
        }
    }
    return NIL;
}

Inst newInst() { /* Add new instance to table        */
    if (instHw - INSTMIN >= NUM_INSTS) {
        errHead(0);
        errPrintf("Instance storage space exhausted");
        errFail();
    }
    instRec(instHw)->kinds = NIL;
    instRec(instHw)->head = NIL;
    instRec(instHw)->specifics = NIL;
    instRec(instHw)->implements = NIL;
    instRec(instHw)->builder = NIL;

    return instHw++;
}

Inst findFirstInst(Tycon tc) /* look for 1st instance involving */
{                            /* the type constructor tc         */
    return findNextInst(tc, INSTMIN - 1);
}

Inst findNextInst(Tycon tc, Inst in) /* look for next instance involving*/
                                     /* the type constructor tc         */
{                                    /* starting after instance in      */
    while (++in < instHw) {
        Cell pi = instRec(in)->head;
        for (; isAp(pi); pi = fun(pi))
            if (typeInvolves(arg(pi), tc))
                return in;
    }
    return NIL;
}

static Bool local typeInvolves(Type ty,
                               Type tc) /* Test to see if type ty involves */
                                        /* type constructor/tuple tc.      */
{
    return (ty == tc) || (isAp(ty) && (typeInvolves(fun(ty), tc) ||
                                       typeInvolves(arg(ty), tc)));
}

/* --------------------------------------------------------------------------
 * Control stack:
 *
 * Various parts of the system use a stack of cells.  Most of the stack
 * operations are defined as macros, expanded inline.
 * ------------------------------------------------------------------------*/

Cell DEFTABLE(cellStack, NUM_STACK); /* Storage for cells on stack          */
StackPtr sp;                         /* stack pointer                       */

#if GIMME_STACK_DUMPS

# define UPPER_DISP 5 /* # display entries on top of stack   */
# define LOWER_DISP 5 /* # display entries on bottom of stack*/

extern Int rootsp;
extern Cell evalRoots[];

Void cuddlesStackOverflow() { /* Report stack overflow               */
    errHead(0);
    errPrintf("Control stack overflow");
    if (rootsp >= 0) {
        Int i;
        if (rootsp >= UPPER_DISP + LOWER_DISP) {
            for (i = 0; i < UPPER_DISP; i++) {
                errPrintf("\nwhile evaluating: ");
                errExpr(evalRoots[rootsp - i]);
            }
            errPrintf("\n...");
            for (i = LOWER_DISP - 1; i >= 0; i--) {
                errPrintf("\nwhile evaluating: ");
                errExpr(evalRoots[i]);
            }
        } else {
            for (i = rootsp; i >= 0; i--) {
                errPrintf("\nwhile evaluating: ");
                errExpr(evalRoots[i]);
            }
        }
    }
    /* No newline here: each entry above opens with one, and errFail ends the
     * message, so the two versions of this function print the same thing
     * apart from the trace itself. */
    errFail();
}

#else /* !GIMME_STACK_DUMPS */

Void cuddlesStackOverflow() { /* Report stack overflow               */
    errHead(0);
    errPrintf("Control stack overflow");
    errFail();
}

#endif /* !GIMME_STACK_DUMPS */

/* --------------------------------------------------------------------------
 * Observation storage
 * ------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
 * Module storage:
 *
 * A Module represents a user defined module.  
 *
 * Note: there are now two lookup mechanisms in the system:
 *
 * 1) The exports from a module are stored in a big list.
 *    We resolve qualified names, and import lists by linearly scanning
 *    through this list.
 *
 * 2) Unqualified imports and local definitions for the current module
 *    are stored in hash tables (tyconHash and nameHash) or linear lists
 *    (classes).
 *
 * ------------------------------------------------------------------------*/

static Module moduleHw; /* next unused Module              */
struct strModule DEFTABLE(tabModule, NUM_MODULE); /* Module storage         */
Module currentModule; /* Module currently being processed*/

Bool isValidModule(Module m) /* is m a legitimate module id?     */
{
    return (MODMIN <= m && m < moduleHw);
}

Module newModule(Text t) /* add new module to module table  */
{
    if (moduleHw - MODMIN >= NUM_MODULE) {
        errHead(0);
        errPrintf("Module storage space exhausted");
        errFail();
    }
    moduleRec(moduleHw)->text = t; /* clear new module record         */
    moduleRec(moduleHw)->tycons = NIL;
    moduleRec(moduleHw)->names = NIL;
    moduleRec(moduleHw)->classes = NIL;
    moduleRec(moduleHw)->exports = NIL;
    moduleRec(moduleHw)->modAliases = NIL;
    moduleRec(moduleHw)->modImports = NIL;
    moduleRec(moduleHw)->qualImports = NIL;
    return moduleHw++;
}

Module findModule(Text t) /* locate Module in module table  */
{
    Module m;
    for (m = MODMIN; m < moduleHw; ++m) {
        if (moduleRec(m)->text == t)
            return m;
    }
    return NIL;
}

Module findModid(Cell c) /* Find module by name or filename  */
{
    switch (whatIs(c)) {
    case STRCELL: {
        Script s = scriptThisFile(snd(c));
        return (s == -1) ? NIL : moduleOfScript(s);
    }
    case CONIDCELL:
        return findModule(textOf(c));
    default:
        internal("findModid");
    }
    return NIL; /*NOTUSED*/
}

/* Don't use - use findQualifiers() instead */
Module findQualifier(Text t) /* locate Module in alias list */
{
    Module ms;
    for (ms = moduleRec(currentModule)->modAliases; nonNull(ms); ms = tl(ms)) {
        if (textOf(fst(hd(ms))) == t) {
            return snd(hd(ms));
        }
    }
    if (moduleRec(currentModule)->text == t) {
        return currentModule;
    }
    return NIL;
}

/*
 * locate Modules with a local alias 't' in the import list.
 * Notice that more than one module may map to the same local
 * alias.
 */
List findQualifiers(Text t) /* locate Modules in alias list */
{
    List ms;
    List res = NIL;

    for (ms = moduleRec(currentModule)->modAliases; nonNull(ms); ms = tl(ms)) {
        if (textOf(fst(hd(ms))) == t && !cellIsMember(snd(hd(ms)), res)) {
            res = cons(snd(hd(ms)), res);
        }
    }
    /* Legal? "module Foo where { import Bar as Foo; ... }" */
    if (moduleRec(currentModule)->text == t) {
        res = cons(currentModule, res);
    }
    return res;
}

Text findModAlias(Text t) /* given a module name 't', locate its alias. */
{
    List ms;
    for (ms = moduleRec(currentModule)->modAliases; nonNull(ms); ms = tl(ms)) {
        if (moduleRec(snd(hd(ms)))->text == t) {
            return textOf(fst(hd(ms)));
        }
    }
    return NIL;
}

Void setCurrModule(Module m) /* set lookup tables for current module */
{
    Int i;
    if (m != currentModule) {
        /* The effective module import list of a module
           is held onto until we switch to another. Ideally,
           we'd keep around the effective import lists for
           a module all the time (so that if a user uses
           the :m command to evaluate/query in the context
           of a different module, all the imported entities
           are in scope.) However, the run-time cost of 
           keeping these large lists around is considerable,
           so we approximate.
        */
        if (currentModule != NIL) {
            moduleRec(currentModule)->modImports = NIL;
        }
        currentModule = m; /* This is the only assignment to currentModule */
        for (i = 0; i < TYCONHSZ; ++i)
            tyconHash[i] = NIL;
        mapProc(hashTycon, moduleRec(m)->tycons);
        for (i = 0; i < NAMEHSZ; ++i)
            nameHash[i] = NIL;
        mapProc(hashName, moduleRec(m)->names);
        classes = moduleRec(m)->classes;
    }
}

List getModuleImports(
    Module m) /* In 'currentModule', look up imports from 'm'. */
{
    Module ms = moduleRec(currentModule)->modImports;

    for (; nonNull(ms); ms = tl(ms)) {
        if (isPair(hd(ms)) && fst(hd(ms)) == m) {
            return (snd(hd(ms)));
        }
    }
    return NIL;
}

/* --------------------------------------------------------------------------
 * Script file storage:
 *
 * script files are read into the system one after another.  The state of
 * the stored data structures (except the garbage-collected heap) is recorded
 * before reading a new script.  In the event of being unable to read the
 * script, or if otherwise requested, the system can be restored to its
 * original state immediately before the file was read.
 *
 * That state is the marks half of the script records, which script.c shares;
 * see the script file control section of storage.h.  scriptHw counts the
 * records whose marks have been taken, so script scriptHw-1 is the one being
 * read, or the anonymous record startNewScript(0) makes to give an
 * expression at the prompt something to recover to.
 * ------------------------------------------------------------------------*/

struct strScript DEFTABLE(tabScript, NUM_SCRIPTS); /* Script storage       */

static Script scriptHw; /* number of scripts whose marks   */
                        /* have been recorded              */

Script startNewScript(String f) /* start new script, keeping record */
{                               /* of status for later restoration  */
    struct strScriptMarks *m;

    if (scriptHw >= NUM_SCRIPTS) {
        errHead(0);
        errPrintf("Too many module files (maximum of %d allowed)", NUM_SCRIPTS);
        errFail();
    }

    m = &scriptRec(scriptHw)->marks;
    m->file = findText(f ? f : "<nofile>");
    m->textHw = textHw;
    m->nextNewText = nextNewText;
    m->nextNewDText = nextNewDText;
    m->addrHw = addrHw;
    m->moduleHw = moduleHw;
    m->tyconHw = tyconHw;
    m->nameHw = nameHw;
    m->classHw = classHw;
    m->instHw = instHw;
    m->prims = NULL;

    return scriptHw++;
}

Bool isPreludeScript() { /* Test whether script 0 is being read */
    return (scriptHw <= 1);
}

Bool moduleThisScript(Module m) /* Test if given module is defined */
{                               /* in current script file          */
    return scriptHw < 1 || m >= scriptRec(scriptHw - 1)->marks.moduleHw;
}

Module lastModule() { /* Return module in current script file */
    return (moduleHw > MODMIN ? moduleHw - 1 : modulePrelude);
}

/* Each table grows as the scripts are read, so the script that made entry
 * x is the last one whose mark for that table is at or below x; entries
 * below the first mark are built in and belong to the Prelude.
 */
static Script local
scriptThisMark(Cell x, Cell (*markOf)(const struct strScriptMarks *)) {
    Script s = 1;
    while (s < scriptHw && x >= markOf(&scriptRec(s)->marks))
        s++;
    return s - 1;
}

static Cell local markName(const struct strScriptMarks *m) {
    return m->nameHw;
}
static Cell local markTycon(const struct strScriptMarks *m) {
    return m->tyconHw;
}
static Cell local markInst(const struct strScriptMarks *m) {
    return m->instHw;
}
static Cell local markClass(const struct strScriptMarks *m) {
    return m->classHw;
}

Script scriptThisName(Name x) {
    return scriptThisMark(x, markName);
}
Script scriptThisTycon(Tycon x) {
    return scriptThisMark(x, markTycon);
}
Script scriptThisInst(Inst x) {
    return scriptThisMark(x, markInst);
}
Script scriptThisClass(Class x) {
    return scriptThisMark(x, markClass);
}

Module moduleOfScript(Script s) {
    /* The Prelude's module is made before its script is read, so script 0
     * does not follow the rule that a script's module is the one its marks
     * were taken at.                                                    */
    return (s == 0) ? modulePrelude : scriptRec(s)->marks.moduleHw;
}

String fileOfModule(Module m) {
    Script s;
    if (m == modulePrelude) {
        return findMPathname(STD_PRELUDE_CUDDLES);
    }
    for (s = 1; s < scriptHw; ++s) {
        if (scriptRec(s)->marks.moduleHw == m) {
            return textToStr(scriptRec(s)->marks.file);
        }
    }
    return 0;
}

Script scriptThisFile(Text f) {
    Script s;
    for (s = 1; s < scriptHw; ++s) {
        if (scriptRec(s)->marks.file == f) {
            return s;
        }
    }
    if (f == textPrelude) {
        return 0;
    }
    return (-1);
}

Void dropAScript(Script sno) /* elide one script from the stack */
{
    Script i;

    if (sno < 0 || sno >= scriptHw)
        return;

    if (scriptRec(sno)->marks.prims) {
        freePrimInfo(scriptRec(sno)->marks.prims);
        scriptRec(sno)->marks.prims = 0;
    }
    for (i = sno + 1; i < scriptHw; i++)
        scriptRec(i - 1)->marks = scriptRec(i)->marks;
    /* The last slot's marks were just copied down, so it is now a
     * duplicate: shrink the stack so that nothing iterates over it again.
     * Without this, scriptHw still counted it, and both it and the slot it
     * was copied into held the same "prims" DLL handle, which
     * dropScriptsFrom/EXIT would then free twice.                       */
    scriptHw--;
}

Void dropScriptsFrom(Script sno)      /* Restore storage to state prior  */
{                                     /* to reading script sno           */
    if (sno >= 0 && sno < scriptHw) { /* is there anything to restore?   */
        struct strScriptMarks *m = &scriptRec(sno)->marks;
        int i;
        textHw = m->textHw;
        nextNewText = m->nextNewText;
        nextNewDText = m->nextNewDText;
        moduleHw = m->moduleHw;
        addrHw = m->addrHw;
        tyconHw = m->tyconHw;
        nameHw = m->nameHw;
        classHw = m->classHw;
        instHw = m->instHw;

        for (i = 0; i < TEXTHSZ; ++i) {
            int j = 0;
            while (j < NUM_TEXTH && textHash[i][j] != NOTEXT &&
                   textHash[i][j] < textHw)
                ++j;
            if (j < NUM_TEXTH)
                textHash[i][j] = NOTEXT;
        }

        currentModule = NIL;
        for (i = 0; i < TYCONHSZ; ++i) {
            tyconHash[i] = NIL;
        }
        for (i = 0; i < NAMEHSZ; ++i) {
            nameHash[i] = NIL;
        }

        for (i = CLASSMIN; i < classHw; i++) {
            List ins = cclassRec(i)->instances;
            List is = NIL;

            while (nonNull(ins)) {
                List temp = tl(ins);
                if (hd(ins) < instHw) {
                    tl(ins) = is;
                    is = ins;
                }
                ins = temp;
            }
            cclassRec(i)->instances = rev(is);
        }

        for (i = sno; i < scriptHw; i++) {
            if (scriptRec(i)->marks.prims) {
                freePrimInfo(scriptRec(i)->marks.prims);
                scriptRec(i)->marks.prims = 0;
            }
        }
        scriptHw = sno;
    }
}

Void setScriptPrims(void *p) /* set the current script's primitive record. */
{
    scriptRec(scriptHw - 1)->marks.prims = (struct primInfoDef *)p;
    return;
}

/* --------------------------------------------------------------------------
 * Heap storage:
 *
 * Provides a garbage collectable heap for storage of expressions etc.
 * ------------------------------------------------------------------------*/

Int heapSize = DEFAULTHEAP; /* number of cells in heap         */
Heap heapFst;               /* array of fst component of pairs */
Heap heapSnd;               /* array of snd component of pairs */
Heap heapTopFst;
Heap heapTopSnd;
Bool consGC = TRUE; /* Set to FALSE to turn off gc from*/
                    /* C stack; use with extreme care! */
#if PROFILING
Heap heapThd, heapTopThd;     /* to keep record of producers     */
Int sysCount;                 /* record unattached cells         */
Name producer;                /* current producer, if any        */
Bool profiling = FALSE;       /* should profiling be performed   */
Int profInterval = MAXPOSINT; /* interval between samples        */
FILE *profile = 0;            /* pointer to profiler log, if any */
#endif
Long numCells;
Int numGcs;         /* number of garbage collections   */
Int cellsRecovered; /* number of cells recovered       */

static Cell freeList;     /* free list of unused cells       */
static Cell lsave, rsave; /* save components of pair         */

static List weakPtrs; /* list of weak ptrs                 */
                      /* reconstructed during every GC   */
List finalizers = NIL;
List liveWeakPtrs = NIL;

#define startGC()
#define endGC()

#define initStackRoots()
#define recordStackRoot()

#define start()
#define end(thing, root)
#define recordMark()

unsigned char whatCode[N_WHATCODE]; /* array of whatIs codes */

#define fillWhatCode(code, from, to) memset(whatCode + from, code, to - from)

static Void initFastWhat(Void) {
    int i;

    for (i = 0; i < TUPMIN; i++)
        whatCode[i] = i;
    fillWhatCode(TUPLE, TUPMIN, OFFMIN);
    fillWhatCode(OFFSET, OFFMIN, MODMIN);
    fillWhatCode(MODULE, MODMIN, TYCMIN);
    fillWhatCode(TYCON, TYCMIN, NAMEMIN);
    fillWhatCode(NAME, NAMEMIN, INSTMIN);
    fillWhatCode(INSTANCE, INSTMIN, CLASSMIN);
    fillWhatCode(CLASS, CLASSMIN, CHARMIN);
    fillWhatCode(CHARCELL, CHARMIN, INTMIN);
    fillWhatCode(INTCELL, INTMIN, INTMAX + 1); /* +1 is intentional */
}

#undef fillWhatCode

Cell pair(Cell l, Cell r) /* Allocate pair (l, r) from       */
{                         /* heap, garbage collecting first  */
    Cell c = freeList;    /* if necessary ...                */

    if (isNull(c)) {
        lsave = l;
        rsave = r;
        garbageCollect();
        l = lsave;
        lsave = NIL;
        r = rsave;
        rsave = NIL;
        c = freeList;
    }
    freeList = snd(freeList);
    fst(c) = l;
    snd(c) = r;
#if PROFILING
    thd(c) = producer;
#endif
    numCells++;
    return c;
}

Void overwrite(Cell dst, Cell src) /* overwrite dst cell with src cell*/
{                                  /* both *MUST* be pairs            */
    if (isPair(dst) && isPair(src)) {
        fst(dst) = fst(src);
        snd(dst) = snd(src);
    } else
        internal("overwrite");
}

static unsigned *marks;
static Int marksSize;

/* --------------------------------------------------------------------------
 * Roots: the tables of static cells registered by the modules (see
 * registerRoots in storage.h).
 * ------------------------------------------------------------------------*/

struct rootTable {
    Cell *const *cells;
    Int count;
};

static struct rootTable *rootTables = 0;
static Int numRootTables = 0;
static Int maxRootTables = 0;

Void gcRegisterRoots(Cell *const *cells, Int count) {
    if (numRootTables == maxRootTables) {
        Int more = maxRootTables ? 2 * maxRootTables : 16;
        struct rootTable *grown = realloc(rootTables, more * sizeof(*grown));
        if (!grown) {
            errHead(0);
            errPrintf("Cannot allocate root table");
            errFail();
        }
        rootTables = grown;
        maxRootTables = more;
    }
    rootTables[numRootTables].cells = cells;
    rootTables[numRootTables].count = count;
    numRootTables++;
}

static Void local markRoots(Void) {
    Int t, i;
    for (t = 0; t < numRootTables; t++)
        for (i = 0; i < rootTables[t].count; i++)
            *rootTables[t].cells[i] = markExpr(*rootTables[t].cells[i]);
}

Cell markExpr(Cell c) /* External interface to markCell  */
{
    return isGenPair(c) ? markCell(c) : c;
}

/* --------------------------------------------------------------------------
 * markCell and markSnd used to recurse in C, once per cell, along the fst
 * link of the object graph (with markSnd looping, rather than recursing,
 * along a run of snd links -- see the old comments in the jj history of
 * this file).  A long, thin part of the graph can still be many cells
 * deep along the fst link: every list cons cell's fst is itself an
 * application cell, so a long list, or the left-nested chain of thunks
 * that a lazy foldl-shaped function leaves behind, used to recurse once
 * per cell and could overflow the C stack.  Longjmp'ing out of the
 * collector is not safe, so stackCheck() is not used here; instead both
 * links are marked from an explicit work stack of pending slots, kept in
 * a malloc'd array that is grown (doubled) as needed and kept between
 * collections.
 * ------------------------------------------------------------------------*/

static Cell **markStack = 0;  /* malloc'd stack of pending Cell* slots*/
static Int markStackSize = 0; /* capacity of markStack, in entries    */
static Int markStackTop = 0;  /* number of entries currently in use   */

static Void local growMarkStack(Void) /* Double the mark stack, reporting  */
{                                     /* a fatal error if malloc fails --  */
    Int newSize = markStackSize ? 2 * markStackSize : 1024;
    /* longjmp'ing out of the collector  */
    /* is not safe here.                 */
    Cell **newStack = (Cell **)realloc(markStack, newSize * sizeof(Cell *));
    if (newStack == 0) {
        fatal("Cannot grow garbage collector mark stack");
    }
    markStack = newStack;
    markStackSize = newSize;
}

#define pushMarkSlot(slotp)                                                    \
    do {                                                                       \
        if (markStackTop >= markStackSize)                                     \
            growMarkStack();                                                   \
        markStack[markStackTop++] = (slotp);                                   \
    } while (0)

static Void local markSlot(Cell *slot) /* Mark whatever *slot refers to,  */
{                   /* pushing whichever of its own fst/snd links     */
    Cell c = *slot; /* still need marking.  Used for the fst and snd  */
                    /* link of a cell, and for markCell's root arg.  */
    if (!isPair(c)) /* fst/snd links pushed here are not guaranteed  */
        return;     /* to be pairs (unlike markCell's root); a snd  */
                    /* link commonly terminates the chain here.      */
mc:
    switch (fst(c)) {
    case INDIRECT:
        c = indirectChain(c);
        if (isGenPair(c))
            goto mc;
        *slot = c;
        return;
    case WEAKCELL: {
        register int place = placeInSet(c);
        register unsigned mask = maskInSet(c);
        if (!(marks[place] & mask)) {
            marks[place] |= mask;
            marks[placeInSet(snd(c))] |= maskInSet(snd(c));
            /* printf("Found Weak Pointer %d\n", c); */
            nextWeakPtr(c) = weakPtrs;
            weakPtrs = c;
            recordMark();
        }
        *slot = c; /* c may have come through an      */
        return;    /* indirection chain               */
    }
    default:
        break;
    }
    *slot = c; /* the indirection chain above may have been resolved   */
               /* (possibly through several INDIRECT cells) before      */
               /* falling out of the switch here; write the final,      */
               /* indirection-free cell back so the caller's link no    */
               /* longer keeps the (now unreachable) indirection nodes  */
               /* alive for the sweep that follows marking.             */

    {
        register int place = placeInSet(c);
        register unsigned mask = maskInSet(c);
        if (marks[place] & mask)
            return;
        else {
            marks[place] |= mask;
            recordMark();
        }
    }

    if (isGenPair(fst(c))) {
        pushMarkSlot(&snd(c)); /* the fst link is popped first, so a long */
        pushMarkSlot(&fst(c)); /* list leaves one pending snd link, not   */
    } /* one pending fst link per element        */
    else if (isNull(fst(c)) || fst(c) >= BCSTAG) {
        pushMarkSlot(&snd(c));
    }
}

static Cell markCellRoot; /* Static (not stack) storage for markCell's own */
                          /* root slot: pushMarkSlot needs the address of  */
                          /* a slot that stays valid for the whole drain,  */
                          /* which a local variable's address is not.      */

static Cell local markCell(Cell c) /* Traverse part of graph marking  */
{                                  /* cells reachable from given root */
                                   /* markCell(c) is only called if c */
                                   /* is a pair                       */
    markCellRoot = c;
    pushMarkSlot(&markCellRoot);
    while (markStackTop > 0) {
        markSlot(markStack[--markStackTop]);
    }
    return markCellRoot;
}

static Cell local markSnd(Cell c) /* Variant of markCell used to     */
{                                 /* mark cells reachable from the snd       */
    /* link of c; c itself is assumed already  */
    /* marked.                                 */
    pushMarkSlot(&snd(c));
    while (markStackTop > 0) {
        markSlot(markStack[--markStackTop]);
    }
    return c;
}

static Cell local indirectChain(Cell c) /* Scan chain of indirections      */
{                                       /* Detecting loops of indirections */
    Cell is = c;                        /* Uses pointer reversal ...       */
    c = snd(is);
    snd(is) = NIL;
    fst(is) = INDIRECT1;

    while (isPair(c) && fst(c) == INDIRECT) {
        register Cell temp = snd(c);
        snd(c) = is;
        is = c;
        c = temp;
        fst(is) = INDIRECT1;
    }

    if (isPair(c) && fst(c) == INDIRECT1)
        c = nameBlackHole;

    do {
        register Cell temp = snd(is);
        fst(is) = INDIRECT;
        snd(is) = c;
        is = temp;
    } while (nonNull(is));

    return c;
}

Void markWithoutMove(Cell n) /* Garbage collect cell at n, as if*/
{                            /* it was a cell ref, but don't    */
                             /* move cell (i.e. retain INDIRECT */
                             /* at top level) so we don't have  */
                             /* to modify the stored value of n */
    if (isGenPair(n)) {
        recordStackRoot();
        if (fst(n) == INDIRECT) { /* special case for indirections   */
            register int place = placeInSet(n);
            register unsigned mask = maskInSet(n);
            marks[place] |= mask;
            recordMark();
            markSnd(n);
        } else
            markCell(n); /* normal pairs don't move anyway  */
    }
}

static Bool local isMarked(Cell c) {
    if (isGenPair(c)) {
        Int place = placeInSet(c);
        unsigned mask = maskInSet(c);
        return (marks[place] & mask) != 0;
    } else {
        return TRUE;
    }
}

Void garbageCollect() {              /* Run garbage collector ...       */
    Bool breakStat = breakOn(FALSE); /* disable break checking          */
    Int i, j;
    register unsigned mask;
    register Int place;
    Int recovered;
    jmp_buf regs; /* save registers on stack         */
    setjmp(regs);

    gcStarted();
    for (i = 0; i < marksSize; ++i) /* initialise mark set to empty    */
        marks[i] = 0;
    weakPtrs = NIL;  /* clear list of weak pointers     */
    everybody(MARK); /* Mark all components of system   */
    markRoots();     /* and the registered roots        */

    for (i = 0; i < (Int)num_handles; ++i) /* release any unused handles      */
        if (nonNull(handles[i].hcell)) {
            register int place = placeInSet(handles[i].hcell);
            register unsigned mask = maskInSet(handles[i].hcell);
            if ((marks[place] & mask) == 0)
                freeHandle(i);
        }
    for (i = mallocPtr_hw - 1; i >= 0;
         i--) { /* release any unused mallocptrs   */
        if (isPair(mallocPtrs[i].mpcell)) {
            register int place = placeInSet(mallocPtrs[i].mpcell);
            register unsigned mask = maskInSet(mallocPtrs[i].mpcell);
            if ((marks[place] & mask) == 0)
                incMallocPtrRefCnt(i, -1);
        }
    }

    /* After GC completes, we scan the list of weak pointers that are
     * still live and zap their contents unless the contents are still
     * live (by some other means).
     * Note that this means the contents must itself be heap allocated.
     * This means it can't be a nullary constructor or an Int or a Name
     * or lots of other things - hope this doesn't bite too hard.
     */
    for (; nonNull(weakPtrs); weakPtrs = nextWeakPtr(weakPtrs)) {
        Cell ptr = derefWeakPtr(weakPtrs);
        if (isGenPair(ptr)) {
            Int place = placeInSet(ptr);
            unsigned mask = maskInSet(ptr);
            if ((marks[place] & mask) == 0) {
                /* printf("Zapping weak pointer %d\n", ptr); */
                derefWeakPtr(weakPtrs) = NIL;
            } else {
                /* printf("Keeping weak pointer %d\n", ptr); */
            }
        } else if (nonNull(ptr)) {
            printf("Weak ptr contains object which isn't heap allocated %d\n",
                   ptr);
        }
    }

    if (nonNull(liveWeakPtrs) || nonNull(finalizers)) {
        Bool anyMarked; /* Weak pointers with finalizers   */
        List wps;
        List newFins = NIL;

        /* Step 1: iterate until we've found out what is reachable         */
        do {
            anyMarked = FALSE;
            for (wps = liveWeakPtrs; nonNull(wps); wps = tl(wps)) {
                Cell wp = hd(wps);
                Cell k = fst(snd(wp));
                if (isNull(k)) {
                    internal("bad weak ptr");
                }
                if (isMarked(k)) {
                    Cell vf = snd(snd(wp));
                    if (!isMarked(fst(vf)) || !isMarked(snd(vf))) {
                        mark(fst(vf));
                        mark(snd(vf));
                        anyMarked = TRUE;
                    }
                }
            }
        } while (anyMarked);

        /* Step 2: Now we know which weak pointers will die, so we can     */
        /* remove them from the live set and gather their finalizers.  But */
        /* note that we mustn't mark *anything* at this stage or we will   */
        /* corrupt our view of what's alive, and what's dead.              */
        wps = NIL;
        while (nonNull(liveWeakPtrs)) {
            Cell wp = hd(liveWeakPtrs);
            List nx = tl(liveWeakPtrs);
            Cell k = fst(snd(wp));
            if (!isMarked(k)) {         /* If the key is dead, then*/
                Cell vf = snd(snd(wp)); /* stomp on weak pointer   */
                if (nonNull(snd(vf))) {
                    fst(vf) = snd(vf);
                    snd(vf) = newFins;
                    newFins = vf; /* reuse because we can't  */
                } /* reallocate here ...           */
                fst(snd(wp)) = NIL;
                snd(snd(wp)) = NIL;
                snd(wp) = NIL;
                liveWeakPtrs = nx;
            } else {
                tl(liveWeakPtrs) = wps; /* Otherwise, weak pointer */
                wps = liveWeakPtrs;     /* survives to face another*/
                liveWeakPtrs = nx;      /* garbage collection      */
            }
        }
        liveWeakPtrs = wps;

        /* Step 3: Now we've identified the live cells and the newly       */
        /* scheduled finalizers, but we had better make sure that they are */
        /* all marked now, including any internal structure, to ensure that*/
        /* they make it to the other side of gc.                           */
        for (; nonNull(wps); wps = tl(wps)) {
            mark(snd(hd(wps)));
        }
        mark(liveWeakPtrs);
        mark(newFins);
        finalizers = revOnto(newFins, finalizers);
    }

    gcScanning(); /* scan mark set                   */
    mask = 1;
    place = 0;
    recovered = 0;
    j = 0;
#if PROFILING
    if (profile) {
        sysCount = 0;
        for (i = NAMEMIN; i < nameHw; i++)
            nameRec(i)->count = 0;
    }
#endif
    freeList = NIL;
    for (i = 1; i <= heapSize; i++) {
        if ((marks[place] & mask) == 0) {
            snd(-i) = freeList;
            fst(-i) = FREECELL;
            freeList = -i;
            recovered++;
        }
#if PROFILING
        else if (nonNull(thd(-i)))
            nameRec(thd(-i))->count++;
        else
            sysCount++;
#endif
        mask <<= 1;
        if (++j == bitsPerWord) {
            place++;
            mask = 1;
            j = 0;
        }
    }

    gcRecovered(recovered);
    breakOn(breakStat); /* restore break trapping if nec.  */

#if PROFILING
    if (profile) {
        fprintf(profile, "BEGIN_SAMPLE %ld.00\n", numReductions);
        /* For the time being, we won't include the system count in the output:
        if (sysCount>0)
            fprintf(profile,"  SYSTEM %d\n",sysCount);
*/
        /* Accumulate costs in top level objects */
        for (i = NAMEMIN; i < nameHw; i++) {
            Name cc = i;
            /* Use of "while" instead of "if" is pure paranoia - ADR */
            while (isName(nameRec(cc)->parent))
                cc = nameRec(cc)->parent;
            if (i != cc) {
                nameRec(cc)->count += nameRec(i)->count;
                nameRec(i)->count = 0;
            }
        }
        for (i = NAMEMIN; i < nameHw; i++)
            if (nameRec(i)->count > 0) {
                if (isPair(nameRec(i)->parent)) {
                    Pair p = nameRec(i)->parent;
                    Cell f = fst(p);
                    fprintf(profile, "  ");
                    if (isClass(f))
                        fprintf(profile, "%s", textToStr(cclassRec(f)->text));
                    else {
                        fprintf(profile, "%s_",
                                textToStr(cclassRec(instRec(f)->c)->text));
                        /* Will hp2ps accept the spaces produced by this? */
                        printPred(profile, instRec(f)->head);
                    }
                    fprintf(profile, "_%s %d\n",
                            textToStr(nameRec(snd(p))->text),
                            nameRec(i)->count);
                } else {
                    fprintf(profile, "  %s %d\n", textToStr(nameRec(i)->text),
                            nameRec(i)->count);
                }
            }
        fprintf(profile, "END_SAMPLE %ld.00\n", numReductions);
    }
#endif

    /* can only return if freeList is nonempty on return. */
    if (recovered < minRecovery || isNull(freeList)) {
        errHead(0);
        errPrintf("Garbage collection fails to reclaim sufficient space");
        errFail();
    }
    numGcs++;
    cellsRecovered = recovered;
}

#if PROFILING
Void profilerLog(String s) /* turn heap profiling on, saving log*/
{                          /* in specified file                 */
    if ((profile = fopen(s, "w")) != NULL) {
        fprintf(profile, "JOB \"Cuddles Heap Profile\"\n");
        fprintf(profile, "DATE \"%s\"\n", timeString());
        fprintf(profile, "SAMPLE_UNIT \"reductions\"\n");
        fprintf(profile, "VALUE_UNIT \"cells\"\n");
    } else {
        errHead(0);
        errPrintf("Cannot open profile log file \"%s\"", s);
        errFail();
    }
}
#endif

/* --------------------------------------------------------------------------
 * Code for saving last expression entered:
 *
 * This is a little tricky because some text values (e.g. strings or variable
 * names) may not be defined or have the same value when the expression is
 * recalled.  These text values are therefore saved in the top portion of
 * the text table.
 * ------------------------------------------------------------------------*/

static Cell lastExprSaved; /* last expression to be saved     */

Void setLastExpr(Cell e) /* save expression for later recall*/
{
    lastExprSaved = NIL; /* in case attempt to save fails   */
    savedText = NUM_TEXT;
    lastExprSaved = lowLevelLastIn(e);
}

static Cell local lowLevelLastIn(Cell c) /* Duplicate expression tree (i.e. */
{                                        /* acyclic graph) for later recall */
    if (isPair(c))                       /* Duplicating any text strings    */
        if (isBoxTag(fst(c)))            /* in case these are lost at some  */
            switch (fst(c)) {            /* point before the expr is reused */
            case VARIDCELL:
            case VAROPCELL:
            case DICTVAR:
            case CONIDCELL:
            case CONOPCELL:
            case STRCELL:
                return pair(fst(c), saveText(textOf(c)));
            default:
                return pair(fst(c), snd(c));
            }
        else
            return pair(lowLevelLastIn(fst(c)), lowLevelLastIn(snd(c)));
    else
        return c;
}

Cell getLastExpr() { /* recover previously saved expr   */
    return lowLevelLastOut(lastExprSaved);
}

static Cell local lowLevelLastOut(Cell c) /* As with lowLevelLastIn() above  */
{                                         /* except that Cells refering to   */
    if (isPair(c))                        /* Text values are restored to     */
        if (isBoxTag(fst(c)))             /* appropriate values              */
            switch (fst(c)) {
            case VARIDCELL:
            case VAROPCELL:
            case DICTVAR:
            case CONIDCELL:
            case CONOPCELL:
            case STRCELL:
                return pair(fst(c), findText(text + intValOf(c)));
            default:
                return pair(fst(c), snd(c));
            }
        else
            return pair(lowLevelLastOut(fst(c)), lowLevelLastOut(snd(c)));
    else
        return c;
}

/* --------------------------------------------------------------------------
 * Miscellaneous operations on heap cells:
 * ------------------------------------------------------------------------*/

/* Profiling suggests that the number of calls to whatIs() is typically    */
/* rather high.  The recoded version below attempts to improve the average */
/* performance for whatIs() using a binary search for part of the analysis */

#if DEBUG_PRINTER
/* A very, very simple printer.
 * Output is uglier than from printExp - but the printer is more
 * robust and can be used on any data structure irrespective of
 * its type.
 */
Void printList(List, Int);
Void print(Cell, Int);
Void print(Cell c, Int depth) {
    if (0 == depth) {
        Printf("...");
    } else {
        Int tag = whatIs(c);
        switch (tag) {
        case AP:
            Putchar('(');
            print(fst(c), depth - 1);
            Putchar(',');
            print(snd(c), depth - 1);
            Putchar(')');
            break;
        case FREECELL:
            Printf("free(%d)", c);
            break;
        case INTCELL:
            Printf("int(%d)", intOf(c));
            break;
        case FLOATCELL:
            Printf("float(%f)", floatOf(c));
            break;
        case DOUBLECELL:
            Printf("double(%f)", doubleOf(c));
            break;
        case CHARCELL:
            Printf("char('%c')", charOf(c));
            break;
        case CLASS:
            Printf("class(%d)", c - CLASSMIN);
            if (CLASSMIN <= c && c < classHw)
                Printf("=\"%s\"", textToStr(cclassRec(c)->text));
            break;
        case INSTANCE:
            Printf("instance(%d)", c - INSTMIN);
            break;
        case NAME:
            Printf("name(%d)", c - NAMEMIN);
            if (NAMEMIN <= c && c < nameHw)
                Printf("=\"%s\"", textToStr(nameRec(c)->text));
            break;
        case TYCON:
            Printf("tycon(%d)", c - TYCMIN);
            if (TYCMIN <= c && c < tyconHw)
                Printf("=\"%s\"", textToStr(tyconRec(c)->text));
            break;
        case MODULE:
            Printf("module(%d)", c - MODMIN);
            break;
        case OFFSET:
            Printf("Offset %d", offsetOf(c));
            break;
        case TUPLE:
            Printf("Tuple %d", tupleOf(c));
            break;
        case NIL:
            Printf("NIL");
            break;
        case DOTDOT:
            Printf("DOTDOT");
            break;
        case DICTVAR:
            Printf("{dict %d}", textOf(c));
            break;
        case VARIDCELL:
        case VAROPCELL:
        case CONIDCELL:
        case CONOPCELL:
            Printf("{id %s}", textToStr(textOf(c)));
            break;
        case QUALIDENT:
            Printf("{qid %s.%s}", textToStr(qmodOf(c)), textToStr(qtextOf(c)));
            break;
        case LETREC:
            Printf("LetRec(");
            print(fst(snd(c)), depth - 1);
            Putchar(',');
            print(snd(snd(c)), depth - 1);
            Putchar(')');
            break;
        case BIGLAM:
            Printf("BigLam(");
            printList(fst(snd(c)), depth - 1);
            Putchar(',');
            print(snd(snd(c)), depth - 1);
            Putchar(')');
            break;
        case ESIGN:
            Printf("ESign(");
            print(fst(snd(c)), depth - 1);
            Putchar(',');
            print(snd(snd(c)), depth - 1);
            Putchar(')');
            break;
        case COMP:
            Printf("COMP(");
            print(fst(snd(c)), depth - 1);
            Putchar(',');
            print(snd(snd(c)), depth - 1);
            Putchar(')');
            break;
        case FROMQUAL:
            Printf("FROMQUAL(");
            print(fst(snd(c)), depth - 1);
            Putchar(',');
            print(snd(snd(c)), depth - 1);
            Putchar(')');
            break;
        default:
            if (isBoxTag(tag)) {
                Printf("Tag(%d)=%d", c, tag);
            } else if (isConTag(tag)) {
                Printf("%d@(%d,", c, tag);
                print(snd(c), depth - 1);
                Putchar(')');
                break;
            } else if (c == tag) {
                Printf("Tag(%d)", c);
            } else {
                Printf("Tag(%d)=%d", c, tag);
            }
            break;
        }
    }
    FlushStdout();
}

Void printList(List l, Int depth) {
    Int tag;
    Cell n;
    Putchar('[');
    if ((tag = whatIs(l)) != NIL)
        for (;; l = n) {
            if (tag == AP) {
                print(fst(l), depth - 1);
                n = snd(l);
                tag = whatIs(n);
                if (tag == NIL)
                    break;
                Putchar(',');
            } else {
                Printf("NotAList!\n");
                break;
            }
        }
    Putchar(']');
}
#endif

Bool isVar(Cell c) /* is cell a VARIDCELL/VAROPCELL ? */
{                  /* also recognises DICTVAR cells   */
    return isPair(c) &&
           (fst(c) == VARIDCELL || fst(c) == VAROPCELL || fst(c) == DICTVAR);
}

Bool isCon(Cell c) /* is cell a CONIDCELL/CONOPCELL ?  */
{
    return isPair(c) && (fst(c) == CONIDCELL || fst(c) == CONOPCELL);
}

Bool isQVar(Cell c) /* is cell a [un]qualified varop/id? */
{
    if (!isPair(c))
        return FALSE;
    switch (fst(c)) {
    case VARIDCELL:
    case VAROPCELL:
        return TRUE;

    case QUALIDENT:
        return isVar(snd(snd(c)));

    default:
        return FALSE;
    }
}

Bool isQCon(Cell c) /*is cell a [un]qualified conop/id? */
{
    if (!isPair(c))
        return FALSE;
    switch (fst(c)) {
    case CONIDCELL:
    case CONOPCELL:
        return TRUE;

    case QUALIDENT:
        return isCon(snd(snd(c)));

    default:
        return FALSE;
    }
}

Bool isQualIdent(Cell c) /* is cell a qualified identifier?  */
{
    return isPair(c) && (fst(c) == QUALIDENT);
}

Bool isIdent(Cell c) /* is cell an identifier?           */
{
    if (!isPair(c))
        return FALSE;
    switch (fst(c)) {
    case VARIDCELL:
    case VAROPCELL:
    case CONIDCELL:
    case CONOPCELL:
        return TRUE;

    case QUALIDENT:
        return TRUE;

    default:
        return FALSE;
    }
}

Bool isInt(Cell c) /* cell holds integer value?        */
{
    return isSmall(c) || (isPair(c) && fst(c) == INTCELL);
}

Int intOf(Cell c) /* find integer value of cell?      */
{
    return isPair(c) ? (Int)(snd(c)) : (Int)(c - INTZERO);
}

Cell mkInt(Int n) /* make cell representing integer   */
{
    return (MINSMALLINT <= n && n <= MAXSMALLINT) ? INTZERO + n
                                                  : pair(INTCELL, n);
}

/* --------------------------------------------------------------------------
 * Unicode Characters: NUM_SHORT_CHARS is the number of one-cell characters.
 * Like integers, the constructor, the selector, and the predicate
 * are defined as external functions implemented in storage.c
 * For characters with value less than NUM_SHORT_CHARS, one cell will be
 * allocated. For greater character values, a pair of cells will be
 * allocated. The pair of cells will have CHARCELL as fst, and
 * the UNICODE value of the character as snd.
 * ------------------------------------------------------------------------*/

Bool isChar(Cell c) /* cell holds character value?      */
{
    return isShortChar(c) || (isPair(c) && fst(c) == CHARCELL);
}

Char charOf(Cell c) /* find character value of cell?    */
{
    return isPair(c) ? (Char)(snd(c)) : (Char)(c - CHARMIN);
}

Cell mkChar(Char c) /* make cell representing character */
{
    return (0 <= c && c < NUM_SHORT_CHARS) ? CHARMIN + c : pair(CHARCELL, c);
}

Bool isBignum(Cell c) /* cell holds bignum value?         */
{
    return c == ZERONUM ||
           (isPair(c) && (fst(c) == POSNUM || fst(c) == NEGNUM));
}

#if SIZEOF_INTP == SIZEOF_INT
typedef union {
    Int i;
    Pointer p;
} IntOrPointer;
Cell mkPtr(Pointer p) {
    IntOrPointer x;
    x.p = p;
    return pair(PTRCELL, x.i);
}

Pointer ptrOf(Cell c) {
    IntOrPointer x;
    assert(fst(c) == PTRCELL);
    x.i = snd(c);
    return x.p;
}
#elif SIZEOF_INTP == 2 * SIZEOF_INT
typedef union {
    struct {
        Int i1;
        Int i2;
    } i;
    Pointer p;
} IntOrPointer;
Cell mkPtr(Pointer p) {
    IntOrPointer x;
    x.p = p;
    return pair(PTRCELL, pair(mkInt(x.i.i1), mkInt(x.i.i2)));
}

Pointer ptrOf(Cell c) {
    IntOrPointer x;
    assert(fst(c) == PTRCELL);
    x.i.i1 = intOf(fst(snd(c)));
    x.i.i2 = intOf(snd(snd(c)));
    return x.p;
}
#else
# warning "type Ptr not supported on this architecture - don't use it"
Cell mkPtr(Pointer p) {
    errHead(0);
    errPrintf("mkPtr: type Ptr not supported on this architecture");
    errFail();
}

Pointer ptrOf(Cell c) {
    errHead(0);
    errPrintf("ptrOf: type Ptr not supported on this architecture");
    errFail();
}
#endif

/* --------------------------------------------------------------------------
 * List operations:
 * ------------------------------------------------------------------------*/

Int length(List xs) /* calculate length of list xs      */
{
    Int n = 0;
    for (; nonNull(xs); ++n)
        xs = tl(xs);
    return n;
}

List appendOnto(List xs, List ys) /* Destructively prepend xs onto    */
{                                 /* ys by modifying xs ...           */
    if (isNull(xs))
        return ys;
    else {
        List zs = xs;
        while (nonNull(tl(zs)))
            zs = tl(zs);
        tl(zs) = ys;
        return xs;
    }
}

List dupOnto(List xs,
             List ys) /* non-destructively prepend xs backwards onto ys */
{
    for (; nonNull(xs); xs = tl(xs))
        ys = cons(hd(xs), ys);
    return ys;
}

List dupList(List xs) /* Duplicate spine of list xs       */
{
    List ys = NIL;
    for (; nonNull(xs); xs = tl(xs))
        ys = cons(hd(xs), ys);
    return rev(ys);
}

List dupUpto(List xs, Cell c) /* Duplicate spine of list xs       */
{
    List ys = NIL;
    for (; nonNull(xs) && hd(xs) != c; xs = tl(xs))
        ys = cons(hd(xs), ys);
    return rev(ys);
}

List revOnto(List xs, List ys) /* Destructively reverse elements of*/
{                              /* list xs onto list ys...          */
    Cell zs;

    while (nonNull(xs)) {
        zs = tl(xs);
        tl(xs) = ys;
        ys = xs;
        xs = zs;
    }
    return ys;
}

List concat(List xss) {
    List xs = NIL;
    for (; nonNull(xss); xss = tl(xss))
        xs = dupOnto(hd(xss), xs);
    return xs;
}

List intersect(List xs, List ys) {
    List zs = NIL;
    for (; nonNull(xs); xs = tl(xs)) {
        if (varIsMember(textOf(hd(xs)), ys))
            zs = cons(hd(xs), zs);
    }
    return zs;
}

Cell varIsMember(Text t, List xs) /* Test if variable is a member of  */
                                  /* given list of variables          */
{
    for (; nonNull(xs); xs = tl(xs))
        if (t == textOf(hd(xs)))
            return hd(xs);
    return NIL;
}

Name nameIsMember(Text t, List ns) /* Test if name with text t is a   */
                                   /* member of list of names xs           */
{
    for (; nonNull(ns); ns = tl(ns)) {
        if (t == nameRec(hd(ns))->text) {
            return hd(ns);
        }
    }
    return NIL;
}

/* Locating entities in import/export lists */

Name nameInIEList(Name nm, List ns) /* Test if Name is a member of */
                                    /* of import/export list xs.   */
{
    Text t = nameRec(nm)->text;
    Bool isMethod = isClass(nameRec(nm)->parent);
    Bool isDCon = isTycon(nameRec(nm)->parent) && !findTycon(nameRec(nm)->text);

    for (; nonNull(ns); ns = tl(ns)) {
        if (isName(hd(ns)) && t == nameRec(hd(ns))->text) {
            return hd(ns);
        }
        if ((isMethod && isPair(hd(ns)) && isClass(fst(hd(ns)))) ||
            (isDCon && isPair(hd(ns)) && isTycon(fst(hd(ns))) &&
             (tyconRec(fst(hd(ns)))->what != SYNONYM) &&
             (tyconRec(fst(hd(ns)))->what != RESTRICTSYN))) {
            Name r;
            List subs = snd(hd(ns));
            if (subs == DOTDOT) {
                if (isClass(fst(hd(ns)))) {
                    subs = cclassRec(fst(hd(ns)))->members;
                } else if (isTycon(fst(hd(ns)))) {
                    subs = tyconRec(fst(hd(ns)))->defn;
                }
            }
            r = nameInIEList(nm, subs);
            if (r)
                return r;
        }
    }
    return NIL;
}

Tycon tyconInIEList(Text t, List ns) /* Test if Tycon with text t is a  */
                                     /* member of import/export list xs */
{
    for (; nonNull(ns); ns = tl(ns)) {
        if (isTycon(hd(ns)) && t == tyconRec(hd(ns))->text) {
            return hd(ns);
        } else if (isPair(hd(ns)) && isTycon(fst(hd(ns))) &&
                   t == tyconRec(fst(hd(ns)))->text) {
            return fst(hd(ns));
        }
    }
    return NIL;
}

Class classInIEList(Text t, List ns) /* Test if Class with text t is a  */
                                     /* member of import/export list xs */
{
    for (; nonNull(ns); ns = tl(ns)) {
        if (isClass(hd(ns)) && t == cclassRec(hd(ns))->text) {
            return hd(ns);
        }
        if (isPair(hd(ns)) && isClass(fst(hd(ns))) &&
            t == cclassRec(fst(hd(ns)))->text) {
            return fst(hd(ns));
        }
    }
    return NIL;
}

List nubList(List ls) /* (non-destructively) remove duplicates from list */
{
    List res = NIL;

    while (nonNull(ls)) {
        if (!cellIsMember(hd(ls), res)) {
            res = cons(hd(ls), res);
        }
        ls = tl(ls);
    }
    return res;
}

Cell intIsMember(Int n, List xs) /* Test if integer n is member of   */
                                 /* given list of integers           */
{
    for (; nonNull(xs); xs = tl(xs))
        if (n == intOf(hd(xs)))
            return hd(xs);
    return NIL;
}

Cell cellIsMember(Cell x, List xs) /* Test for membership of specific  */
                                   /* cell x in list xs                */
{
    for (; nonNull(xs); xs = tl(xs))
        if (x == hd(xs))
            return hd(xs);
    return NIL;
}

Cell cellAssoc(Cell c, List xs) /* Lookup cell in association list  */
{
    for (; nonNull(xs); xs = tl(xs))
        if (c == fst(hd(xs)))
            return hd(xs);
    return NIL;
}

Cell cellRevAssoc(Cell c, List xs) /* Lookup cell in range of          */
                                   /* association lists            */
{
    for (; nonNull(xs); xs = tl(xs))
        if (c == snd(hd(xs)))
            return hd(xs);
    return NIL;
}

List replicate(Int n, Cell x) /* create list of n copies of x      */
{
    List xs = NIL;
    while (0 < n--)
        xs = cons(x, xs);
    return xs;
}

List diffList(List from, List take) /* list difference: from\take       */
{                                   /* result contains all elements of  */
    List result = NIL;              /* `from' not appearing in `take'   */

    while (nonNull(from)) {
        List next = tl(from);
        if (!cellIsMember(hd(from), take)) {
            tl(from) = result;
            result = from;
        }
        from = next;
    }
    return rev(result);
}

List deleteCell(List xs, Cell y) /* copy xs deleting pointers to y  */
{
    List result = NIL;
    for (; nonNull(xs); xs = tl(xs)) {
        Cell x = hd(xs);
        if (x != y) {
            result = cons(x, result);
        }
    }
    return rev(result);
}

List take(Int n, List xs) /* destructively truncate list to  */
                          /* specified length                */
{
    List ys = xs;

    if (n == 0)
        return NIL;
    while (1 < n-- && nonNull(xs))
        xs = tl(xs);
    if (nonNull(xs))
        tl(xs) = NIL;
    return ys;
}

List splitAt(Int n, List xs) /* drop n things from front of list*/
{
    for (; n > 0; --n) {
        xs = tl(xs);
    }
    return xs;
}

Cell nth(Int n, List xs) /* extract n'th element of list    */
{
    for (; n > 0 && nonNull(xs); --n, xs = tl(xs)) {
    }
    if (isNull(xs))
        internal("nth");
    return hd(xs);
}

List removeCell(Cell x, List xs) /* destructively remove cell from  */
                                 /* list                            */
{
    if (nonNull(xs)) {
        if (hd(xs) == x)
            return tl(xs); /* element at front of list        */
        else {
            List prev = xs;
            List curr = tl(xs);
            for (; nonNull(curr); prev = curr, curr = tl(prev))
                if (hd(curr) == x) {
                    tl(prev) = tl(curr);
                    return xs; /* element in middle of list       */
                }
        }
    }
    return xs; /* here if element not found       */
}

/* --------------------------------------------------------------------------
 * Operations on applications:
 * ------------------------------------------------------------------------*/

Int argCount; /* number of args in application    */

Cell getHead(Cell e) /* get head cell of application     */
{                    /* set number of args in argCount   */
    for (argCount = 0; isAp(e); e = fun(e))
        argCount++;
    return e;
}

List getArgs(Cell e) /* get list of arguments in function*/
{                    /* application:                     */
    List as;         /* getArgs(f e1 .. en) = [e1,..,en] */

    for (as = NIL; isAp(e); e = fun(e))
        as = cons(arg(e), as);
    return as;
}

Cell nthArg(Int n, Cell e) /* return nth arg in application    */
                           /* of function to m args (m>=n)     */
{                          /* nthArg n (f x0 x1 ... xm) = xn   */
    for (n = numArgs(e) - n - 1; n > 0; n--)
        e = fun(e);
    return arg(e);
}

Int numArgs(Cell e) /* find number of arguments to expr */
{
    Int n;
    for (n = 0; isAp(e); e = fun(e))
        n++;
    return n;
}

Cell applyToArgs(Cell f, List args) /* destructively apply list of args */
                                    /* to function f                    */
{
    while (nonNull(args)) {
        Cell temp = tl(args);
        tl(args) = hd(args);
        hd(args) = f;
        f = args;
        args = temp;
    }
    return f;
}

/* --------------------------------------------------------------------------
 * Handle table - declared here for the GC to access.
 * ------------------------------------------------------------------------*/

DynTable *dynTabHandles = NULL;

struct strHandle *handles;
unsigned long num_handles = 0;

/* --------------------------------------------------------------------------
 * Freeing a handle.
 * ------------------------------------------------------------------------*/
static Void local freeHandle(Int n) /* release handle storage when no  */
{                                   /* heap references to it remain    */
    if (0 <= n && n < (Int)num_handles && nonNull(handles[n].hcell)) {
        if (n > HSTDERR && handles[n].hmode != HCLOSED && handles[n].hfp) {
            fclose(handles[n].hfp);
            handles[n].hfp = 0;
        }
        fst(handles[n].hcell) = snd(handles[n].hcell) = NIL;
        handles[n].hcell = NIL;
    }
}

/* --------------------------------------------------------------------------
 * Malloc Ptrs:
 * ------------------------------------------------------------------------*/

struct strMallocPtr mallocPtrs[NUM_MALLOCPTRS];

/* Points to the next available slot in 'mallocPtrs'. */
int mallocPtr_hw;

/* It might GC (because it uses a table not a list) which will trash any
 * unstable pointers.  
 * (It happens that we never use it with unstable pointers.)
 */
Cell newMallocPtr(Pointer ptr) /* create a new malloc pointer     */
{
    Int i;
    for (i = 0; i < NUM_MALLOCPTRS && mallocPtrs[i].refCount != 0; ++i)
        ;                      /* Search for unused entry */
    if (i >= NUM_MALLOCPTRS) { /* If at first we don't    */
        garbageCollect();      /* succeed, garbage collect*/
        for (i = 0; i < NUM_MALLOCPTRS && mallocPtrs[i].refCount != 0; ++i)
            ; /* and try again ...         */
    }
    if (i >= NUM_MALLOCPTRS) { /* ... before we give up   */
        errHead(0);
        errPrintf("Too many ForeignPtrs open");
        errFail();
    }
    mallocPtrs[i].ptr = ptr;
    mallocPtrs[i].finalizers = NIL;
    mallocPtrs[i].refCount = 1;

    /* adjust the high-water mark for the table. */
    if (i >= mallocPtr_hw) {
        mallocPtr_hw = i + 1;
    }
    return (mallocPtrs[i].mpcell = ap(MPCELL, i));
}

Cell mkMallocPtr(Pointer ptr,
                 CFinalizer cleanup) /* create a new malloc pointer    */
{
    Cell mp;
    Int i;
    mp = newMallocPtr(ptr);
    i = mpOf(mp);
    mallocPtrs[i].finalizers =
        cons(mkPtr((Pointer)cleanup), mallocPtrs[i].finalizers);
    return mp;
}

Void incMallocPtrRefCnt(Int n, Int i) /* change ref count of MallocPtr */
{
    if (!(0 <= n && n < NUM_MALLOCPTRS && mallocPtrs[n].refCount > 0))
        internal("freeMallocPtr");
    mallocPtrs[n].refCount += i;
    if (mallocPtrs[n].refCount <= 0) {
        Cell p;
        for (p = mallocPtrs[n].finalizers; nonNull(p); p = tl(p)) {
            Cell fin = hd(p);
            if (isPtr(fin))
                ((CFinalizer)ptrOf(fin))(mallocPtrs[n].ptr);
            else
                ((CFinalizerEnv)ptrOf(fst(fin)))(ptrOf(snd(fin)),
                                                 mallocPtrs[n].ptr);
        }

        mallocPtrs[n].ptr = 0;
        mallocPtrs[n].finalizers = NIL;
        mallocPtrs[n].refCount = 0;
        mallocPtrs[n].mpcell = NIL;

        /* Freed the slot next to the high-water mark; adjust the marker. */
        if ((n + 1) == mallocPtr_hw) {
            do {
                --mallocPtr_hw;
            } while (mallocPtr_hw > 0 &&
                     isNull(mallocPtrs[mallocPtr_hw - 1].mpcell));
        }
    }
}

/* --------------------------------------------------------------------------
 * Stable pointers
 * This is a mechanism that allows the C world to manipulate pointers into the
 * Haskell heap without having to worry that the garbage collector is going
 * to delete it or move it around.
 * The implementation and interface is based on my implementation in
 * GHC - but, at least for now, is simplified by using a fixed size
 * table of stable pointers.
 * ------------------------------------------------------------------------*/

/* Each entry in the stable pointer table is either a heap pointer
 * or is not currently allocated.
 * Unallocated entries are threaded together into a freelist.
 * The last entry in the list contains the Cell 0; all other values
 * contain a Cell whose value is the next free stable ptr in the list.
 * It follows that stable pointers are strictly positive (>0).
 */
static Cell stablePtrTable[NUM_STABLEPTRS];
static Bool stablePtrFree[NUM_STABLEPTRS]; /* whether slot sp is on the    */
                                           /* free list, to catch double  */
                                           /* frees (the free list link   */
                                           /* and a live Cell share the   */
                                           /* same slot, so there's no    */
                                           /* other way to tell them apart*/
static Int sptFreeList;
#define SPT(sp) stablePtrTable[(sp) - 1]
#define SPTFREE(sp) stablePtrFree[(sp) - 1]

static Void local resetStablePtrs() {
    Int i;
    /* It would be easier to build the free list in the other direction
     * but, when debugging, it's way easier to understand if the first
     * pointer allocated is "1".
     */
    for (i = 1; i < NUM_STABLEPTRS; ++i) {
        SPT(i) = i + 1;
        SPTFREE(i) = TRUE;
    }
    SPT(NUM_STABLEPTRS) = 0;
    SPTFREE(NUM_STABLEPTRS) = TRUE;
    sptFreeList = 1;
}

Int mkStablePtr(Cell c) /* Create a stable pointer            */
{
    Int i = sptFreeList;
    if (i == 0)
        return 0;
    sptFreeList = SPT(i);
    SPT(i) = c;
    SPTFREE(i) = FALSE;
    return i;
}

Cell derefStablePtr(Int p) /* Dereference a stable pointer       */
{
    if (!(1 <= p && p <= NUM_STABLEPTRS)) {
        internal("derefStablePtr");
    }
    return SPT(p);
}

Void freeStablePtr(Int i) /* Free a stable pointer             */
{
    if (!(1 <= i && i <= NUM_STABLEPTRS)) {
        internal("freeStablePtr");
    }
    if (SPTFREE(i)) /* freeing an already-free pointer is a no-op, the */
        return;     /* same as C's free(); it must not corrupt the    */
                    /* free list by threading it into itself           */
    SPT(i) = sptFreeList;
    SPTFREE(i) = TRUE;
    sptFreeList = i;
}

#undef SPT
#undef SPTFREE

/* --------------------------------------------------------------------------
 * storage control:
 * ------------------------------------------------------------------------*/

DynTable *allocDynTable(unsigned long eltSize, unsigned long maxIdx,
                        unsigned long hWater, const char *tabName) {
    DynTable *tab = (DynTable *)malloc(sizeof(struct strDynTable));

    if (tab != NULL) {
        tab->maxIdx = maxIdx;
        tab->hWater = hWater;
        tab->tabName = tabName;
        tab->eltSize = eltSize;
        tab->data = (void *)malloc(eltSize * maxIdx);
    }

    if (tab == NULL || tab->data == NULL) {
        errHead(0);
        errPrintf("Cannot allocate dynamic table \"%s\"", tabName);
        errFail();
    }
    return tab;
}

void freeDynTable(DynTable *tab) {
    if (tab) {
        free(tab->data);
        free(tab);
    }
    return;
}

void growDynTable(DynTable *tab) {
    unsigned long newSize;
    void *newData;

    if (tab == NULL) {
        errHead(0);
        errPrintf("growDynTable: null table");
        errFail();
    }

    /* Are we already at the limit? */
    if (tab->hWater != 0 && tab->maxIdx == tab->hWater) {
        errHead(0);
        errPrintf(
            "growDynTable: unable to grow table \"%s\" further (reached limit: %lu)",
            tab->tabName, tab->hWater);
        errFail();
    }
    /* Growing currently means doubling. */
    newSize = 2 * tab->maxIdx;

    /* don't grow beyond hWater */
    if (tab->hWater != 0 && tab->hWater < newSize) {
        newSize = tab->hWater;
    } else if (newSize == 0) {
        newSize = 1;
    }
    newData = (void *)realloc(tab->data, newSize * tab->eltSize);

    if (newData == NULL) {
        errHead(0);
        errPrintf("growDynTable: unable to grow table \"%s\" (limit: %lu)",
                  tab->tabName, tab->hWater);
        errFail();
    } else {
        tab->maxIdx = newSize;
        tab->data = newData;
    }
}

Void controlFuns(Int what) {
    Int i;
    struct primInfoDef *info_def = firstPrimInfo;
    for (; 0 != info_def; info_def = info_def->nextPrimInfoDef) {
        if (info_def->p_info->controlFun) {
            (*(info_def->p_info->controlFun))(what);
        }
    }

    if (EXIT == what) {
        struct primInfoDef *ptr, *tmp;

        /* Release all primitive DLLs */
        for (i = 0; i < scriptHw; i++) {
            if (scriptRec(i)->marks.prims) {
                freePrimInfo(scriptRec(i)->marks.prims);
                scriptRec(i)->marks.prims = 0;
            }
        }

        ptr = firstPrimInfo;
        while (ptr) {
            tmp = ptr->nextPrimInfoDef;
            free(ptr);
            ptr = tmp;
        }
    }
}

Void storage(Int what) {
    Int i;

    switch (what) {
    case RESET:
        clearStack();

        /* the next 2 statements are particularly important
                        * if you are using GLOBALfst or GLOBALsnd since the
                        * corresponding registers may be reset to their
                        * uninitialised initial values by a longjump.
                        */
        heapTopFst = heapFst + heapSize;
        heapTopSnd = heapSnd + heapSize;
#if PROFILING
        heapTopThd = heapThd + heapSize;
        if (profile) {
            garbageCollect();
            fclose(profile);
# if HAVE_HP2PS
            {
                Argv cmd;
                argvInit(&cmd);
                argvWord(&cmd, "hp2ps");
                argvWord(&cmd, "profile.hp");
                argvRun(&cmd, TRUE);
                argvFree(&cmd);
            }
# endif
            profile = 0;
        }
#endif
        handles[HSTDIN].hmode = HREAD;
        handles[HSTDIN].hbufMode = HUNKNOWN_BUFFERING;
        handles[HSTDOUT].hmode = HAPPEND;
        handles[HSTDOUT].hbufMode = HUNKNOWN_BUFFERING;
        handles[HSTDERR].hmode = HAPPEND;
        handles[HSTDERR].hbufMode = HUNKNOWN_BUFFERING;
#if CHAR_ENCODING
        handles[HSTDIN].hLookAhead = -1;
        handles[HSTDIN].hBinaryMode = FALSE;
        handles[HSTDOUT].hBinaryMode = FALSE;
        handles[HSTDERR].hBinaryMode = FALSE;
#endif /* CHAR_ENCODING */
        consGC = TRUE;
        lsave = NIL;
        rsave = NIL;
        if (isNull(lastExprSaved))
            savedText = NUM_TEXT;
        break;

    case MARK:
        start();
        for (i = NAMEMIN; i < nameHw; ++i) {
            mark(nameRec(i)->parent);
            mark(nameRec(i)->defn);
            mark(nameRec(i)->type);
            mark(nameRec(i)->clashes);
        }
        end("Names", nameHw - NAMEMIN);
        start();
        for (i = MODMIN; i < moduleHw; ++i) {
            mark(moduleRec(i)->tycons);
            mark(moduleRec(i)->names);
            mark(moduleRec(i)->classes);
            mark(moduleRec(i)->exports);
            mark(moduleRec(i)->modImports);
            mark(moduleRec(i)->modAliases);
            mark(moduleRec(i)->qualImports);
        }
        end("Modules", moduleHw - MODMIN);

        start();
        for (i = TYCMIN; i < tyconHw; ++i) {
            mark(tyconRec(i)->defn);
            mark(tyconRec(i)->kind);
            mark(tyconRec(i)->what);
            mark(tyconRec(i)->clashes);
        }
        end("Type constructors", tyconHw - TYCMIN);

        start();
        for (i = CLASSMIN; i < classHw; ++i) {
            mark(cclassRec(i)->head);
            mark(cclassRec(i)->tyvars);
            mark(cclassRec(i)->kinds);
            mark(cclassRec(i)->fds);
            mark(cclassRec(i)->xfds);
            mark(cclassRec(i)->dsels);
            mark(cclassRec(i)->supers);
            mark(cclassRec(i)->members);
            mark(cclassRec(i)->defaults);
            mark(cclassRec(i)->instances);
            mark(cclassRec(i)->clashes);
        }
        mark(classes);
        end("Classes", classHw - CLASSMIN);

        start();
        for (i = INSTMIN; i < instHw; ++i) {
            mark(instRec(i)->head);
            mark(instRec(i)->kinds);
            mark(instRec(i)->specifics);
            mark(instRec(i)->implements);
        }
        end("Instances", instHw - INSTMIN);

        start();
        for (i = 0; i <= sp; ++i)
            mark(stack(i));
        end("Stack", sp + 1);

        start();
        mark(lastExprSaved);
        mark(lsave);
        mark(rsave);
        end("Last expression", 3);
        start();
        mark(handles[HSTDIN].hcell);
        mark(handles[HSTDOUT].hcell);
        mark(handles[HSTDERR].hcell);
        end("Standard handles", 3);

        start();
        for (i = 0; i < NUM_STABLEPTRS; ++i)
            mark(stablePtrTable[i]);
        end("Stable pointers", NUM_STABLEPTRS);

        mark(finalizers);

        for (i = 0; i < mallocPtr_hw; ++i)
            if (isPair(mallocPtrs[i].mpcell))
                mark(mallocPtrs[i].finalizers);

        if (consGC) {
            start();
            gcCStack();
            end("C stack", stackRoots);
        }

        break;

    case INSTALL:
        heapFst = heapAlloc(heapSize);
        heapSnd = heapAlloc(heapSize);

        if (heapFst == (Heap)0 || heapSnd == (Heap)0) {
            errHead(0);
            errPrintf("Cannot allocate heap storage (%d cells)", heapSize);
            errFail();
        }

        heapTopFst = heapFst + heapSize;
        heapTopSnd = heapSnd + heapSize;
#if PROFILING
        heapThd = heapAlloc(heapSize);
        if (heapThd == (Heap)0) {
            errHead(0);
            errPrintf("Cannot allocate profiler storage space");
            errFail();
        }
        heapTopThd = heapThd + heapSize;
        profile = 0;
        if (0 == profInterval)
            profInterval = heapSize / DEF_PROFINTDIV;
#endif
        for (i = 1; i < heapSize; ++i) {
            fst(-i) = FREECELL;
            snd(-i) = -(i + 1);
        }
        snd(-heapSize) = NIL;
        freeList = -1;
        numGcs = 0;
        consGC = TRUE;
        lsave = NIL;
        rsave = NIL;

        marksSize = bitArraySize(heapSize);
        if ((marks = (unsigned *)calloc(marksSize, sizeof(unsigned))) == 0) {
            errHead(0);
            errPrintf("Unable to allocate gc markspace");
            errFail();
        }

        dynTabHandles =
            allocDynTable(sizeof(struct strHandle), 4, 0, "handles");
        handles = (struct strHandle *)(dynTabHandles->data);
        num_handles = dynTabHandles->maxIdx;
        clearStack();
        for (i = 0; i < (Int)num_handles; i++)
            handles[i].hcell = NIL;

        handles[HSTDIN].hcell = ap(HANDCELL, HSTDIN);
        handles[HSTDIN].hfp = stdin;
        handles[HSTDOUT].hcell = ap(HANDCELL, HSTDOUT);
        handles[HSTDOUT].hfp = stdout;
        handles[HSTDERR].hcell = ap(HANDCELL, HSTDERR);
        handles[HSTDERR].hfp = stderr;

        textHw = 0;
        nextNewText = NUM_TEXT;
        nextNewDText = (-1);
        lastExprSaved = NIL;
        savedText = NUM_TEXT;
        for (i = 0; i < TEXTHSZ; ++i)
            textHash[i][0] = NOTEXT;

        addrHw = 0;

        moduleHw = MODMIN;

        tyconHw = TYCMIN;
        for (i = 0; i < TYCONHSZ; ++i)
            tyconHash[i] = NIL;

        finalizers = NIL;
        liveWeakPtrs = NIL;
        /* The high-water mark for the table points at the
                        * next available slot in the table
                        */
        mallocPtr_hw = 0;
        for (i = 0; i < NUM_MALLOCPTRS; i++)
            mallocPtrs[i].mpcell = NIL;

        resetStablePtrs();

        nameHw = NAMEMIN;
        for (i = 0; i < NAMEHSZ; ++i)
            nameHash[i] = NIL;

        classHw = CLASSMIN;
        instHw = INSTMIN;

        scriptHw = 0;

        initFastWhat();
        break;
    case EXIT:
        /* Let go of dynamic storage */
        if (heapFst)
            free(heapFst);
        if (heapSnd)
            free(heapSnd);
#if PROFILING
        if (heapThd)
            free(heapThd);
#endif
        if (dynTabHandles)
            freeDynTable(dynTabHandles);
        if (marks)
            free(marks);

        /* Note: primInfoDef memory is freed in controlFuns() above */

        break;
    default:
        break;
    }
}

/*-------------------------------------------------------------------------*/
