#include "globals.h"

void setupcmdtail(BYTE *progname, BYTE *args)
{
    FCB *f;
    BYTE *cmdtail;
    BYTE *fcb;
    BYTE fullcmd[128];
    BYTE fname[9], fext[4];
    BYTE *p;
    int i;
    WORD count;

    f = &fcbs[1];              /* FCB for :CI: */
    cmdtail = (BYTE *)0x80;    /* ISIS-II command tail */
    fcb = (BYTE *)0x5C;        /* FCB location */

    /* Build the full command line: "progname args" */
    fullcmd[0] = '\0';
    strcpy(fullcmd, progname);
    if (args && *args && *args != '\r') {
        strcat(fullcmd, " ");
        strcat(fullcmd, args);
    }

    /* Convert to uppercase (ISIS programs expect uppercase) */
    for (i = 0; fullcmd[i]; i++) {
        if (fullcmd[i] >= 'a' && fullcmd[i] <= 'z')
            fullcmd[i] -= ('a' - 'A');
    }

    /* Clear FCB at 0x5C */
    setb(0, fcb, 36);

    /* Parse first filename from args into FCB (not progname) */
    p = args;
    while (p && (*p == ' ' || *p == '\t')) p++;  /* skip spaces */
    if (p && *p) {
        /* Extract filename (up to 8 chars) */
        for (i = 0; i < 8 && *p && *p != '.' && *p != ' ' && *p != '\t'; i++, p++)
            fname[i] = (*p >= 'a' && *p <= 'z') ? (*p - 'a' + 'A') : *p;
        while (i < 8) fname[i++] = ' ';
        /* Extract extension (up to 3 chars) */
        i = 0;
        if (*p == '.') {
            p++;
            for (i = 0; i < 3 && *p && *p != ' ' && *p != '\t'; i++, p++)
                fext[i] = (*p >= 'a' && *p <= 'z') ? (*p - 'a' + 'A') : *p;
        }
        while (i < 3) fext[i++] = ' ';
        fcb[0] = 0;
        movb(fname, &fcb[1], 8);
        movb(fext, &fcb[9], 3);
    }

    /* Build command tail at 0x80 */
    count = strlen(fullcmd);
    cmdtail[0] = count + 1;         /* include leading space */
    cmdtail[1] = ' ';
    movb(fullcmd, &cmdtail[2], count);
    cmdtail[count + 2] = '\r';

    /* Copy the full command line into f_lbuf */
    movb(fullcmd, f->f_lbuf, count);
    movb("\r\n", &f->f_lbuf[count], 2);
    f->f_count = count + 2;
    f->f_index = 0;
}