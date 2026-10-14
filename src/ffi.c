/* --------------------------------------------------------------------------
 * This is the Cupid foreign function interface
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 * ------------------------------------------------------------------------*/

#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "errors.h"
#include "strutil.h"
#include "subst.h"
#include "tycon.h"
#include "typeexp.h"
#include "ffi.h"

/* --------------------------------------------------------------------------
 * Foreign import declarations are Cupid's equivalent of GHC's ccall mechanism.
 * They are used to "import" C functions into a module.
 * Foreign export declarations allow Cupid functions to be called from C.
 * Foreign export declarations provide the address of a C symbol.
 * ------------------------------------------------------------------------*/

/* When using the FFI, you first run Cupid with generateFFI == true
 * to generate C files for any modules which contain foreign import/export
 * declarations.  You then compile and partially link the C files and run
 * Cupid with generateFFI == false to load the object files and lookup
 * the appropriate helper functions in the object files.
 * Only when you run Cupid in the second mode can you actually execute code.
 */
bool generateFFI = false;

bool generate_ffi = false; /* generate FFI for the current module? */

static int checkCallConv(int, Text);

/* Checking if the calling convention is supported */
static int checkCallConv(int line, Text t) {
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
/* Handle foreign imports */
void foreignImport(Cell l, Cell callconv, Cell safety, Cell ext, Cell intName,
                   Cell type) {
    Text t = textOf(intName);
    Name n = findName(t);
    int sfty;
    int cconv;
    int line = intOf(l);

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

/* Handle foreign exports */
void foreignExport(Cell l, Cell v, Cell callconv, Cell ext, Cell intName,
                   Cell type) {
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
     * shared between the generated code and Cupid.  Since these
     * are two separate invocations of Cupid, we cannot use an
     * invented Text so we generate the text by adding a prefix
     * to the start which cannot conflict with normal identifiers.
     */
    int line = intOf(l);
    int cconv;
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

static char *skipSpaces(char *);
static char *matchToken(char *, char *);
static char *skipToSpace(char *);
static char *skipToChar(char *, int);
static char *matchFname(char *);

static char *skipSpaces(char *s) {
    while (isspace(*s)) {
        ++s;
    }
    return s;
}

static char *skipToSpace(char *s) {
    while (*s != '\0' && !isspace(*s)) {
        ++s;
    }
    return s;
}

static char *skipToChar(char *s, int c) {
    while (*s != '\0' && *s != c) {
        ++s;
    }
    return s;
}

static char *matchToken(char *t, char *s) {
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

static char *matchFname(char *s) {
    char *t = skipToSpace(s);
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

/* Check foreign import */
void checkForeignImport(Name p) {
    int line = nameRec(p)->line;
    char *ext = textToStr(nameRec(p)->extFun);
    char *e = 0;
    Type t = NIL;
    List argTys = NIL;

    bool generate_stub = generate_ffi;

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
        bool isIO = false;

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
            isIO = true;
            t = fullerExpand(hd(getArgs(t)));
        }

        if (generate_stub) {
            nameRec(p)->arity = 1 + length(argTys) + (isIO ? IOArity : 0);
            nameRec(p)->extFun = inventText();
            implementForeignImportDynamic(line, nameRec(p)->foreignId,
                                          nameRec(p)->extFun, argTys, isIO, t);
        }

    } else if ((e = matchToken("wrapper", ext))) { /* thunk builder */
        bool isIO = false;

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
            isIO = true;
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
        bool isStatic = false;
        Text fn = -1;
        Text cid = -1;
        bool isLabel = false;

        if ((e = matchToken("static", ext))) {
            isStatic = true;
            ext = skipSpaces(e);
        }
        if ((e = matchFname(ext))) {
            fn = subText(ext, (int)(e - ext));
            ext = skipSpaces(e);
        }

        if ((e = matchToken("&", ext))) {
            isLabel = true;
            ext = skipSpaces(e);
        }

        if ((e = matchToken("[", ext))) {
            ext = skipToChar(e, ']');
            if (*ext != ']' || ext == e)
                goto cantparse;
            errHead(line);
            errPrintf("Cupid doesn't use library specifications.");
            errFail();
        }
        if (*ext != '\0') {
            e = skipToSpace(ext);
            cid = subText(ext, (int)(e - ext));
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
            bool isIO = false;

            if (getHead(t) == typeIO && argCount == 1) {
                isIO = true;
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

/* Check foreign export */
void checkForeignExport(Name p) {
    int line = nameRec(p)->line;
    Text ext = nameRec(p)->extFun;
    Type t;
    List argTys = NIL;
    bool isIO = false;

    emptySubstitution();
    nameRec(p)->type =
        checkSigType(line, "foreign export declaration", p, nameRec(p)->type);
    t = nameRec(p)->type;
    t = fullerExpand(t);

    argTys = peelArgTypes(t, &t); /* the arguments; the result stays in t */

    if (getHead(t) == typeIO && argCount == 1) {
        t = fullerExpand(hd(getArgs(t)));
        isIO = true;
    }

    if (generate_ffi) {
        nameRec(p)->arity = length(argTys) + (isIO ? IOArity : 0);
        implementForeignExport(line, nameRec(p)->foreignId, ext, argTys, isIO,
                               t);
    }
}

/* Link an ffi-generated primitive */
void linkForeign(Name p) {
    addPrim(nameRec(p)->line, p, textToStr(nameRec(p)->text), nameRec(p)->mod,
            nameRec(p)->type);
}

/* --------------------------------------------------------------------------
 * Local function prototypes:
 * ------------------------------------------------------------------------*/

static void foreignType(int, Type);
static void foreignGet(int, Type, char *, int);
static void foreignPut(int, Type, char *, int);
static void ffiInclude(Text);
static void checkCEntity(int, Text);
static void ffiDeclare(int, Type, char *, int);
static void ffiDeclareList(int, List, char *);
static void ffiGetList(int, List, char *);
static void ffiPutList(int, List, char *);
static void ffiCallFun(int, Text, List, List);
static void ffiDeclareFun(int, Text, bool, bool, List, Type);
static void ffiFunTypeCast(int, List, Type);
static void ffiPrimProto(Text, int);
static void ffiPrimHeader(Text, int);
static void ffiReturn(Type, char *, int);

static FILE *out = NIL;     /* file we're generating code into */
static List includes = NIL; /* files already #included         */

/* The statics that hold cells: registered as roots at INSTALL and marked
 * by the collector on every collection */
static Cell *const ffiRoots[] = {
    &includes,
};

/* Close any stub file left open by an aborted run, and clear the list
 * of headers already #included into the current one. */
void resetFfi(void) {
    if (out) {
        fclose(out);
        out = NIL;
    }
    includes = NIL;
}

/* Register the roots; done once, before the Prelude is read. */
void installFfi(void) {
    registerRoots(ffiRoots);
}

static char *ffiFlags = 0; /* extra flags for compilation command line */

void ffiSetFlags(char *s) {
    if (s == 0)
        return;
    if (ffiFlags) {
        int l = (int)strlen(ffiFlags);
        char *t = (char *)realloc(ffiFlags, l + strlen(s) + 2);
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

static char *cppDirectives = 0; /* extra C preprocessor directives */

void ffiAddCppInclude(char *s) {
    int l;
    char *t;
    int needed = (int)strlen(s) + 11;
    if (cppDirectives) {
        l = (int)strlen(cppDirectives);
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
    snprintf(cppDirectives + l, needed, "#include %s\n", s);
}

bool foreignNeedStubs(List imps, List exps) {
    return (nonNull(imps) || nonNull(exps));
}

void foreignHeader(char *fn) {
    char *fnm = mkFFIFilename(fn);
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
    fprintf(out, "static CupidAPI5 *cupid = 0;\n");
}

void foreignFooter(char *fn, Text mn, List is, List es) {
    List xs = NIL;
    fprintf(out, "\n");

    /* Table of all primitives generated by foreign imports */
    fprintf(out, "static struct cupid_primitive cupid_primTable[] = {\n");
    for (xs = is; nonNull(xs); xs = tl(xs)) {
        Name n = hd(xs);
        fprintf(out, "    {\"%s\", ", textToStr(nameRec(n)->text));
        fprintf(out, "%d, ", nameRec(n)->arity);
        fprintf(out, "cupidprim_%s_%d},\n", textToStr(nameRec(n)->extFun),
                nameRec(n)->foreignId);
    }
    for (xs = es; nonNull(xs); xs = tl(xs)) {
        Name n = hd(xs);
        Text ext = nameRec(n)->extFun;
        bool dynamic = inventedText(ext);
        if (dynamic) {
            fprintf(out, "    {\"%s\", 3, ", textToStr(nameRec(n)->text));
            fprintf(out, "cupidprim_%s},\n", textToStr(nameRec(n)->extFun));
        }
    }
    fprintf(out, "};\n");
    fprintf(out, "\n");

    /* The reset hook: rebuilds the stable-pointer table for each export.
     * A module with no exports has nothing to reset, so it gets no hook
     * at all rather than one that runs and does nothing. */
    if (nonNull(es)) {
        fprintf(out, "static void cupid_primReset(void);\n"
                     "static void cupid_primReset(void)\n"
                     "{\n");
        for (xs = es; nonNull(xs); xs = tl(xs)) {
            Name n = hd(xs);
            Text ext = nameRec(n)->extFun;
            bool dynamic = inventedText(ext);
            if (!dynamic) {
                fprintf(out, "    cupid_stable_for_%s = ", textToStr(ext));
                fprintf(out, "cupid->lookupName(");
                fprintf(out, "\"%s\"",
                        textToStr(moduleRec(nameRec(n)->mod)->text));
                fprintf(out, ", \"%s\"", textToStr(nameRec(n)->text));
                fprintf(out, ");\n");
            }
        }
        fprintf(out, "}\n");
    }

    /* The subsystem this module's control lives in: a reset hook when it
     * has one, and nothing else. */
    fprintf(out,
            "static const cupid_subsystem cupid_subsystem_hooks = {\n"
            "    \"%s\", 0, %s, 0, 0, 0\n"
            "};\n",
            textToStr(mn), nonNull(es) ? "cupid_primReset" : "0");

    /* For use as a plugin, rename the initialization function with a name  */
    /* derived from the module name, but abbreviated for limited linkers.   */
    /* example: Foreign.Marshal.Alloc yields initFMAlloc()                  */
    fprintf(out, "\n");
    fprintf(out, "#ifdef STATIC_LINKAGE\n");
    fprintf(out, "#define initModule init");
    {
        char *s = textToStr(mn);
        char *next;
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
        "static struct cupid_primInfo cupid_prims = { &cupid_subsystem_hooks, cupid_primTable, 0 };\n"
        "\n"
        "#ifdef __cplusplus\n"
        "extern \"C\" {\n"
        "#endif\n"
        "#ifndef __cplusplus\n"
        "DLLEXPORT(int)  CupidAPIVersion(void);\n"
        "#endif\n"
        "DLLEXPORT(int)  CupidAPIVersion() {return (%d);}\n"
        "DLLEXPORT(void) initModule(CupidAPI5 *);\n"
        "DLLEXPORT(void) initModule(CupidAPI5 *cupidAPI) {\n"
        "    cupid = cupidAPI;\n"
        "    cupid->registerPrims(&cupid_prims);\n",
        CUPID_API_VERSION);
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
    bool applied;   /* match getHead(t) rather than t           */
    char *hsType;   /* C type it is declared with, 0 for ()     */
    char *marshal;  /* suffix of the get/put pair               */
};

static const struct ffiType ffiTypes[] = {
    {&typeUnit, false, 0, "Unit"},
    {&typeChar, false, "HsChar", "Char"},
    {&typeInt, false, "HsInt", "Int"},
    {&typeInt8, false, "HsInt8", "Int8"},
    {&typeInt16, false, "HsInt16", "Int16"},
    {&typeInt32, false, "HsInt32", "Int32"},
    {&typeInt64, false, "HsInt64", "Int64"},
    {&typeWord8, false, "HsWord8", "Word8"},
    {&typeWord16, false, "HsWord16", "Word16"},
    {&typeWord32, false, "HsWord32", "Word32"},
    {&typeWord64, false, "HsWord64", "Word64"},
    {&typeFloat, false, "HsFloat", "Float"},
    {&typeDouble, false, "HsDouble", "Double"},
    {&typeBool, false, "HsBool", "Bool"},
    {&typeAddr, false, "HsAddr", "Addr"},
    {&typePtr, true, "HsPtr", "Ptr"},
    {&typeFunPtr, true, "HsFunPtr", "FunPtr"},
    {&typeForeign, true, "CupidForeign", "Foreign"},
    {&typeStable, true, "HsStablePtr", "StablePtr4"},
    {0, false, 0, 0}};

/* The row for t, or 0 if the stub generator cannot marshal it.  getHead is
 * called at most once, and only if an application row is reached, so the
 * argCount it leaves behind is the one the chains left. */
static const struct ffiType *ffiTypeOf(Type t) {
    Type h = NIL;
    bool haveHead = false;
    const struct ffiType *r;

    for (r = ffiTypes; r->ty; ++r) {
        if (r->applied) {
            if (!haveHead) {
                h = getHead(t);
                haveHead = true;
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
static void ffiTypeError(int l, char *what, Type t) {
    errHead(l);
    errPrintf("%s", what);
    errPrintf(" \"");
    errType(t);
    errPrintf("\"");
    errFail();
}

static void foreignType(int l, Type t) {
    const struct ffiType *r = ffiTypeOf(t);
    if (r == 0 || r->hsType == 0) { /* () is not a C type */
        ffiTypeError(l, "Illegal foreign type", t);
    }
    fprintf(out, "%s", r->hsType);
}

static void foreignGet(int l, Type t, char *nm, int num) {
    const struct ffiType *r = ffiTypeOf(t);
    if (r == 0) {
        ffiTypeError(l, "Illegal outbound (away from Haskell) type", t);
    }
    if (r->hsType == 0) { /* () is read, but there is nothing to name */
        fprintf(out, "cupid->get%s();\n", r->marshal);
    } else {
        fprintf(out, "%s%d = cupid->get%s();\n", nm, num, r->marshal);
    }
}

static void foreignPut(int l, Type t, char *nm, int num) {
    const struct ffiType *r = ffiTypeOf(t);
    if (r == 0) {
        ffiTypeError(l, "Illegal inbound (coming into Haskell) type", t);
    }
    if (r->hsType == 0) { /* () is written by writing nothing */
        fprintf(out, "\n");
    } else {
        fprintf(out, "cupid->put%s(%s%d);\n", r->marshal, nm, num);
    }
}

/* Add #include */
static void ffiInclude(Text fn) {
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
 * uncompilable C and a C compiler error instead of a Cupid one; catch that
 * here instead.
 */
static void checkCEntity(int line, Text cid) {
    char *s = textToStr(cid);
    char *p = s;
    bool ok =
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

/* Declare variable */
static void ffiDeclare(int line, Type ty, char *prefix, int i) {
    if (ty != typeUnit) {
        fprintf(out, "    ");
        foreignType(line, ty);
        fprintf(out, " %s%d;\n", prefix, i);
    }
}

/* Return variable */
static void ffiReturn(Type ty, char *prefix, int i) {
    if (ty != typeUnit) {
        fprintf(out, "    return %s%d;\n", prefix, i);
    } else {
        fprintf(out, "    return;\n");
    }
}

/* Declare variables */
static void ffiDeclareList(int line, List tys, char *prefix) {
    int i;
    for (i = 1; nonNull(tys); tys = tl(tys), ++i) {
        ffiDeclare(line, hd(tys), prefix, i);
    }
}

/* Get values from Haskell */
static void ffiGetList(int line, List tys, char *prefix) {
    int i;
    for (i = 1; nonNull(tys); tys = tl(tys), ++i) {
        fprintf(out, "    ");
        foreignGet(line, hd(tys), prefix, i);
    }
}

/* Put values to Haskell */
static void ffiPutList(int line, List tys, char *prefix) {
    int i;
    for (i = 1; nonNull(tys); tys = tl(tys), ++i) {
        fprintf(out, "    ");
        foreignPut(line, hd(tys), prefix, i);
    }
}

/* Add a StablePtr argument? */
static void ffiDeclareFun(int line, Text n, bool indirect, bool extraArg,
                          List argTys, List resultTy) {
    int i;
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
        fprintf(out, "CupidStablePtr fun1");
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

static void ffiFunTypeCast(int line, List argTys, List resultTy) {
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

static void ffiCallFun(int line, Text e, List argTys, Type resultTy) {
    int i;
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

/* Generate a Cupid Prim prototype.
 * name should match the C function we're calling because we know
 * that name is a valid C identifier whereas the Haskell name may
 * not be.
 */
static void ffiPrimProto(Text name, int id) {
    fprintf(out, "\nstatic void cupidprim_%s_%d(CupidStackPtr);\n",
            textToStr(name), id);
}

/* Generate a Cupid Prim Header.
 * name should match the C function we're calling because we know
 * that name is a valid C identifier whereas the Haskell name may
 * not be.
 */
static void ffiPrimHeader(Text name, int id) {
    fprintf(out, "static void cupidprim_%s_%d(CupidStackPtr cupid_root)\n",
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
 *     static void cupidprim_extnm(CupidStackPtr);
 *     static void cupidprim_extnm(CupidStackPtr cupid_root)
 *     {
 *         int   arg1 = cupid->getInt();                             
 *         float arg2 = cupid->getFloat();
 *         char  res1 = ext_nm(arg1,arg2);
 *         cupid->putChar(res1);
 *         cupid->returnIO(cupid_root,1);
 *     }
 * 
 */
/* Include file Function name */
void implementForeignImport(int line, Name n, int id, Text fn, Text cid,
                            bool isStatic, Text libName, List argTys, bool isIO,
                            Type resultTy) {

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
        fprintf(out, "    cupid->return%s(cupid_root,%d);\n",
                isIO ? "IO" : "Id", resultTy == typeUnit ? 0 : 1);
    }
    fprintf(out, "}\n");
}

void implementForeignImportDynamic(int line, int id, Text e, List argTys,
                                   bool isIO, Type resultTy) {
    ffiPrimProto(e, id);
    ffiPrimHeader(e, id);
    fprintf(out, "{\n");

    /* Declare arguments and result */
    fprintf(out, "    ");
    ffiDeclareFun(line, e, true, false, argTys, resultTy);
    fprintf(out, ";\n");

    ffiDeclareList(line, argTys, "arg");
    ffiDeclare(line, resultTy, "res", 1);
    fprintf(out, "    %s = ", textToStr(e));
    ffiFunTypeCast(line, argTys, resultTy);
    fprintf(out, "cupid->getFunPtr();\n");
    ffiGetList(line, argTys, "arg");
    ffiCallFun(line, e, argTys, resultTy);
    fprintf(out, "    ");
    foreignPut(line, resultTy, "res", 1);
    if (isIO || nonNull(argTys)) {
        fprintf(out, "    cupid->return%s(cupid_root,%d);\n",
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
 *     static HsChar wrapper(CupidStablePtr arg1, HsInt arg2, HsFloat arg3);
 *     static HsChar wrapper(CupidStablePtr arg1, HsInt arg2, HsFloat arg3);
 *     {
 *         HsChar res1;
 *         cupid->derefStablePtr4(arg1);                             
 *         cupid->putInt(arg2);                             
 *         cupid->putFloat(arg3);
 *         if (cupid->runIO(2)) {
 *             exit(cupid->getInt());
 *         }
 *         res1 = cupid->getChar();
 *         return res1;
 *     }
 * 
 *     static void cupidprim_name(CupidStackPtr cupid_root);
 *     static void cupidprim_name(CupidStackPtr cupid_root)
 *     {
 *         CupidStablePtr arg1 = cupid->makeStablePtr4();    
 *         void* thunk = cupid->mkThunk(&wrapper,arg1);
 *         cupid->putAddr(thunk);
 *         cupid->returnIO(cupid_root,1);
 *     }
 */
void implementForeignImportWrapper(int line, int id, Text e, List argTys,
                                   bool isIO, Type resultTy) {
    /* Prototype for function we're generating */
    fprintf(out, "\nstatic ");
    ffiDeclareFun(line, e, false, true, argTys, resultTy);
    fprintf(out, ";\n");

    /* The function wrapper */
    fprintf(out, "static ");
    ffiDeclareFun(line, e, false, true, argTys, resultTy);
    fprintf(out, "\n{\n");
    ffiDeclare(line, resultTy, "res", 1);

    /* Push function pointer and arguments */
    fprintf(out, "    cupid->derefStablePtr4(fun1);\n");
    ffiPutList(line, argTys, "arg");

    /* Make the call and check for uncaught exception */
    /* ToDo: I'm not sure that exiting from the Cupid session is the right 
     * response to the Haskell function calling System.exit.
     */
    fprintf(out, "    if (cupid->run%s(%d)) {\n", isIO ? "IO" : "Id",
            length(argTys));
    fprintf(out, "        exit(cupid->getInt());\n"
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
            "    CupidStablePtr arg1 = cupid->makeStablePtr4();\n"
            "    void* thunk = cupid->mkThunk((HsFunPtr)%s,arg1);\n",
            textToStr(e));
    fprintf(out, "    cupid->putAddr(thunk);\n"
                 "    cupid->returnIO(cupid_root,1);\n"
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
 *     static CupidStablePtr cupid_stable_for_extnm = -1;
 *     char extnm(int arg1, float arg2);
 *     char extnm(int arg1, float arg2)
 *     {
 *         char  res1;
 *         cupid->putInt(cupid_stable_for_extnm);
 *         cupid->putInt(arg1);                             
 *         cupid->putFloat(arg2);                             
 *         if (cupid->runIO(2)) {
 *             exit(cupid->getInt());
 *         }
 *         res1 = cupid->getChar();
 *         return res1;
 *     }
 *
 */
void implementForeignExport(int line, int id, Text e, List argTys, bool isIO,
                            Type resultTy) {

    /* Prototype for function we're generating */
    fprintf(out, "\nextern ");
    ffiDeclareFun(line, e, false, false, argTys, resultTy);
    fprintf(out, ";\n");

    fprintf(out, "static CupidStablePtr cupid_stable_for_%s = -1;\n",
            textToStr(e));

    /* The function wrapper */
    ffiDeclareFun(line, e, false, false, argTys, resultTy);
    fprintf(out, "\n{\n");
    ffiDeclare(line, resultTy, "res", 1);

    /* Push function pointer and arguments */
    fprintf(out, "    cupid->putInt(cupid_stable_for_%s);\n", textToStr(e));
    ffiPutList(line, argTys, "arg");

    /* Make the call and check for uncaught exception */
    if (isIO) {
        /* ToDo: I'm not sure that exiting from the Cupid session is the right 
         * response to the Haskell function calling System.exit.
         */
        fprintf(out, "    if (cupid->runIO(%d)) {\n", length(argTys));
        fprintf(out, "        exit(cupid->getInt());\n"
                     "    }\n");
    } else {
        fprintf(out, "    cupid->ap(%d);\n", length(argTys));
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
 *     static void cupidprim_name(CupidStackPtr);
 *     static void cupidprim_name(CupidStackPtr cupid_root)
 *     {
 *         extern int cid; // probably the wrong type but it doesn't matter 
 *         cupid->putAddr(&cid);
 *         cupid_returnId(1);
 *     }
 */
/* Include file Function name Haskell name */
void implementForeignImportLabel(int line, int id, Text fn, Text cid, Text n,
                                 Type ty) {
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
        fprintf(out, "    cupid->putFunPtr((HsFunPtr)&%s);\n", textToStr(cid));
    else
        fprintf(out, "    cupid->putPtr(&%s);\n", textToStr(cid));
    fprintf(out, "}\n");
}

/* ToDo: 
 * chain all foreign exports together and free at end of run?
 * copy GreenCard.h into Test.c? 
 */

/*-------------------------------------------------------------------------*/
