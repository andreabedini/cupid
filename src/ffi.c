/* --------------------------------------------------------------------------
 * This is the Cuddles foreign function interface
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: ffi.c,v $
 * $Revision: 1.40 $
 * $Date: 2005/06/21 11:17:48 $
 * ------------------------------------------------------------------------*/

#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "errors.h"
#include "output.h"
#include "strutil.h"
#include "machdep.h"
#include "subst.h"
#include "module.h"
#include "goal.h"
#include "tycon.h"
#include "typeexp.h"
#include "ffi.h"

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

Bool generate_ffi = FALSE; /* generate FFI for the current module? */

static Int checkCallConv(Int, Text);

/* Checking if the calling convention is supported */
static Int checkCallConv(Int line, Text t) {
    if (t == textCCall)
        return FFI_CCONV_CCALL;
#if STDCALL_SUPPORTED
    /* also support the stdcall calling convention */
    if (t == textStdcall)
        return FFI_CCONV_STDCALL;
#endif
    return FFI_CCONV_UNKNOWN;
}

/* Tricky naming detail:
 *
 * When we generate C function names, we need to make sure they
 * are unique within each module.  This is done using the foreignCount
 * variable.
 */
Void foreignImport(Cell l, Cell callconv, Cell safety, Cell ext, Cell intName,
                   Cell type)
/* Handle foreign imports    */
{
    Text t = textOf(intName);
    Name n = findName(t);
    Int sfty;
    Int cconv;
    Int line = intOf(l);

    if (isNull(n)) {
        n = newName(t, NIL);
    } else if (nameRec(n)->defn != PREDEFINED) {
        errHead(line);
        errPrintf("Redeclaration of foreign \"%s\"", textToStr(t));
        errFail();
    }
    cconv = checkCallConv(line, textOf(callconv));
    if (cconv == FFI_CCONV_UNKNOWN) {
        errHead(line);
        errPrintf("Foreign import calling convention \"%s\" not supported",
                  textToStr(textOf(callconv)));
        errFail();
    }

    if (isNull(safety) || textOf(safety) == textSafe) {
        sfty = FFI_SAFE;
    } else if (textOf(safety) == textUnsafe) {
        sfty = FFI_UNSAFE;
    } else if (textOf(safety) == textThreadsafe) {
        sfty = FFI_THREADSAFE;
    } else {
        errHead(line);
        errPrintf("Foreign import safety level \"%s\" not supported",
                  textToStr(textOf(safety)));
        errFail();
    }
    nameRec(n)->line = line;
    nameRec(n)->type = type;
    nameRec(n)->extFun = textOf(ext);
    nameRec(n)->foreignId = foreignCount++;
    nameRec(n)->foreignFlags = sfty | cconv;
    foreignImports = cons(n, foreignImports);
}

Void foreignExport(Cell l, Cell v, Cell callconv, Cell ext, Cell intName,
                   Cell type)
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
    Int line = intOf(l);
    Int cconv;
    Text t = concatText("--FFI_", textToStr(textOf(ext)));
    Name n = newName(t, NIL);

    if (textOf(v) != textExport) {
        errHead(line);
        errPrintf(
            "Foreign declarations must be either import or export not \"%s\"",
            textToStr(textOf(v)));
        errFail();
    }
    cconv = checkCallConv(line, textOf(callconv));
    if (cconv == FFI_CCONV_UNKNOWN) {
        errHead(line);
        errPrintf("Foreign export calling convention \"%s\" not supported",
                  textToStr(textOf(callconv)));
        errFail();
    }

    nameRec(n)->line = line;
    nameRec(n)->type = type;
    nameRec(n)->extFun = textOf(ext);
    nameRec(n)->defn = intName;
    nameRec(n)->foreignId = foreignCount++;
    nameRec(n)->foreignFlags = FFI_NOSAFETY | cconv;
    foreignExports = cons(n, foreignExports);
}

static String skipSpaces(String);
static String matchToken(String, String);
static String skipToSpace(String);
static String skipToChar(String, int);
static String matchFname(String);

static String skipSpaces(String s) {
    while (isspace(*s)) {
        ++s;
    }
    return s;
}

static String skipToSpace(String s) {
    while (*s != '\0' && !isspace(*s)) {
        ++s;
    }
    return s;
}

static String skipToChar(String s, int c) {
    while (*s != '\0' && *s != c) {
        ++s;
    }
    return s;
}

static String matchToken(String t, String s) {
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

static String matchFname(String s) {
    String t = skipToSpace(s);
    if (t - s >= 3 && t[-2] == '.' && t[-1] == 'h') {
        return t;
    } else {
        return 0;
    }
}

/* Peel a function type into its argument types and its result type: for
 * `a -> b -> r' the list [a, b], with r left in *resultTy.  Every type that
 * comes back has been through fullerExpand, so the caller sees no synonyms
 * and no newtypes; a type that is not a function gives the empty list and
 * itself as the result.  The three foreign declaration checks below all
 * start this way.
 *
 * hasIOResultType in tycon.c has the same loop head but expands only the
 * result, so it cannot share this: expanding the argument types too can
 * raise an error on a polymorphic newtype, which it does not raise today. */
static List peelArgTypes(Type t, Type *resultTy) {
    List argTys = NIL;
    while (getHead(t) == typeArrow && argCount == 2) {
        Type ta = fullerExpand(arg(fun(t)));
        Type tr = fullerExpand(arg(t));
        argTys = cons(ta, argTys);
        t = tr;
    }
    *resultTy = t;
    return rev(argTys);
}

Void checkForeignImport(Name p) /* Check foreign import          */
{
    Int line = nameRec(p)->line;
    String ext = textToStr(nameRec(p)->extFun);
    String e = 0;
    Type t = NIL;
    List argTys = NIL;

    Bool generate_stub = generate_ffi;

    emptySubstitution();
    nameRec(p)->type = checkSigType(
        nameRec(p)->line, "foreign import declaration", p, nameRec(p)->type);

    t = nameRec(p)->type;
    if (isPolyType(t)) {
        t = monotypeOf(t);
    }
    t = fullerExpand(t);
    argTys = peelArgTypes(t, &t); /* the arguments; the result stays in t */

    /* What kind of import is this? */
    ext = skipSpaces(ext);
    if ((e = matchToken("dynamic", ext))) { /* dynamic import */
        Type ta = NIL;
        Bool isIO = FALSE;

        e = skipSpaces(e);
        if (*e != '\0')
            goto cantparse;

        /* type must be of the form:
         *
         *    (FunPtr fty) -> fty
         *
         */

        if (length(argTys) < 1)
            goto dynerr;
        ta = hd(argTys);
        argTys = tl(argTys);

        if (getHead(ta) != typeFunPtr || argCount != 1)
            goto dynerr;
        ta = hd(getArgs(ta));

        /* ToDo: check that ta == argTys -> t and check it's a valid type */

        if (getHead(t) == typeIO && argCount == 1) {
            isIO = TRUE;
            t = fullerExpand(hd(getArgs(t)));
        }

        if (generate_stub) {
            nameRec(p)->arity = 1 + length(argTys) + (isIO ? IOArity : 0);
            nameRec(p)->extFun = inventText();
            implementForeignImportDynamic(line, nameRec(p)->foreignId,
                                          nameRec(p)->extFun, argTys, isIO, t);
        }

    } else if ((e = matchToken("wrapper", ext))) { /* thunk builder */
        Bool isIO = FALSE;

        e = skipSpaces(e);
        if (*e != '\0')
            goto cantparse;

        if (length(argTys) != 1)
            goto wraperr;

        if (getHead(t) != typeIO || argCount != 1)
            goto wraperr;
        t = fullerExpand(hd(getArgs(t)));

        if (getHead(t) != typeFunPtr)
            goto wraperr;
        t = fullerExpand(hd(getArgs(t)));

        /* ToDo: check that hd(argTys) is the type t we just unwrapped */

        t = fullerExpand(t);
        argTys = peelArgTypes(t, &t); /* arguments; the result stays in t */

        if (getHead(t) == typeIO && argCount == 1) {
            isIO = TRUE;
            t = fullerExpand(hd(getArgs(t)));
        }
        t = fullerExpand(t);

        if (generate_stub) {
            nameRec(p)->arity = 1 + IOArity;
            nameRec(p)->extFun = inventText();
            implementForeignImportWrapper(line, nameRec(p)->foreignId,
                                          nameRec(p)->extFun, argTys, isIO, t);
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
        Text fn = -1;
        Text cid = -1;
        Bool isLabel = FALSE;

        if ((e = matchToken("static", ext))) {
            isStatic = TRUE;
            ext = skipSpaces(e);
        }
        if ((e = matchFname(ext))) {
            fn = subText(ext, (Int)(e - ext));
            ext = skipSpaces(e);
        }

        if ((e = matchToken("&", ext))) {
            isLabel = TRUE;
            ext = skipSpaces(e);
        }

        if ((e = matchToken("[", ext))) {
            ext = skipToChar(e, ']');
            if (*ext != ']' || ext == e)
                goto cantparse;
            errHead(line);
            errPrintf("Cuddles doesn't use library specifications.");
            errFail();
        }
        if (*ext != '\0') {
            e = skipToSpace(ext);
            cid = subText(ext, (Int)(e - ext));
            ext = skipSpaces(e);
        } else {
            cid = nameRec(p)->text;
        }

        if (*ext != '\0')
            goto cantparse;

        if (isLabel) {
            if (!isNull(argTys))
                goto labelerr;
            if (!((getHead(t) == typePtr && argCount == 1) ||
                  (getHead(t) == typeFunPtr && argCount == 1))) {
                goto labelerr;
            }
            if (generate_stub) {
                nameRec(p)->arity = 0;
                implementForeignImportLabel(line, nameRec(p)->foreignId, fn,
                                            cid, nameRec(p)->text, t);
                nameRec(p)->extFun = cid;
            }
        } else {
            Bool isIO = FALSE;

            if (getHead(t) == typeIO && argCount == 1) {
                isIO = TRUE;
                t = fullerExpand(hd(getArgs(t)));
            }

            if (generate_stub) {
                nameRec(p)->arity = length(argTys) + (isIO ? IOArity : 0);
                implementForeignImport(line, p, nameRec(p)->foreignId, fn, cid,
                                       isStatic, -1, argTys, isIO, t);
                nameRec(p)->extFun = cid;
            }
        }
    }

    return;

cantparse:
    errHead(line);
    errPrintf("Can't parse external entity '");
    errPrintf("%s", ext);
    errPrintf("'\n");
    errFail();

dynerr:
    errHead(line);
    errPrintf("foreign import dynamic must have type '(FunPtr ft) -> ft'");
    errFail();

wraperr:
    errHead(line);
    errPrintf("foreign import wrapper must have type 'ft -> IO (FunPtr ft)'");
    errFail();

labelerr:
    errHead(line);
    errPrintf("foreign import & must have type 'Ptr a' or 'FunPtr a'");
    errFail();
}

Void checkForeignExport(Name p) /* Check foreign export      */
{
    Int line = nameRec(p)->line;
    Text ext = nameRec(p)->extFun;
    Type t;
    List argTys = NIL;
    Bool isIO = FALSE;

    emptySubstitution();
    nameRec(p)->type =
        checkSigType(line, "foreign export declaration", p, nameRec(p)->type);
    t = nameRec(p)->type;
    t = fullerExpand(t);

    argTys = peelArgTypes(t, &t); /* the arguments; the result stays in t */

    if (getHead(t) == typeIO && argCount == 1) {
        t = fullerExpand(hd(getArgs(t)));
        isIO = TRUE;
    }

    if (generate_ffi) {
        nameRec(p)->arity = length(argTys) + (isIO ? IOArity : 0);
        implementForeignExport(line, nameRec(p)->foreignId, ext, argTys, isIO,
                               t);
    }
}

Void linkForeign(Name p) /* Link an ffi-generated primitive */
{
    addPrim(nameRec(p)->line, p, textToStr(nameRec(p)->text), nameRec(p)->mod,
            nameRec(p)->type);
}

/* --------------------------------------------------------------------------
 * Local function prototypes:
 * ------------------------------------------------------------------------*/

static Void local foreignType(Int, Type);
static Void local foreignGet(Int, Type, String, Int);
static Void local foreignPut(Int, Type, String, Int);
static Void local ffiInclude(Text);
static Void local checkCEntity(Int, Text);
static Void local ffiDeclare(Int, Type, String, Int);
static Void local ffiDeclareList(Int, List, String);
static Void local ffiGetList(Int, List, String);
static Void local ffiPutList(Int, List, String);
static Void local ffiCallFun(Int, Text, List, List);
static Void local ffiDeclareFun(Int, Text, Bool, Bool, List, Type);
static Void local ffiFunTypeCast(Int, List, Type);
static Void local ffiPrimProto(Text, Int);
static Void local ffiPrimHeader(Text, Int);
static Void local ffiReturn(Type, String, Int);

static FILE *out = NIL;     /* file we're generating code into */
static List includes = NIL; /* files already #included         */

/* The statics that hold cells: registered as roots at INSTALL and marked
 * by the collector on every collection */
static Cell *const ffiRoots[] = {
    &includes,
};

Void ffi(Int what) {
    switch (what) {
    case RESET:
        if (out) {
            fclose(out);
            out = NIL;
        }
        includes = NIL;
        break;
    case INSTALL:
        registerRoots(ffiRoots);
        break;
    default:
        break;
    }
}

static String ffiFlags = 0; /* extra flags for compilation command line */

Void ffiSetFlags(String s) {
    if (s == 0)
        return;
    if (ffiFlags) {
        Int l = (Int)strlen(ffiFlags);
        String t = (char *)realloc(ffiFlags, l + strlen(s) + 2);
        if (t == 0) { /* keep old ffiFlags on failure     */
            errHead(0);
            errPrintf("String storage space exhausted");
            errFail();
        }
        ffiFlags = t;
        ffiFlags[l] = ' ';
        strcpy(ffiFlags + l + 1, s);
    } else {
        ffiFlags = strCopy(s);
    }
}

static String cppDirectives = 0; /* extra C preprocessor directives */

Void ffiAddCppInclude(String s) {
    Int l;
    String t;
    Int needed = (Int)strlen(s) + 11;
    if (cppDirectives) {
        l = (Int)strlen(cppDirectives);
        t = (char *)realloc(cppDirectives, l + needed);
    } else {
        l = 0;
        t = (char *)malloc(needed);
    }
    if (t == 0) { /* keep old cppDirectives on error  */
        errHead(0);
        errPrintf("String storage space exhausted");
        errFail();
    }
    cppDirectives = t;
    sprintf(cppDirectives + l, "#include %s\n", s);
}

Bool foreignNeedStubs(List imps, List exps) {
    return (nonNull(imps) || nonNull(exps));
}

Void foreignHeader(String fn) {
    String fnm = mkFFIFilename(fn);
    FILE *f = fopen(fnm, "w");
    if (f == NULL) {
        errHead(0);
        errPrintf("Unable to create file '%s'", fnm);
        errFail();
    }
    out = f;
    fprintf(out, "/* Machine generated file, do not modify */\n");
    fprintf(out, "#include <stdlib.h>\n");
    fprintf(out, "#include \"HsFFI.h\"\n");
    if (cppDirectives)
        fprintf(out, "%s", cppDirectives);
    fprintf(out, "\n");
    fprintf(out, "static CuddlesAPI5 *cuddles = 0;\n");
}

Void foreignFooter(String fn, Text mn, List is, List es) {
    List xs = NIL;
    fprintf(out, "\n");

    /* Table of all primitives generated by foreign imports */
    fprintf(out, "static struct cuddles_primitive cuddles_primTable[] = {\n");
    for (xs = is; nonNull(xs); xs = tl(xs)) {
        Name n = hd(xs);
        fprintf(out, "    {\"%s\", ", textToStr(nameRec(n)->text));
        fprintf(out, "%d, ", nameRec(n)->arity);
        fprintf(out, "cuddlesprim_%s_%d},\n", textToStr(nameRec(n)->extFun),
                nameRec(n)->foreignId);
    }
    for (xs = es; nonNull(xs); xs = tl(xs)) {
        Name n = hd(xs);
        Text ext = nameRec(n)->extFun;
        Bool dynamic = inventedText(ext);
        if (dynamic) {
            fprintf(out, "    {\"%s\", 3, ", textToStr(nameRec(n)->text));
            fprintf(out, "cuddlesprim_%s},\n", textToStr(nameRec(n)->extFun));
        }
    }
    fprintf(out, "};\n");
    fprintf(out, "\n");

    /* The control function: rebuilds stable ptr table on RESET */
    fprintf(out, "static void cuddles_primControl(int);\n"
                 "static void cuddles_primControl(int what)\n"
                 "{\n");

    if (nonNull(es)) {
        fprintf(out,
                "    switch (what) {\n"
                "        case %d:\n",
                RESET);
    }

    for (xs = es; nonNull(xs); xs = tl(xs)) {
        Name n = hd(xs);
        Text ext = nameRec(n)->extFun;
        Bool dynamic = inventedText(ext);
        if (!dynamic) {
            fprintf(out, "        cuddles_stable_for_%s = ", textToStr(ext));
            fprintf(out, "cuddles->lookupName(");
            fprintf(out, "\"%s\"", textToStr(moduleRec(nameRec(n)->mod)->text));
            fprintf(out, ", \"%s\"", textToStr(nameRec(n)->text));
            fprintf(out, ");\n");
        }
    }
    if (nonNull(es)) {
        fprintf(out, "    }\n");
    }
    fprintf(out, "}\n");

    /* For use as a plugin, rename the initialization function with a name  */
    /* derived from the module name, but abbreviated for limited linkers.   */
    /* example: Foreign.Marshal.Alloc yields initFMAlloc()                  */
    fprintf(out, "\n");
    fprintf(out, "#ifdef STATIC_LINKAGE\n");
    fprintf(out, "#define initModule init");
    {
        String s = textToStr(mn);
        String next;
        while ((next = strchr(s, '.')) != NULL) {
            fprintf(out, "%c", s[0]);
            s = next + 1;
        }
        fprintf(out, "%s\n", s);
    }
    fprintf(out, "#endif\n");
    fprintf(out, "\n");

    /* Boilerplate initialization function */
    fprintf(
        out,
        "static struct cuddles_primInfo cuddles_prims = { cuddles_primControl, cuddles_primTable, 0 };\n"
        "\n"
        "#ifdef __cplusplus\n"
        "extern \"C\" {\n"
        "#endif\n"
        "#ifndef __cplusplus\n"
        "DLLEXPORT(int)  CuddlesAPIVersion(void);\n"
        "#endif\n"
        "DLLEXPORT(int)  CuddlesAPIVersion() {return (%d);}\n"
        "DLLEXPORT(void) initModule(CuddlesAPI5 *);\n"
        "DLLEXPORT(void) initModule(CuddlesAPI5 *cuddlesAPI) {\n"
        "    cuddles = cuddlesAPI;\n"
        "    cuddles->registerPrims(&cuddles_prims);\n",
        CUDDLES_API_VERSION);
    fprintf(out, "}\n"
                 "#ifdef __cplusplus\n"
                 "}\n"
                 "#endif\n"
                 "\n");

    fclose(out);
    out = NIL;

    compileAndLink(fn, ffiFlags);
    if (ffiFlags) {
        free(ffiFlags);
        ffiFlags = 0;
    }
}

/* --------------------------------------------------------------------------
 * The types the stub generator can marshal.
 *
 * One row per type, replacing the three parallel if-chains foreignType,
 * foreignGet and foreignPut used to be.  `ty' points at the global the type
 * checker fills in when it links the Prelude, so the table holds the address
 * of a type and not the type itself.  `applied' says the row matches the
 * head of an application, as in `Ptr a', rather than the type itself; the
 * plain rows come first, as they did in the chains.  `hsType' is the C type
 * a value is declared with, and `marshal' names the pair get<marshal> and
 * put<marshal> that carries it across.  The unit type is the odd one out: it
 * has no C type, so foreignType rejects it, it is read with getUnit(), and
 * writing it writes nothing.
 * ------------------------------------------------------------------------*/

struct ffiType {
    const Type *ty; /* the type, or the head of the application */
    Bool applied;   /* match getHead(t) rather than t           */
    String hsType;  /* C type it is declared with, 0 for ()     */
    String marshal; /* suffix of the get/put pair               */
};

static const struct ffiType ffiTypes[] = {
    {&typeUnit, FALSE, 0, "Unit"},
    {&typeChar, FALSE, "HsChar", "Char"},
    {&typeInt, FALSE, "HsInt", "Int"},
    {&typeInt8, FALSE, "HsInt8", "Int8"},
    {&typeInt16, FALSE, "HsInt16", "Int16"},
    {&typeInt32, FALSE, "HsInt32", "Int32"},
    {&typeInt64, FALSE, "HsInt64", "Int64"},
    {&typeWord8, FALSE, "HsWord8", "Word8"},
    {&typeWord16, FALSE, "HsWord16", "Word16"},
    {&typeWord32, FALSE, "HsWord32", "Word32"},
    {&typeWord64, FALSE, "HsWord64", "Word64"},
    {&typeFloat, FALSE, "HsFloat", "Float"},
    {&typeDouble, FALSE, "HsDouble", "Double"},
    {&typeBool, FALSE, "HsBool", "Bool"},
    {&typeAddr, FALSE, "HsAddr", "Addr"},
    {&typePtr, TRUE, "HsPtr", "Ptr"},
    {&typeFunPtr, TRUE, "HsFunPtr", "FunPtr"},
    {&typeForeign, TRUE, "CuddlesForeign", "Foreign"},
    {&typeStable, TRUE, "HsStablePtr", "StablePtr4"},
    {0, FALSE, 0, 0}};

/* The row for t, or 0 if the stub generator cannot marshal it.  getHead is
 * called at most once, and only if an application row is reached, so the
 * argCount it leaves behind is the one the chains left. */
static const struct ffiType *local ffiTypeOf(Type t) {
    Type h = NIL;
    Bool haveHead = FALSE;
    const struct ffiType *r;

    for (r = ffiTypes; r->ty; ++r) {
        if (r->applied) {
            if (!haveHead) {
                h = getHead(t);
                haveHead = TRUE;
            }
            if (h == *r->ty) {
                return r;
            }
        } else if (t == *r->ty) {
            return r;
        }
    }
    return 0;
}

/* The tail the three chains shared, under their three different headings. */
static Void local ffiTypeError(Int l, String what, Type t) {
    errHead(l);
    errPrintf("%s", what);
    errPrintf(" \"");
    errType(t);
    errPrintf("\"");
    errFail();
}

static Void local foreignType(Int l, Type t) {
    const struct ffiType *r = ffiTypeOf(t);
    if (r == 0 || r->hsType == 0) { /* () is not a C type */
        ffiTypeError(l, "Illegal foreign type", t);
    }
    fprintf(out, "%s", r->hsType);
}

static Void local foreignGet(Int l, Type t, String nm, Int num) {
    const struct ffiType *r = ffiTypeOf(t);
    if (r == 0) {
        ffiTypeError(l, "Illegal outbound (away from Haskell) type", t);
    }
    if (r->hsType == 0) { /* () is read, but there is nothing to name */
        fprintf(out, "cuddles->get%s();\n", r->marshal);
    } else {
        fprintf(out, "%s%d = cuddles->get%s();\n", nm, num, r->marshal);
    }
}

static Void local foreignPut(Int l, Type t, String nm, Int num) {
    const struct ffiType *r = ffiTypeOf(t);
    if (r == 0) {
        ffiTypeError(l, "Illegal inbound (coming into Haskell) type", t);
    }
    if (r->hsType == 0) { /* () is written by writing nothing */
        fprintf(out, "\n");
    } else {
        fprintf(out, "cuddles->put%s(%s%d);\n", r->marshal, nm, num);
    }
}

static Void local ffiInclude(Text fn) /* Add #include  */
{
    if (fn != -1 && !varIsMember(fn, includes)) {
        fprintf(out, "#include \"%s\"\n", textToStr(fn));
        includes = cons(mkVar(fn), includes);
    }
}

/* checkForeignImport above only checks that the entity string of a
 * "static"/label foreign import parses into a [fname] [&] [cid]; cid, the
 * bit we treat as a C identifier below (and in implementForeignImport and
 * implementForeignImportLabel), is whatever text is left over - it could
 * be an operator like "+", or default to the Haskell name, which need not
 * be a valid C identifier either.  Printed raw into generated C (as a
 * function/variable name, and after #undef), anything else produces
 * uncompilable C and a C compiler error instead of a Cuddles one; catch that
 * here instead.
 */
static Void local checkCEntity(Int line, Text cid) {
    String s = textToStr(cid);
    String p = s;
    Bool ok =
        (*p == '_' || (*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z'));
    for (++p; ok && *p; p++)
        ok = (*p == '_' || (*p >= 'A' && *p <= 'Z') ||
              (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9'));
    if (!ok) {
        errHead(line);
        errPrintf("Foreign import entity \"%s\" is not a valid C identifier",
                  s);
        errFail();
    }
}

static Void local ffiDeclare(Int line, Type ty, String prefix,
                             Int i) /* Declare variable  */
{
    if (ty != typeUnit) {
        fprintf(out, "    ");
        foreignType(line, ty);
        fprintf(out, " %s%d;\n", prefix, i);
    }
}

static Void local ffiReturn(Type ty, String prefix,
                            Int i) /* Return variable  */
{
    if (ty != typeUnit) {
        fprintf(out, "    return %s%d;\n", prefix, i);
    } else {
        fprintf(out, "    return;\n");
    }
}

static Void local ffiDeclareList(Int line, List tys,
                                 String prefix) /* Declare variables */
{
    Int i;
    for (i = 1; nonNull(tys); tys = tl(tys), ++i) {
        ffiDeclare(line, hd(tys), prefix, i);
    }
}

static Void local ffiGetList(Int line, List tys,
                             String prefix) /* Get values from Haskell */
{
    Int i;
    for (i = 1; nonNull(tys); tys = tl(tys), ++i) {
        fprintf(out, "    ");
        foreignGet(line, hd(tys), prefix, i);
    }
}

static Void local ffiPutList(Int line, List tys,
                             String prefix) /* Put values to Haskell */
{
    Int i;
    for (i = 1; nonNull(tys); tys = tl(tys), ++i) {
        fprintf(out, "    ");
        foreignPut(line, hd(tys), prefix, i);
    }
}

static Void local ffiDeclareFun(Int line, Text n, Bool indirect, Bool extraArg,
                                List argTys, List resultTy)
/* Add a StablePtr argument? */
{
    Int i;
    if (resultTy == typeUnit) {
        fprintf(out, "void");
    } else {
        foreignType(line, resultTy);
    }
    if (indirect) {
        fprintf(out, " (*%s)", textToStr(n));
    } else {
        fprintf(out, " %s", textToStr(n));
    }
    fprintf(out, "(");
    if (extraArg) {
        fprintf(out, "CuddlesStablePtr fun1");
        if (nonNull(argTys)) {
            fprintf(out, ", ");
        }
    }
    for (i = 1; nonNull(argTys); argTys = tl(argTys), ++i) {
        foreignType(line, hd(argTys));
        fprintf(out, " arg%d", i);
        if (nonNull(tl(argTys))) {
            fprintf(out, ", ");
        }
    }
    fprintf(out, ")");
}

static Void local ffiFunTypeCast(Int line, List argTys, List resultTy) {
    fprintf(out, "(");
    if (resultTy == typeUnit) {
        fprintf(out, "void");
    } else {
        foreignType(line, resultTy);
    }
    fprintf(out, " (*)(");
    for (; nonNull(argTys); argTys = tl(argTys)) {
        foreignType(line, hd(argTys));
        if (nonNull(tl(argTys))) {
            fprintf(out, ", ");
        }
    }
    fprintf(out, "))");
}

static Void local ffiCallFun(Int line, Text e, List argTys, Type resultTy) {
    Int i;
    fprintf(out, "    ");
    if (resultTy != typeUnit) {
        fprintf(out, "res1 = ");
    }
    fprintf(out, "%s(", textToStr(e));
    for (i = 1; nonNull(argTys); argTys = tl(argTys), ++i) {
        fprintf(out, "arg%d", i);
        if (nonNull(tl(argTys))) {
            fprintf(out, ", ");
        }
    }
    fprintf(out, ");\n");
}

/* Generate a Cuddles Prim prototype.
 * name should match the C function we're calling because we know
 * that name is a valid C identifier whereas the Haskell name may
 * not be.
 */
static Void local ffiPrimProto(Text name, Int id) {
    fprintf(out, "\nstatic void cuddlesprim_%s_%d(CuddlesStackPtr);\n",
            textToStr(name), id);
}

/* Generate a Cuddles Prim Header.
 * name should match the C function we're calling because we know
 * that name is a valid C identifier whereas the Haskell name may
 * not be.
 */
static Void local ffiPrimHeader(Text name, Int id) {
    fprintf(out,
            "static void cuddlesprim_%s_%d(CuddlesStackPtr cuddles_root)\n",
            textToStr(name), id);
}

/* Generate C code for calling C functions from Haskell.
 * The code has to be compiled with a C compiler and dynamically
 * loaded.
 *
 * For example:
 * 
 *     foreign import "static fn ext_nm" name :: Int -> Float -> IO Char
 * ==>
 *
 *     #ifndef ENABLE_MACRO_INTERFACE
 *     #undef ext_nm
 *     #endif
 *
 *     static void cuddlesprim_extnm(CuddlesStackPtr);
 *     static void cuddlesprim_extnm(CuddlesStackPtr cuddles_root)
 *     {
 *         int   arg1 = cuddles->getInt();                             
 *         float arg2 = cuddles->getFloat();
 *         char  res1 = ext_nm(arg1,arg2);
 *         cuddles->putChar(res1);
 *         cuddles->returnIO(cuddles_root,1);
 *     }
 * 
 */
Void implementForeignImport(Int line, Name n, Int id, Text fn, Text cid,
                            Bool isStatic, Text libName, List argTys, Bool isIO,
                            Type resultTy)
/* Include file */
/* Function name */
{

    checkCEntity(line, cid);
    ffiInclude(fn);

    /* Prevent the cid from matching a C macro */
    fprintf(out, "\n#ifndef ENABLE_MACRO_INTERFACE\n");
    fprintf(out, "#undef %s\n", textToStr(cid));
    fprintf(out, "#endif\n");

    ffiPrimProto(cid, id);
    ffiPrimHeader(cid, id);
    fprintf(out, "{\n");

    ffiDeclareList(line, argTys, "arg");
    ffiDeclare(line, resultTy, "res", 1);
    ffiGetList(line, argTys, "arg");
    ffiCallFun(line, cid, argTys, resultTy);
    fprintf(out, "    ");
    foreignPut(line, resultTy, "res", 1);
    if (isIO || nonNull(argTys)) {
        fprintf(out, "    cuddles->return%s(cuddles_root,%d);\n",
                isIO ? "IO" : "Id", resultTy == typeUnit ? 0 : 1);
    }
    fprintf(out, "}\n");
}

Void implementForeignImportDynamic(Int line, Int id, Text e, List argTys,
                                   Bool isIO, Type resultTy) {
    ffiPrimProto(e, id);
    ffiPrimHeader(e, id);
    fprintf(out, "{\n");

    /* Declare arguments and result */
    fprintf(out, "    ");
    ffiDeclareFun(line, e, TRUE, FALSE, argTys, resultTy);
    fprintf(out, ";\n");

    ffiDeclareList(line, argTys, "arg");
    ffiDeclare(line, resultTy, "res", 1);
    fprintf(out, "    %s = ", textToStr(e));
    ffiFunTypeCast(line, argTys, resultTy);
    fprintf(out, "cuddles->getFunPtr();\n");
    ffiGetList(line, argTys, "arg");
    ffiCallFun(line, e, argTys, resultTy);
    fprintf(out, "    ");
    foreignPut(line, resultTy, "res", 1);
    if (isIO || nonNull(argTys)) {
        fprintf(out, "    cuddles->return%s(cuddles_root,%d);\n",
                isIO ? "IO" : "Id", resultTy == typeUnit ? 0 : 1);
    }
    fprintf(out, "}\n");
}

/*
 * For wrappers, we generate:
 *
 * For example:
 * 
 *     foreign import "wrapper" name ::            (Int -> Float -> Char) 
 *                                   -> IO (FunPtr (Int -> Float -> Char))
 * ==>
 *     
 *     static HsChar wrapper(CuddlesStablePtr arg1, HsInt arg2, HsFloat arg3);
 *     static HsChar wrapper(CuddlesStablePtr arg1, HsInt arg2, HsFloat arg3);
 *     {
 *         HsChar res1;
 *         cuddles->derefStablePtr4(arg1);                             
 *         cuddles->putInt(arg2);                             
 *         cuddles->putFloat(arg3);
 *         if (cuddles->runIO(2)) {
 *             exit(cuddles->getInt());
 *         }
 *         res1 = cuddles->getChar();
 *         return res1;
 *     }
 * 
 *     static void cuddlesprim_name(CuddlesStackPtr cuddles_root);
 *     static void cuddlesprim_name(CuddlesStackPtr cuddles_root)
 *     {
 *         CuddlesStablePtr arg1 = cuddles->makeStablePtr4();    
 *         void* thunk = cuddles->mkThunk(&wrapper,arg1);
 *         cuddles->putAddr(thunk);
 *         cuddles->returnIO(cuddles_root,1);
 *     }
 */
Void implementForeignImportWrapper(Int line, Int id, Text e, List argTys,
                                   Bool isIO, Type resultTy) {
    /* Prototype for function we're generating */
    fprintf(out, "\nstatic ");
    ffiDeclareFun(line, e, FALSE, TRUE, argTys, resultTy);
    fprintf(out, ";\n");

    /* The function wrapper */
    fprintf(out, "static ");
    ffiDeclareFun(line, e, FALSE, TRUE, argTys, resultTy);
    fprintf(out, "\n{\n");
    ffiDeclare(line, resultTy, "res", 1);

    /* Push function pointer and arguments */
    fprintf(out, "    cuddles->derefStablePtr4(fun1);\n");
    ffiPutList(line, argTys, "arg");

    /* Make the call and check for uncaught exception */
    /* ToDo: I'm not sure that exiting from the Cuddles session is the right 
     * response to the Haskell function calling System.exit.
     */
    fprintf(out, "    if (cuddles->run%s(%d)) {\n", isIO ? "IO" : "Id",
            length(argTys));
    fprintf(out, "        exit(cuddles->getInt());\n"
                 "    }\n");
    fprintf(out, "    ");
    foreignGet(line, resultTy, "res", 1);
    ffiReturn(resultTy, "res", 1);

    /* Return result */
    fprintf(out, "}\n");

    ffiPrimProto(e, id);
    ffiPrimHeader(e, id);
    fprintf(out,
            "{\n"
            "    CuddlesStablePtr arg1 = cuddles->makeStablePtr4();\n"
            "    void* thunk = cuddles->mkThunk((HsFunPtr)%s,arg1);\n",
            textToStr(e));
    fprintf(out, "    cuddles->putAddr(thunk);\n"
                 "    cuddles->returnIO(cuddles_root,1);\n"
                 "}\n");
}

/* 
 * Generate C code for calling C functions from Haskell.
 * The code has to be compiled with a C compiler and dynamically
 * loaded.
 *
 * For example:
 * 
 *     foreign export "extnm" name :: Int -> Float -> IO Char
 * ==>
 *     
 *     static CuddlesStablePtr cuddles_stable_for_extnm = -1;
 *     char extnm(int arg1, float arg2);
 *     char extnm(int arg1, float arg2)
 *     {
 *         char  res1;
 *         cuddles->putInt(cuddles_stable_for_extnm);
 *         cuddles->putInt(arg1);                             
 *         cuddles->putFloat(arg2);                             
 *         if (cuddles->runIO(2)) {
 *             exit(cuddles->getInt());
 *         }
 *         res1 = cuddles->getChar();
 *         return res1;
 *     }
 *
 */
Void implementForeignExport(Int line, Int id, Text e, List argTys, Bool isIO,
                            Type resultTy) {

    /* Prototype for function we're generating */
    fprintf(out, "\nextern ");
    ffiDeclareFun(line, e, FALSE, FALSE, argTys, resultTy);
    fprintf(out, ";\n");

    fprintf(out, "static CuddlesStablePtr cuddles_stable_for_%s = -1;\n",
            textToStr(e));

    /* The function wrapper */
    ffiDeclareFun(line, e, FALSE, FALSE, argTys, resultTy);
    fprintf(out, "\n{\n");
    ffiDeclare(line, resultTy, "res", 1);

    /* Push function pointer and arguments */
    fprintf(out, "    cuddles->putInt(cuddles_stable_for_%s);\n", textToStr(e));
    ffiPutList(line, argTys, "arg");

    /* Make the call and check for uncaught exception */
    if (isIO) {
        /* ToDo: I'm not sure that exiting from the Cuddles session is the right 
         * response to the Haskell function calling System.exit.
         */
        fprintf(out, "    if (cuddles->runIO(%d)) {\n", length(argTys));
        fprintf(out, "        exit(cuddles->getInt());\n"
                     "    }\n");
    } else {
        fprintf(out, "    cuddles->ap(%d);\n", length(argTys));
    }
    fprintf(out, "    ");
    foreignGet(line, resultTy, "res", 1);
    ffiReturn(resultTy, "res", 1);
    fprintf(out, "}\n");
}

/* 
 * Generate primitive for address of a C symbol.
 *
 * For example:
 * 
 *     foreign import "static & cid" name :: Addr
 * ==>
 *
 *     #ifndef ENABLE_MACRO_INTERFACE
 *     #undef cid
 *     #endif
 *
 *     static void cuddlesprim_name(CuddlesStackPtr);
 *     static void cuddlesprim_name(CuddlesStackPtr cuddles_root)
 *     {
 *         extern int cid; // probably the wrong type but it doesn't matter 
 *         cuddles->putAddr(&cid);
 *         cuddles_returnId(1);
 *     }
 */
Void implementForeignImportLabel(Int line, Int id, Text fn, Text cid, Text n,
                                 Type ty)
/* Include file */
/* Function name */
/* Haskell name */
{
    checkCEntity(line, cid);
    ffiInclude(fn);

    /* Prevent the cid from matching a C macro */
    fprintf(out, "\n#ifndef ENABLE_MACRO_INTERFACE\n");
    fprintf(out, "#undef %s\n", textToStr(cid));
    fprintf(out, "#endif\n");

    ffiPrimProto(cid, id);
    ffiPrimHeader(cid, id);
    fprintf(out, "{\n");
    if (getHead(ty) == typeFunPtr)
        fprintf(out, "    cuddles->putFunPtr((HsFunPtr)&%s);\n",
                textToStr(cid));
    else
        fprintf(out, "    cuddles->putPtr(&%s);\n", textToStr(cid));
    fprintf(out, "}\n");
}

/* ToDo: 
 * chain all foreign exports together and free at end of run?
 * copy GreenCard.h into Test.c? 
 */

/*-------------------------------------------------------------------------*/
