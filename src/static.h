/* --------------------------------------------------------------------------
 * The parts of static.c that its siblings need.
 *
 * static.c keeps everything else to itself; these are the few functions and
 * the one list that deriving.c reaches back for.
 * ------------------------------------------------------------------------*/
#ifndef __STATIC_H__
#define __STATIC_H__

extern List cfunSfuns; /* [(Cfun, [SelectorVar])], for Show and Read       */

extern List classBindings(String, Class, List);
extern Void insertInst(Inst);
extern Name newInstImp(Inst);
extern Void kindInst(Inst, Int);
extern Int userArity(Name);
extern Type fullerExpand(Type);
extern Type checkSigType(Int, String, Cell, Type);
extern Name conDefined(Int, Cell, Bool);
extern Cell findBinding(Text, List);
extern Syntax lookupSyntax(Text); /* fixity from the enclosing bindings */

#endif /* __STATIC_H__ */
