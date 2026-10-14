/*
 * String utilities needed throughout the Cupid codebase.
 */
#include "prelude.h"
#include "errors.h"
#include "strutil.h"
#include "char.h"

/* --------------------------------------------------------------------------
 * String manipulation routines:
 * ------------------------------------------------------------------------*/

/* Make malloced copy of a string */
char *strCopy(char *s) {
    if (s) {
        char *t;
        if ((t = (char *)malloc(strlen(s) + 1)) == 0) {
            errHead(0);
            errPrintf("String storage space exhausted");
            errFail();
        }
        strcpy(t, s);
        return t;
    }
    return NULL;
}

/* Make malloced copy of a substring */
char *strnCopy(char *s, int n) {
    if (s) {
        char *t;
        if ((int)strlen(s) < n)
            n = (int)strlen(s);
        if ((t = (char *)malloc(n + 1)) == 0) {
            errHead(0);
            errPrintf("String storage space exhausted");
            errFail();
        }
        strncpy(t, s, n);
        t[n] = '\0';
        return t;
    }
    return NULL;
}

/* Given a string containing a possibly qualified name,
 * split it up into a module and a name portion.
 */
void splitQualString(char *nm, char **pMod, char **pName) {
    char *dot;

    if (*nm == '\0') { /* nothing to split in an empty    */
        *pMod = NULL;  /* string; nm+strlen(nm)-1 would   */
        *pName = nm;   /* point before the start of nm    */
        return;
    }

    /* Find the last occurrence of '.' preceded by an identifier */
    dot = nm + strlen(nm) - 1;
    while (dot != nm && !(*dot == '.' && isIn(dot[-1], IDAFTER)))
        dot--;

    if (dot == nm) {
        *pMod = NULL;
        *pName = nm;
    } else {
        /* The module portion consists of everything upto the last dot. */
        *pMod = strnCopy(nm, (int)(dot - nm));

        /* Everything after the last '.' is the name string */
        *pName = dot + 1;
    }
}

/* Cheap&cheerful expandable strings / StringBuilders.
 *
 * Note: I'm willfully breaking the convention of using K&R style
 * function declarations here. Their time has come and gone.
 */
struct StringBuilder {
    unsigned int len;
    unsigned int size;
    char *buf;
};

#define INITIAL_BUILDER_SIZE 200

static bool expandStringBuilder(StringBuilder *b, unsigned int newSz);

/*
 * Function: newStringBuilder(sz)
 *
 * Allocate a new StringBuilder object, giving its
 * initial buffer size 'sz'. If sz == 0, the initial size
 * is set to INITIAL_BUILDER_SIZE..
 *
 */
StringBuilder *newStringBuilder(unsigned int sz) {
    StringBuilder *b = (StringBuilder *)malloc(sizeof(StringBuilder));

    if (!b)
        return NULL;

    b->size = (sz == 0 ? INITIAL_BUILDER_SIZE : sz);
    b->len = 0;
    b->buf = (char *)malloc(sizeof(char) * b->size);

    if (!b->buf) {
        free(b);
        return NULL;
    }
    *(b->buf) = '\0';
    return b;
}

/*
 * Function: expandStringBuilder(builder, newSz)
 *
 * Internal function for expanding a StringBuilder's internal
 * buffer to 'newSz'. Along with expansion the old contents of
 * the buffer is copied over. 
 * 
 * Returns true if successful.
 */
static bool expandStringBuilder(StringBuilder *b, unsigned int newSz) {
    char *buf;

    if (!b)
        return false;
    if (b->size > newSz)
        return false;

    buf = realloc(b->buf, newSz);
    if (!buf)
        return false;
    b->size = newSz;
    b->buf = buf;

    return true;
}

/*
 * Function: appendString(builder, str)
 *
 * Append 'str' to the back of 'builder', expanding its
 * buffer to accommodate the added string, if needs be.
 *
 * Returns true if successful.
 */
bool appendString(StringBuilder *b, char *str) {
    unsigned int len, newLen;

    if (!b)
        return false;
    if (!str)
        return true;

    len = strlen(str);
    newLen = b->len + len;
    if (len == 0)
        return true;

    if (newLen >= b->size) {
        unsigned int newSz = b->size * 2;
        if (newSz < newLen) {
            newSz = newLen;
        }
        if (!expandStringBuilder(b, newSz)) {
            return false;
        }
    }

    memcpy(b->buf + b->len, str, len + 1);
    b->len = newLen;

    return true;
}

/*
 * Function: toString(builder)
 *
 * Returns a pointer to the builder's internal buffer, i.e.,
 * a copy is not made (=> the caller isn't passed the
 * responsibility of freeing the returned string.)
 *
 * 
 */
char *toString(StringBuilder *b) {
    if (!b)
        return NULL;
    return (b->buf);
}

/*
 * Function: freeStringBuilder(builder)
 *
 * Free up a StringBuilder object.
 */
void freeStringBuilder(StringBuilder *b) {
    if (!b)
        return;
    if (b->buf)
        free(b->buf);
    b->buf = NULL;
    free(b);
}
