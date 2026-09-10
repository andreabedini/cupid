/*
 * String utilities needed throughout the Cuddles codebase.
 */
#ifndef __STRUTIL_H__
#define __STRUTIL_H__

#include "prelude.h"

/* string copy operator, allocates new via malloc() */
extern String strCopy(String);

/* substring copy operator, allocates new via malloc() */
extern String strnCopy(String, Int);

/* Given a string containing a possibly qualified name,
 * split it up into a module and a name portion.
 */
extern Void splitQualString(String, String *, String *);

/*
 * Auto-expandable string builder objects:
 */
typedef struct StringBuilder StringBuilder;

/*
 * Function: newStringBuilder(sz)
 *
 * Allocate a new StringBuilder object, giving its
 * initial buffer size 'sz'. If sz == 0, the initial size
 * is set to INITIAL_BUILDER_SIZE..
 *
 */
extern StringBuilder *newStringBuilder(unsigned int sz);

/*
 * Function: freeStringBuilder(builder)
 *
 * Free up a StringBuilder object.
 */
extern void freeStringBuilder(StringBuilder *b);

/*
 * Function: appendString(builder, str)
 *
 * Append 'str' to the back of 'builder', expanding its
 * buffer to accommodate the added string, if needs be.
 *
 * Returns TRUE if successful.
 */
extern Bool appendString(StringBuilder *b, char *str);

/*
 * Function: toString(builder)
 *
 * Returns a pointer to the builder's internal buffer, i.e.,
 * a copy is not made (=> the caller isn't passed the
 * responsibility of freeing the returned string.)
 *
 * 
 */
extern char *toString(StringBuilder *b);

#endif /* __STRUTIL_H__ */
