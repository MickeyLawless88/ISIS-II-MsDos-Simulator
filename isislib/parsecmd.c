#include "globals.h"

BYTE *parsecmd(BYTE *cmdline, BYTE **argstart)
{
    BYTE *progname;
    BYTE *p;

    p = cmdline;
    while (*p == ' ' || *p == '\t') p++;

    if (*p == '\0') {
        *argstart = (BYTE *)0;
        return (BYTE *)0;
    }

    progname = p;
    while (*p && *p != ' ' && *p != '\t') p++;

    if (*p == '\0') {
        *argstart = p;
    } else {
        *p = '\0';
        p++;
        while (*p == ' ' || *p == '\t') p++;
        *argstart = p;
    }
    return progname;
}
