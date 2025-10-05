#include "globals.h"

WORD loadprog(BYTE *progname)
{
    LBLK pblk;
    WORD stat;

    pblk.l_file = progname;
    pblk.l_bias = 0;
    pblk.l_switch = 0;
    pblk.l_enad = &savepc;
    pblk.l_stat = &stat;

    iload(&pblk);
    return stat;
}
