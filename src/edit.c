#include "prelude.h"
#include <math.h>
#include "storage.h"
#include "connect.h"
#include "errors.h"
#include "opts.h"
#include "strutil.h"
#include "machdep.h"

Bool startEdit(Int line, String nm)    /* Start editor on file name at    */
                           /* given line.  Both name and line */
{               /* or just line may be zero        */
    String editorCmd;
    String fullNm;
    Bool   expandedName = FALSE;
    Bool   syncEdit     = TRUE;
    Bool   useShell     = FALSE;
    String he;
    String ec;
    unsigned int roomReqd = 0;
    unsigned int nmLen, lineLen, fullNmLen;
    
    /* First off, check whether we have actually got a plausible editor
     * available. On a Mac, files have creator information, telling which
     * program to launch to, so an editor named to the empty string ""
     * is often desirable.
     */
    if (!hugsEdit 
        || *hugsEdit == '\0'
	             ) {
	ERRMSG(0) "Hugs is not configured to use an editor"
	EEND;
    } 
      
    /* More sanity checks */
    if (nm == NULL) {
      return FALSE;
    }
    
    fullNm = RealPath(nm);
    fullNmLen = strlen(fullNm);
    nmLen = strlen(nm);
    lineLen = 1 + (line == 0 ? 0 : (unsigned int)log10((double)line));
    
    he = hugsEdit;

    /* Compute the length of the expanded 'hugsEdit' string */
    while (*he) {
      if (*he++ == '%') {
	if (*he == 's') {
	  /* assume quotes (shellQuote's) are always put around the name;
	   * this may over-allocate when they are not, but never under. */
	  roomReqd += (unsigned int)shellQuote(NULL,nm);
	  expandedName = TRUE;
	  he++;
	} else if (*he == 'f') {
	  roomReqd += (unsigned int)shellQuote(NULL,fullNm);
	  expandedName = TRUE;
	  he++;
	} else if ( *he == 'd' ) {
	  roomReqd += lineLen;
	  he++;
	} else if ( *he == '%' ) {
	  /* %% is contracted to % in the expanded string */
	  roomReqd++;
	  he++;
	} else if ( *he == '\0' ) {
	  /* trailing '%' with nothing after it: keep it literally, and
	   * don't step past the terminating NUL. */
	  roomReqd++;
	} else {
	  roomReqd += 2;
	  he++;
	}
      } else {
	roomReqd++;
      }
    }
    
    if (!expandedName) {
      /* include room for the shell-quoted name and a leading space */
      roomReqd += 1 + (unsigned int)shellQuote(NULL,nm);
    }
    
    editorCmd = (String)malloc(sizeof(char) * (roomReqd + 1));
    if (editorCmd == NULL) {
      Printf("Warning: Unable to start editor\n");
      return FALSE;
    }
    
    /* Given a properly sized output buffer, perform the expansion */
    expandedName = FALSE;
    ec = editorCmd;
    he = hugsEdit;
    
    /* If the editor command is prefixed with '&', the editor is
     * started up asynchronously (the default is for Hugs to block
     * and wait for the editor to exit).
     *
     * If the editor command is prefixed with '!', then the editor is
     * invoked by going via the shell.
     */
    while (1) {
      if (*he == '&') {
	syncEdit = FALSE;
	he++;
      } else if (*he == '!') {
	useShell = TRUE;
	he++;
      } else {
	break;
      }
    }
    
    while (*he) {
      if (*he=='%') {
	if (*++he=='d') {
	  sprintf(ec,"%d",(line ? line : 1));
	  ec += lineLen;
	  he++;
	} else if (*he == 's' || *he == 'f') {
	  /* Shell-quote it (see shellQuote in machdep.c) if the %s
	   * occurrence is surrounded by wspace only; the start/end of the
	   * template counts as whitespace too, so he[-2] is only looked at
	   * when it is within hugsEdit. Quoting protects against the name
	   * (module- or user-supplied) containing shell metacharacters. */
	  Bool useQuotes = (he-hugsEdit<2 || isspace(he[-2]))
			    && (he[1] == '\0' || isspace(he[1]));
	  String src = (*he == 's') ? nm : fullNm;
	  if (useQuotes) {
	    ec += shellQuote(ec,src);
	  } else {
	    strcpy(ec,src);
	    ec += (*he == 's') ? nmLen : fullNmLen;
	  }
	  expandedName = TRUE;
	  he++;
	} else if (*he == '%') { /* Unescape % */
	  *ec++ = '%';
	  he++;
	} else if (*he == '\0') {
	  /* trailing '%' with nothing after it: keep it literally, and
	   * don't step past the terminating NUL. */
	  *ec++ = '%';
	} else {
	  *ec++ = '%';
	  *ec++ = *he++;
	}
      } else {
	*ec++ = *he++;
      }
    }

    if (!expandedName) {  /* If file name was not included, add it. */
      *ec++=' ';
      /* Protect the filename with shell quoting */
      ec += shellQuote(ec,nm);
    }
    
    /* Terminate the string and go! */
    *ec = '\0';  

    if (shellEsc(editorCmd,syncEdit/*sync*/, useShell/*sep console*/)) {
      Printf("Warning: Editor terminated abnormally\n");
      free(editorCmd);
      return FALSE;
    }
    free(editorCmd);
    return syncEdit;
}
