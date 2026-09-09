/* --------------------------------------------------------------------------
 * Derived instances (deriving.c), as the static analysis sees them.
 * ------------------------------------------------------------------------*/
#ifndef __DERIVING_H__
#define __DERIVING_H__

extern Void checkDerive(Tycon, List, List, Cell); /* record a request       */
extern List derivedInstances(Void); /* implement them and return the list    */
extern Void derivingControl(Int);   /* RESET/INSTALL, called by everybody()  */

#endif /* __DERIVING_H__ */
