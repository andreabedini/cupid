/* --------------------------------------------------------------------------
 * Haskell 98 module system implementation for Cupid.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * -sof 2002.
 * 
 * ------------------------------------------------------------------------*/
#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "errors.h"
#include "module.h"
#include "output.h"

/* --------------------------------------------------------------------------
 * Static analysis of modules:
 *
 * The static checks of the import and export lists are invoked
 * via the entry points in module.h  (cf. static.c:checkDefns()).
 *
 * ------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
 * local function prototypes:
 * ------------------------------------------------------------------------*/
static Name lookupName(Text, List);
static Module modOfEntity(Cell);
static void reportAmbigEntity(Text, Text, List);
static List checkSubentities(List, List, List, char *, Text);
static void checkExportDistinct(List, bool, Cell);
static List checkExportTycon(List, Text, bool, Cell, Tycon);
static List checkExportClass(List, Text, bool, Cell, Class);
static List checkExportModule(List, Text, Cell);
static List checkExport(List, Text, Cell);
static List checkImportEntity(List, Module, bool, Cell);
static List resolveImportList(Module, Cell, bool);
static Cell entityIsMember(Cell, List);
static List augmentEntity(bool, Cell, Cell, List);
static List addEntity(Cell, Cell, List);
static List addEntityPair(Cell, List);
static List mergeImportLists(List, List);
static List getIEOrphans(List);
static List fixupIEList(List);
static List allMethodsOrDCons(List, Cell, Module, bool);

static Cell importEntity(Module, Cell);
static void importTycon(Module, Tycon);
static void importClass(Module, Class);

static void browseName(Name);
static void browseEntity(Cell);

/* Add to qualified import list Original name of module Name module is called
 * within this module (or NIL) List of entity names */
void addQualImport(Cell orig, Cell new, List entities) {
    /* Record the entities imported */
    moduleRec(currentModule)->qualImports =
        addEntity(orig, entities, moduleRec(currentModule)->qualImports);
    /* Record the module --> alias mapping */
    moduleRec(currentModule)->modAliases =
        cons(pair(isNull(new) ? orig : new, orig),
             moduleRec(currentModule)->modAliases);
}

/* An unqualified import Name of module Local alias List of entity names */
void addUnqualImport(Cell mod, Cell new, List entities) {
    /* Add to unqualified import list */
    unqualImports = addEntity(mod, entities, unqualImports);
    /* Record the module --> alias mapping */
    moduleRec(currentModule)->modAliases =
        cons(pair(isNull(new) ? mod : new, mod),
             moduleRec(currentModule)->modAliases);
}

/* Find text t in list of Names :: [Name] */
static Name lookupName(Text t, List nms) {
    for (; nonNull(nms); nms = tl(nms)) {
        if (t == nameRec(hd(nms))->text) {
            return hd(nms);
        }
    }
    return NIL;
}

/* :: [ Q?(Var|Con)(Id|Op) ] :: [Name] "<constructor>|<member> of
 * <type>|<class>" */
static List checkSubentities(List imports, List named, List wanted,
                             char *description, Text textParent) {
    for (; nonNull(named); named = tl(named)) {
        Pair x = hd(named);
        /* ToDo: ignores qualifier; doesn't check that entity is in scope */
        Text t = isPair(snd(x)) ? qtextOf(x) : textOf(x);
        Name n = lookupName(t, wanted);
        if (isNull(n)) {
            errHead(0);
            errPrintf("Entity \"%s\" is not a %s \"%s\"", textToStr(t),
                      description, textToStr(textParent));
            errFail();
        }
        imports = cons(n, imports);
    }
    return imports;
}

/* Locate methods/dcons appearing on their own in an import/export list. */
static List getIEOrphans(List ieList) {
    List orphans = NIL;
    List xs;

    for (xs = ieList; nonNull(xs); xs = tl(xs)) {
        Cell e = hd(xs);
        if (isName(e)) {
            if (isClass(nameRec(e)->parent)) {
                /* a lone member */
                orphans = cons(pair(nameRec(e)->parent, singleton(e)), orphans);
            } else if (isCfun(e)) {
                /* a lone data constructor (can only appear in a hiding list.) */
                orphans = cons(pair(nameRec(e)->parent, singleton(e)), orphans);
            } else if (isSfun(e)) {
                /* a field name */
                Cell p = nameRec(e)->parent; /* the data constructor */
                Cell t = nameRec(p)->parent; /* the type constructor */
                orphans = cons(pair(t, singleton(e)), orphans);
            }
        }
    }
    return orphans;
}

/*
 * fixupIEList() traverses an import/export list, adjusting
 * the list in the following ways:
 *
 *  1. 'orphan'/subordinate names are joined up with their
 *     parents. An orphan E is either a class member, field
 *     name or a data constructor (in 'hiding' lists *only*)
 *     that's imported/exported without referring to its
 *     parent P -- E appears in an import/export list rather
 *     than P(E). If P is appearing elsewhere in the
 *     import/export list, float E inside of P.
 *
 * ToDo: remove duplicates from the resulting list also; it's
 *       harmless for there to be any, but may lead to confusion
 *       later on.
 * 
 */
static List fixupIEList(List ieList) {
    List orphans = NIL;
    List xs;

    orphans = getIEOrphans(ieList);

    if (nonNull(orphans)) {
        /* Transformation 1 (we're actually being a bit sloppy here
       and not removing the orphan from the IE list if it
       can be floated inside its parent.) */
        for (xs = orphans; nonNull(xs); xs = tl(xs)) {
            ieList = augmentEntity(false, fst(hd(xs)), snd(hd(xs)), ieList);
        }
    }
    return ieList;
}

void fixupImportExports(List ls) {
    List xs;
    for (xs = ls; nonNull(xs); xs = tl(xs)) {
        if (isPair(hd(xs)) && !fst(snd(hd(xs)))) {
            snd(snd(hd(xs))) = fixupIEList(snd(snd(hd(xs))));
        }
    }
}

/* Accumulated list of things to import Entry from import/hiding list */
static List checkImportEntity(List imports, Module exporter, bool isHidden,
                              Cell entity) {
    bool impFound = false;
    bool isId = isIdent(entity);
    Cell subEntities = !isId ? snd(entity) : NIL;
    Text t = isId ? textOf(entity) : textOf(fst(entity));
    List es = moduleRec(exporter)->exports;
    bool lookForVar = isVar(entity);

    /* In H98, a data con may be named in a 'hiding' list, so we
     * have to grovel around inside each tycon looking for it.
     */
    bool lookForDataCon =
        isHidden && subEntities == NONE && isCon(isId ? entity : fst(entity));

    /* The use of NONE heralds a dcon */
    subEntities = ((subEntities == NONE) ? NIL : subEntities);

    for (; nonNull(es); es = tl(es)) {
        Cell e = hd(es); /* :: Entity | (Entity, NIL|DOTDOT|[Entity]) */
        if (isPair(e)) {
            Cell f = fst(e);
            if (isTycon(f)) {
                if (tyconRec(f)->text == t) {
                    impFound = true;
                    if (!isId) {
                        switch (tyconRec(f)->what) {
                        case NEWTYPE:
                        case DATATYPE:
                            if (DOTDOT == subEntities) {
                                /* Want all dcons that are _exported_ by
                                   the importing module.
                                */
                                Cell dcons;
                                if (snd(e) == DOTDOT) {
                                    dcons = tyconRec(f)->defn;
                                } else {
                                    dcons = snd(e);
                                }
                                imports = addEntity(f, dcons, imports);
                            } else if (NIL == subEntities) {
                                imports = addEntity(f, NIL, imports);
                            } else {
                                List xs = NIL;
                                xs = checkSubentities(xs, subEntities,
                                                      tyconRec(f)->defn,
                                                      "constructor of type", t);
                                imports = addEntity(f, xs, imports);
                            }
                            break;
                        case SYNONYM:
                        case RESTRICTSYN:
                        case POLYSYN:
                            imports = addEntity(f, DOTDOT, imports);
                            break;
                        default:;
                            /* deliberate fall thru */
                        }
                    } else {
                        imports = addEntity(f, NIL, imports);
                    }
                    if (!lookForDataCon)
                        break;
                }
                /* check the data constructors or field labels for match */
                if (tyconRec(f)->what != SYNONYM &&
                    tyconRec(f)->what != RESTRICTSYN &&
                    tyconRec(f)->what != POLYSYN &&
                    (lookForVar || lookForDataCon)) {
                    /* The type's exported dcons/fields */
                    Cell dcons;
                    if (snd(e) == DOTDOT) {
                        dcons = tyconRec(f)->defn;
                    } else {
                        dcons = snd(e);
                    }
                    while (nonNull(dcons)) {
                        if (isName(hd(dcons)) &&
                            nameRec(hd(dcons))->text == t) {
                            impFound = true;
                            imports = addEntity(hd(dcons), NIL, imports);
                            break;
                        }
                        dcons = tl(dcons);
                    }
                }
            } else if (isClass(f)) {
                List sigs = NIL;
                /* Want all members that are _exported_ by the importing module. */
                if (isPair(e)) {
                    if (snd(e) == DOTDOT) {
                        sigs = cclassRec(f)->members;
                    } else {
                        sigs = snd(e);
                    }
                }
                if (cclassRec(f)->text == t) {
                    impFound = true;
                    if (!isId) {
                        if (DOTDOT == subEntities) {
                            imports = addEntity(f, sigs, imports);
                            return imports;
                        } else if (NIL == subEntities) {
                            imports = addEntity(f, NIL, imports);
                        } else {
                            List xs = NIL;
                            xs = checkSubentities(xs, subEntities,
                                                  cclassRec(f)->members,
                                                  "member of class", t);
                            imports = addEntity(f, xs, imports);
                        }
                    }
                    break;
                }
                if (!impFound && isId) {
                    List xs = sigs;
                    while (nonNull(xs)) {
                        if (isName(hd(xs)) && nameRec(hd(xs))->text == t) {
                            impFound = true;
                            imports = cons(hd(xs), imports);
                            break;
                        }
                        xs = tl(xs);
                    }
                }
            } else {
                internal("checkImportEntity2");
            }
        } else if (isName(e)) {
            if (isId && nameRec(e)->text == t) {
                impFound = true;
                imports = cons(e, imports);
                if (!lookForDataCon)
                    break;
            }
        } else if (isTycon(e)) {
            if (isId && tyconRec(e)->text == t) {
                impFound = true;
                imports = addEntity(e, NIL, imports);
                if (!lookForDataCon)
                    break;
            }
        } else {
            internal("checkImportEntity3");
        }
    }
    /* Hiding a name the module does not export is not an error, as in GHC:
     * import Prelude hiding (catch) is how code written for a Prelude that
     * exported catch avoids the clash with Control.Exception's. */
    if (!impFound && !isHidden) {
        errHead(0);
        errPrintf("Unknown entity \"%s\" imported from module \"%s\"",
                  textToStr(t), textToStr(moduleRec(exporter)->text));
        errFail();
    }
    return imports;
}

/* Exporting module */
static List resolveImportList(Module m, Cell impList, bool isHidden) {
    List imports = NIL;
    if (DOTDOT == impList) {
        List es = moduleRec(m)->exports;
        for (; nonNull(es); es = tl(es)) {
            Cell e = hd(es);
            if (isName(e)) {
                imports = cons(e, imports);
            } else {
                Cell c = fst(e);
                List subentities = NIL;
                if (isClass(c) ||
                    (isTycon(c) && (tyconRec(c)->what == DATATYPE ||
                                    tyconRec(c)->what == NEWTYPE))) {
                    if (snd(e) != DOTDOT) {
                        List ys = snd(e);
                        Name sub;
                        while (nonNull(ys)) {
                            if (isPair(hd(ys))) {
                                if (nonNull(sub = findQualName(hd(ys)))) {
                                    subentities = cons(sub, subentities);
                                }
                            } else {
                                subentities = cons(hd(ys), subentities);
                            }
                            ys = tl(ys);
                        }
                        subentities = rev(subentities);
                    } else {
                        if (isClass(c)) {
                            subentities = cclassRec(c)->members;
                        } else {
                            subentities = tyconRec(c)->defn;
                        }
                    }
                }
                imports = addEntity(c, subentities, imports);
            }
        }
        imports = rev(imports);
    } else {
        map2Accum(checkImportEntity, imports, m, isHidden, impList);
    }
    return imports;
}

void checkQualImportList(Pair importSpec) {
    /* checkQualImport() has verified that the module has already been loaded;
   * just locate the Module for it here & update the qualImports.
   */
    Module m = findModid(fst(importSpec));
    fst(importSpec) = m;

    checkImportList(true, importSpec);
}

/* For pair (e,ls) add ls to the 'is' import/export list. */
static List addEntityPair(Cell e, List is) {
    if (isPair(e)) {
        return addEntity(fst(e), snd(e), is);
    } else if (isName(e)) {
        if (isClass(nameRec(e)->parent)) {
            return addEntity(nameRec(e)->parent, singleton(e), is);
        } else if (isTycon(nameRec(e)->parent) && isCfun(e)) {
            return addEntity(nameRec(e)->parent, singleton(e), is);
        } else if (isSfun(e)) {
            /* a field name */
            Cell p = nameRec(e)->parent; /* the data constructor */
            Cell t = nameRec(p)->parent; /* the type constructor */
            return addEntity(t, singleton(e), is);
        }
    }
    return addEntity(e, NIL, is);
}

/* For entity e, add 'ls' to the 'is' import/export list,combining it with
 * previous entries (if any.) */
static List augmentEntity(bool addNew, Cell e, Cell ls, List is) {

    Cell ms = entityIsMember(e, is);

    if (!ms) {
        if (!addNew) {
            return is;
        } else {
            if (isName(e) && (ls == NIL || ls == NONE)) {
                return cons(e, is);
            } else {
                return cons(pair(e, ls), is);
            }
        }
    } else {
        /* concat the two lists; remove duplicates. */
        if (!isPair(hd(ms)) && ls != NIL) {
            hd(ms) = pair(e, ls);
        } else if (ls == NIL || ls == NONE || snd(hd(ms)) == DOTDOT) {
            ;
        } else if (ls == DOTDOT || snd(hd(ms)) == NIL || snd(hd(ms)) == NONE) {
            snd(hd(ms)) = ls;
        } else {
            snd(hd(ms)) = nubList(dupOnto(ls, snd(hd(ms))));
        }
        return is;
    }
}

static List addEntity(Cell e, Cell ls, List is) {
    return augmentEntity(true, e, ls, is);
}

/* Test for membership of specific entity x in import/export list xs */
static Cell entityIsMember(Cell x, List xs) {
    for (; nonNull(xs); xs = tl(xs)) {
        if (x == hd(xs))
            return xs;
        if (isPair(hd(xs)) && x == fst(hd(xs)))
            return xs;
    }
    return NIL;
}

static List mergeImportLists(List ls1, List ls2) {
    List xs;

    for (xs = ls1; nonNull(xs); xs = tl(xs)) {
        ls2 = addEntityPair(hd(xs), ls2);
    }
    return ls2;
}

/* Import a module (un)qualified */
void checkImportList(bool isQual, Pair importSpec) {
    Module m = fst(importSpec);
    Cell impList = snd(importSpec);

    List imports = NIL; /* entities we want to import */
    List hidden = NIL;  /* entities we want to hide   */

    List modImps = NIL; /* The effective import list */
    List es = NIL;
    bool isHidden = (isPair(impList) && HIDDEN == fst(impList));

    if (!isQual && moduleThisScript(m)) {
        errHead(0);
        errPrintf("Module \"%s\" recursively imports itself",
                  textToStr(moduleRec(m)->text));
        errFail();
    }
    if (isHidden) {
        List orphans;
        /* Somewhat inefficient - but obviously correct:
         * imports = importsOf("module Foo") `setDifference` hidden;
         */
        hidden = fixupIEList(resolveImportList(m, snd(impList), true));
        imports = resolveImportList(m, DOTDOT, false);

        /* Get the lone method/field/dcons that appear in the hiding list. */
        orphans = getIEOrphans(hidden);

        /* remove them from their parents in the import list. */
        for (; nonNull(orphans); orphans = tl(orphans)) {
            /* the 'hd.snd' is the orphan entity, 'fst' is its parent. */

            /* Locate and remove the sub-entity. */
            Cell ls = entityIsMember(fst(hd(orphans)), imports);
            if (ls && isPair(hd(ls))) {
                snd(hd(ls)) =
                    removeCell(hd(snd(hd(orphans))), dupList(snd(hd(ls))));
            }
        }

        /* With the orphans in the 'hiding' list accounted for,
         * compute the effective import list by traversing over the
         * entire import list, checking whether any of the entities
         * do appear in the hiding list.
         */
        for (; nonNull(imports); imports = tl(imports)) {
            Cell e = hd(imports);

            if (isPair(e)) {
                /* A tycon/class */
                Cell tc = fst(e);
                Cell subs = snd(e);
                List ms = entityIsMember(tc, hidden);

                if (!ms) {
                    /* not in the hiding list, add it to effective import list. */
                    if (isQual) {
                        modImps = cons(pair(tc, subs), modImps);
                    } else {
                        modImps = cons(importEntity(m, e), modImps);
                    }
                } else if (isPair(hd(ms))) {
                    /* The parent tycon/class is hidden, but perhaps
                   not all of its subentities. */
                    Cell ent = fst(hd(ms));
                    List hiddenSubs = snd(hd(ms));
                    Module impMod;

                    /* Figure out what module the entity was imported from */
                    if (isClass(ent)) {
                        impMod = cclassRec(ent)->mod;
                    } else if (isTycon(ent)) {
                        impMod = tyconRec(ent)->mod;
                    } else {
                        internal("checkImportList");
                    }

                    for (; nonNull(subs); subs = tl(subs)) {
                        if (!entityIsMember(hd(subs), hiddenSubs)) {
                            /* Register the sub-entity as imported */
                            if (!isQual)
                                importName(impMod, hd(subs));
                            modImps = cons(hd(subs), modImps);
                        }
                    }
                }
            } else {
                if (!entityIsMember(e, hidden)) {
                    if (isQual) {
                        modImps = cons(pair(e, NIL), modImps);
                    } else {
                        modImps = cons(importEntity(m, e), modImps);
                    }
                }
            }
        }

    } else { /* the more common case, no hidings. */
        imports = resolveImportList(m, impList, false);
        if (isQual) {
            modImps = imports;
        } else {
            for (; nonNull(imports); imports = tl(imports)) {
                modImps = addEntityPair(importEntity(m, hd(imports)), modImps);
            }
        }
    }
    /* To be able to handle re-exportation of modules, each module
     * keeps track of the effective import list of all its imports,
     * so that we later on can constrain re-exportation to only
     * contain what was imported.
     */

    /* If there's more than one import decl for the same module,
     * combine the import lists.
     */
    for (es = moduleRec(currentModule)->modImports; nonNull(es); es = tl(es)) {
        if (isPair(hd(es)) && fst(hd(es)) == m) {
            fst(snd(hd(es))) = false; /* => perform fixup at the end. */
            snd(snd(hd(es))) = mergeImportLists(modImps, snd(snd(hd(es))));
            break;
        }
    }
    if (isNull(es)) {
        /* Module not already present, add it. */
        moduleRec(currentModule)->modImports =
            cons(pair(m, pair((Cell)isHidden, modImps)),
                 moduleRec(currentModule)->modImports);
    }
}

static Cell importEntity(Module source, Cell e) {
    Cell ent = e;
    Cell cs = NIL;

    /* If a pair, then the snd component gives the
     * constructors/methods that are specifically imported
     * with the tycon/class.
     */
    if (isPair(e)) {
        ent = fst(e);
        cs = snd(e);
    }
    if (cs != NIL && cs != DOTDOT) {
        List xs;
        for (xs = cs; nonNull(xs); xs = tl(xs)) {
            importEntity(source, hd(xs));
        }
    }
    switch (whatIs(ent)) {
    case VARIDCELL:
    case VAROPCELL:
    case CONIDCELL:
    case CONOPCELL:
        importName(source, snd(ent));
        return e;
    case NAME:
        importName(source, ent);
        return e;
    case TYCON:
        importTycon(source, ent);
        return pair(ent, cs);
    case CLASS:
        importClass(source, ent);
        return pair(ent, cs);

    default:
        internal("importEntity");
        return NIL;
    }
}

void importName(Module source, Name n) {
    Name clash = addName(n);
    if (nonNull(clash) &&
        clash != n
        /* 'n' contains a name imported from another module's
         * export list. Due to module re-exportation, its 'home
         * module' (i.e., the module where 'n' was actually declared)
         * may not be equal to that of the module we're now importing
         * from here ('source'.) So, we've only got a name clash if
         * the home module of 'n' is different from that of 'clash'.
         */
        && nameRec(n)->mod != nameRec(clash)->mod) {
        nameRec(clash)->clashes = cons(n, nameRec(clash)->clashes);
    }
}

/* Types and classes share one namespace: a type and a class of the same
 * name, imported from different modules, make an unqualified use of either
 * ambiguous.  Each goes on the other's list of clashes, so the lists hold
 * tycons and classes alike. */
static void addKindClash(Tycon tc, Class c) {
    if (tyconRec(tc)->mod == cclassRec(c)->mod)
        return;
    if (!cellIsMember(c, tyconRec(tc)->clashes))
        tyconRec(tc)->clashes = cons(c, tyconRec(tc)->clashes);
    if (!cellIsMember(tc, cclassRec(c)->clashes))
        cclassRec(c)->clashes = cons(tc, cclassRec(c)->clashes);
}

static void importTycon(Module source, Tycon tc) {
    Tycon clash = addTycon(tc);
    Class cc;
    if (nonNull(clash) &&
        clash != tc
        /* See importName() comment. */
        && tyconRec(tc)->mod != tyconRec(clash)->mod) {
        tyconRec(clash)->clashes = cons(tc, tyconRec(clash)->clashes);
    }
    if (nonNull(cc = findClass(tyconRec(tc)->text))) {
        addKindClash(clash, cc);
    }
}

static void importClass(Module source, Class c) {
    Class clash = addClass(c);
    Tycon tc;
    if (nonNull(clash) &&
        clash != c
        /* See importName() comment. */
        && cclassRec(c)->mod != cclassRec(clash)->mod) {
        /* Hmm..don't quite understand why we need to record the clash
           on both the class values here..*/
        cclassRec(c)->clashes = cons(clash, cclassRec(c)->clashes);
        cclassRec(clash)->clashes = cons(c, cclassRec(clash)->clashes);
    }
    if (nonNull(tc = findTycon(cclassRec(c)->text))) {
        addKindClash(tc, clash);
    }
}

/* Get at the module of name/tycon/class */
static Module modOfEntity(Cell ent) {
    if (isName(ent)) {
        return nameRec(ent)->mod;
    } else if (isTycon(ent)) {
        return tyconRec(ent)->mod;
    } else if (isClass(ent)) {
        return cclassRec(ent)->mod;
    }
    return NIL;
}

static void reportAmbigEntity(Text mt, Text t, List clashes) {
    if (nonNull(clashes)) {
        /* Unqualified name is ambiguous, report this. */
        Module m1;
        errHead(0);
        errPrintf("Ambiguous export of entity \"%s\"", textToStr(t));
        errPrintf("\n*** Could refer to: %s.%s ", textToStr(mt), textToStr(t));
        for (; nonNull(clashes); clashes = tl(clashes)) {
            m1 = modOfEntity(hd(clashes));

            if (m1) {
                errPrintf("%s.%s ", textToStr(moduleRec(m1)->text),
                          textToStr(t));
            }
        }
        errPrintf("\n");
        errFail();
    }
}

/* verify that the entity is unique in unqualified form */
static void checkExportDistinct(List exports, bool ambigCheck, Cell ent) {
    Name clashNm;
    Tycon clashTc;
    Class clashCc;
    Module mod1, mod2;
    Text txt;
    bool inConflict = false;
    List clashes = NIL;

    if (isName(ent)) {
        clashes = nameRec(ent)->clashes;
        txt = nameRec(ent)->text;
        mod1 = nameRec(ent)->mod;

        if ((clashNm = nameInIEList(ent, exports)) &&
            (nameRec(clashNm)->mod != nameRec(ent)->mod)) {
            mod2 = nameRec(clashNm)->mod;
            inConflict = true;
        }
    } else if (isTycon(ent)) {
        clashes = tyconRec(ent)->clashes;
        txt = tyconRec(ent)->text;
        mod1 = tyconRec(ent)->mod;

        if ((clashTc = tyconInIEList(tyconRec(ent)->text, exports)) &&
            (tyconRec(clashTc)->mod != tyconRec(ent)->mod)) {
            mod2 = tyconRec(clashTc)->mod;
            inConflict = true;
        }
    } else if (isClass(ent)) {
        clashes = cclassRec(ent)->clashes;
        txt = cclassRec(ent)->text;
        mod1 = cclassRec(ent)->mod;

        if ((clashCc = classInIEList(cclassRec(ent)->text, exports)) &&
            (cclassRec(clashCc)->mod != cclassRec(ent)->mod)) {
            mod2 = cclassRec(clashCc)->mod;
            inConflict = true;
        }
    } else if (isPair(ent)) {
        List subs = NIL;
        checkExportDistinct(exports, ambigCheck, fst(ent));
        if (snd(ent) == DOTDOT) {
            if (isTycon(fst(ent))) {
                if (tyconRec(fst(ent))->what == SYNONYM ||
                    tyconRec(fst(ent))->what == RESTRICTSYN ||
                    tyconRec(fst(ent))->what == POLYSYN) {
                    subs = NIL;
                } else {
                    subs = tyconRec(fst(ent))->defn;
                }
            } else if (isClass(fst(ent))) {
                subs = cclassRec(fst(ent))->members;
            }
        } else {
            subs = snd(ent);
        }
        map2Proc(checkExportDistinct, exports, ambigCheck, subs);
        return;
    } else {
        return;
    }

    if (inConflict) {
        errHead(0);
        errPrintf("Conflicting exports of entity \"%s\"", textToStr(txt));
        errPrintf("\n*** Could refer to %s.%s or %s.%s",
                  textToStr(moduleRec(mod1)->text), textToStr(txt),
                  textToStr(moduleRec(mod2)->text), textToStr(txt));
        errFail();
    }

    if (ambigCheck && nonNull(clashes)) {
        reportAmbigEntity(moduleRec(mod1)->text, txt, clashes);
    }
}

static List checkExportTycon(List exports, Text mt, bool viaModExport,
                             Cell spec, Tycon tc) {
    checkExportDistinct(exports, !viaModExport, pair(tc, spec));
    if (DOTDOT == spec || SYNONYM == tyconRec(tc)->what ||
        POLYSYN == tyconRec(tc)->what) {
        return addEntity(tc, DOTDOT, exports);
    } else {
        return addEntity(tc, NIL, exports);
    }
}

static List checkExportClass(List exports, Text mt, bool viaModExport,
                             Cell spec, Class cl) {
    checkExportDistinct(exports, !viaModExport, pair(cl, spec));
    if (DOTDOT == spec) {
        return addEntity(cl, DOTDOT, exports);
    } else {
        return addEntity(cl, NIL, exports);
    }
}

static List checkExportModule(List exports, Text mt, Cell e) {
    /* The name refers to the module alias; get at the modules it refers to */
    Text alias = textOf(snd(e));
    List mods = findQualifiers(alias);
    Module m;

    /* Re-exporting a module we didn't import isn't allowed. */
    if (isNull(mods)) {
        errHead(0);
        errPrintf("Unknown module \"%s\" exported from module \"%s\"",
                  textToStr(alias), textToStr(mt));
        errFail();
    }
    for (; nonNull(mods); mods = tl(mods)) {
        m = hd(mods);
        if (m == currentModule) {
            /* Exporting the current module exports all local definitions */
            List xs;

            for (xs = moduleRec(m)->classes; nonNull(xs); xs = tl(xs)) {
                if (cclassRec(hd(xs))->mod == m)
                    exports =
                        checkExportClass(exports, mt, true, DOTDOT, hd(xs));
            }
            for (xs = moduleRec(m)->tycons; nonNull(xs); xs = tl(xs)) {
                if (tyconRec(hd(xs))->mod == m)
                    exports =
                        checkExportTycon(exports, mt, true, DOTDOT, hd(xs));
            }
            for (xs = moduleRec(m)->names; nonNull(xs); xs = tl(xs)) {
                if (nameRec(hd(xs))->mod == m &&
                    /* don't add dcons or class members */
                    (!isCfun(hd(xs)) && !isClass(nameRec(hd(xs))->parent))) {
                    checkExportDistinct(exports, false, hd(xs));
                    exports = cons(hd(xs), exports);
                }
            }
        } else {
            /* Re-exporting a module alias M exports all unqualified
              * entities that have been imported into scope by modules
              * having that alias _and_ for which the qualified (by
              * the _alias_ M) entities are also visible.
              */
            List xs = moduleRec(currentModule)->modImports;
            List ents = NIL;

            for (; nonNull(xs); xs = tl(xs)) {
                if (isPair(hd(xs)) && fst(hd(xs)) == m) {
                    ents = snd(snd(hd(xs)));
                    break;
                }
            }
            if (isNull(xs)) {
                ents = moduleRec(m)->exports;
            }
            for (; nonNull(ents); ents = tl(ents)) {
                Cell qid;
                Text txtNm;
                Cell ent = NIL;
                Name nm;
                Tycon tc;
                Class cc;

                /* Build the (alias) qualified entity and test whether it's
                  in scope -- ugly. */
                if (isName(hd(ents))) {
                    txtNm = nameRec(hd(ents))->text;
                    qid = mkQId(alias, mkVar(txtNm));
                } else if (isTycon(hd(ents))) {
                    txtNm = tyconRec(hd(ents))->text;
                    qid = mkQId(alias, mkCon(txtNm));
                } else if (isClass(hd(ents))) {
                    txtNm = cclassRec(hd(ents))->text;
                    qid = mkQId(alias, mkCon(txtNm));
                } else if (isQualIdent(hd(ents))) {
                    txtNm = qtextOf(hd(ents));
                    qid = mkQId(alias, snd(snd(hd(ents))));
                } else {
                    /* ({tycon|class}, [entity]) */
                    if (isTycon(fst(hd(ents)))) {
                        txtNm = tyconRec(fst(hd(ents)))->text;
                        qid = mkQId(alias, mkCon(txtNm));
                    } else if (isClass(fst(hd(ents)))) {
                        txtNm = cclassRec(fst(hd(ents)))->text;
                        qid = mkQId(alias, mkCon(txtNm));
                    } else {
                        internal("checkExportModule");
                    }
                }
                /* Decide whether an entity E is to be exported;
                * it needs to satisfy the following conditions:
                *
                *  - it needs to be visible in unqualified form,
                *    _unambiguously_.
                *  - it is also available as A.E (where A is the
                *    alias used in the module re-exportation element
                *    in the export list.)
                *  - the two names refer to the same (declared) name.
                */
                if (((ent = findName(txtNm)) &&
                     !isNull((nm = findQualName(qid))) &&
                     (nonNull((nameRec(ent)->clashes)) ||
                      nameRec(ent)->mod == nameRec(nm)->mod)) ||
                    ((ent = findTycon(txtNm)) &&
                     !isNull((tc = findQualTycon(qid))) &&
                     (nonNull((tyconRec(ent)->clashes)) ||
                      tyconRec(ent)->mod == tyconRec(tc)->mod)) ||
                    ((ent = findClass(txtNm)) &&
                     !isNull((cc = findQualClass(qid))) &&
                     (nonNull((cclassRec(ent)->clashes)) ||
                      cclassRec(ent)->mod == cclassRec(cc)->mod))) {

                    checkExportDistinct(exports, false, hd(ents));
                    exports = cons(hd(ents), exports);
                }
            }
        }
    }
    return exports;
}

static List allMethodsOrDCons(List imps, Cell nm, Module mod,
                              bool wantMethods) {
    /* For a non-local tycon / class exported using (..), locate
   * the list of dcons/methods that are in scope.
   *
   * This requires going through all the import lists, locating
   * the tycon/class and take the union of all the dcons/methods
   * found.
   */
    List xs;
    List resList = NIL;

    for (xs = imps; nonNull(xs); xs = tl(xs)) {
        if (isPair(hd(xs)) && isPair(snd(hd(xs)))) {
            List ns;
            /* Find the entry for 'nm' tycon.. */
            for (ns = snd(snd(hd(xs))); nonNull(ns); ns = tl(ns)) {
                if (isPair(hd(ns)) &&
                    ((!wantMethods && isTycon(fst(hd(ns))) &&
                      fst(hd(ns)) == nm && tyconRec(fst(hd(ns)))->mod == mod) ||
                     (wantMethods && isClass(fst(hd(ns))) &&
                      fst(hd(ns)) == nm &&
                      cclassRec(fst(hd(ns)))->mod == mod))) {
                    resList = dupOnto(snd(hd(ns)), resList);
                    /* Assumption: tycon/class may appear more than
             once in an import list; */
                }
            }
        }
    }
    if (nonNull(resList)) {
        resList = nubList(resList);
    }
    return resList;
}

/* Process entry in export list */
static List checkExport(List exports, Text mt, Cell e) {
    if (isIdent(e)) {
        Name export;
        bool expFound = false;

        if (isQCon(e) && nonNull(export = findQualTycon(e))) {
            expFound = true;
            exports = checkExportTycon(exports, mt, false, NIL, export);
        } else if (isQCon(e) && nonNull(export = findQualClass(e))) {
            /* opaque class export */
            expFound = true;
            exports = checkExportClass(exports, mt, false, NIL, export);
        } else if (nonNull(export = findQualName(e))) {
            /* Data constructors cannot appear in export lists,
             * so flag an error if they do.
             *
             * Notice that we have to be a bit careful when testing
             * for this, as both data constructors and type synonyms
             * have a tycon as parent. (In the case of type synonyms,
             * the parent is the type on the RHS.)
             *
             */
            if (isCfun(export) && !isPreludeScript() &&
                currentModule != moduleUserPrelude) {
                /* Special case reqd for Prelude(s) to handle (:) */
                errHead(0);
                errPrintf("Illegal export of a lone data constructor \"%s\"",
                          textToStr(nameRec(export)->text));
                errFail();
            }
            expFound = true;
            /* Re-use static analysis code to verify that 
           * a qualified export isn't ambiguous. Unqualified
           * ones are better handled by checkExportDistinct().
           */
            if (isQualIdent(e)) {
                depExpr(1, e);
            }
            checkExportDistinct(exports, !isQualIdent(e), export);
            exports = cons(export, exports);
        }
        if (!expFound) {
            errHead(0);
            errPrintf("Unknown entity \"%s\" exported from module \"%s\"",
                      identToStr(e), textToStr(mt));
            errFail();
        }
        return exports;
    } else if (MODULEENT == fst(e)) {
        return checkExportModule(exports, mt, e);
    } else {
        Cell ident = fst(e); /* class name or type name */
        Cell parts = snd(e); /* members or constructors */
        Cell nm;
        if (isQCon(ident) && nonNull(nm = findQualTycon(ident))) {
            switch (tyconRec(nm)->what) {
            case SYNONYM:
            case POLYSYN:
                if (DOTDOT != parts) {
                    errHead(0);
                    errPrintf(
                        "Explicit constructor list given for type synonym \"%s\" in export list of module \"%s\"",
                        identToStr(ident), textToStr(mt));
                    errFail();
                }
                exports = addEntity(nm, DOTDOT, exports);
                return exports;
            case RESTRICTSYN:
                errHead(0);
                errPrintf(
                    "Transparent export of restricted type synonym \"%s\" in export list of module \"%s\"",
                    identToStr(ident), textToStr(mt));
                errFail();
                return exports; /* Not reached */
            case NEWTYPE:
            case DATATYPE:
                if (DOTDOT == parts) {
                    Module thisModule = lastModule();
                    if (tyconRec(nm)->mod == thisModule) {
                        exports = addEntity(nm, DOTDOT, exports);
                    } else {
                        exports = addEntity(
                            nm,
                            allMethodsOrDCons(moduleRec(thisModule)->modImports,
                                              nm, tyconRec(nm)->mod, false),
                            exports);
                    }
                } else {
                    List ps = NIL;
                    ps = checkSubentities(ps, parts, tyconRec(nm)->defn,
                                          "constructor of type",
                                          tyconRec(nm)->text);
                    exports = addEntity(nm, ps, exports);
                }
                return exports;
            default:
                internal("checkExport1");
            }
        } else if (isQCon(ident) && nonNull(nm = findQualClass(ident))) {
            if (DOTDOT == parts) {
                Module thisModule = lastModule();
                if (cclassRec(nm)->mod == thisModule) {
                    exports = addEntity(nm, DOTDOT, exports);
                } else {
                    exports = addEntity(
                        nm,
                        allMethodsOrDCons(moduleRec(thisModule)->modImports, nm,
                                          cclassRec(nm)->mod, true),
                        exports);
                }
            } else {
                List ps = NIL;
                ps = checkSubentities(ps, parts, cclassRec(nm)->members,
                                      "member of class", cclassRec(nm)->text);
                exports = addEntity(nm, ps, exports);
            }
            return exports;
        } else {
            errHead(0);
            errPrintf(
                "Explicit export list given for non-class/datatype \"%s\" in export list of module \"%s\"",
                identToStr(ident), textToStr(mt));
            errFail();
        }
    }
    return exports; /*NOTUSED*/
}

List checkExports(List exports) {
    Module m = lastModule();
    Text mt = moduleRec(m)->text;
    List es = NIL; /* [Entity | (Entity,DOTDOT|NIL|[Entity])] */

    /* To properly handle methods and field names that are exported 
     * separately from their class/type ('orphans'), we construct the effective
     * export list in two passes. First, we resolve and collect up
     * all the entities, be they orphans or not. Secondly, we fix up
     * this list, attempting to join up the each 'orphan' with its
     * parent, but only if that parent (class/tycon) is also exported.
     * i.e., just exporting a method _does not_ cause its class to
     * implicitly be added to the export list.
     *
     * This only applies to methods and field names; data constructors
     * cannot be exported on their own.
     */
    map1Accum(checkExport, es, mt, exports);
    es = fixupIEList(es);

    return es;
}

/* --------------------------------------------------------------------------
 * Browsing module exports
 * ------------------------------------------------------------------------*/

/* Include all names in scope in the module? */
void browseModule(Module mod, bool all) {
    List exports = resolveImportList(mod, DOTDOT, false);
    Printf("module %s where\n", textToStr(moduleRec(mod)->text));
    if (all) {
        List all_names = dupList(moduleRec(mod)->names);
        mapProc(browseName, rev(all_names));
    } else {
        mapProc(browseEntity, exports);
    }
}

/* Entity | (Entity,[Entity]) */
static void browseEntity(Cell entity) {
    if (isName(entity)) {
        browseName(entity);
    } else { /* (Entity,[Entity]) */
        mapProc(browseName, snd(entity));
    }
}

static void browseName(Name nm) {
    /* unwanted artifacts, like lambda lifted values,
       are in the list of names, but have no types */
    if (nonNull(nameRec(nm)->type)) {
        printExp(stdout, nm);
        Printf(" :: ");
        printType(stdout, nameRec(nm)->type);
        if (isCfun(nm)) {
            Printf("  -- data constructor");
        } else if (isMfun(nm)) {
            Printf("  -- class member");
        } else if (isSfun(nm)) {
            Printf("  -- selector function");
        }
        if (nameRec(nm)->primDef) {
            Printf("  -- primitive");
        }
        Printf("\n");
    }
}
