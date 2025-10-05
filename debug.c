/*  primitive 8080 debugger for ISIS-II programs */

#include "portab.h"

/* register images */

EXTERN WORD savepsw;	    /* 8080 psw */
EXTERN WORD savebc;	   /* 8080 bc registers */
EXTERN WORD savede;	   /* 8080 de registers */
EXTERN WORD savehl;	   /* 8080 hl registers */
EXTERN WORD *savesp;	   /* 8080 stack register */
EXTERN BYTE *savepc;	   /* 8080 program counter */

/*  local variables */

BYTE cmdline[80];	/* command line */
BYTE *cptr;		/* pointer to command line */
BYTE *listptr;		/* next location to list (disassemble) */
BYTE opcode;		/* instruction to disassemble */
BYTE *dispptr;		/* next location to display in hex */
BYTE *breakpt = 0;	/* address of breakpoint location (0 if no breakpt) */
BYTE brkdata;		/* original contents of the breakpoint location */

debug()
{
    if (breakpt)
    {
	print("Breakpoint\r\n");
	savepc--;
	*savepc = brkdata;
	breakpt = 0;
    }
    else
	print("Illegal instruction\r\n");

    xcmd();
    listptr = savepc;
    for (;;)
    {
	print("-");
	read(0,cmdline,sizeof(cmdline));
	for (cptr = cmdline; *cptr != '\r'; cptr++)
	    if (*cptr >= 'a') *cptr -= 'a' - 'A';
	cptr = cmdline;
	switch(*cptr++)
	{
	    case 'G':
		if (gcmd()) return;
		return;
	    case 'D':
		dcmd();
		break;
	    case 'X':
		xcmd();
		break;
	    case 'L':
		lcmd();
		break;
	}
    }
}

/* GO command */

gcmd()
{
    BYTE *newpc, *newbrk;
    WORD gotpc;

    gotpc = gethex((WORD *)&newpc);
    if (*cptr++ == ',')
    {
	if (!gethex((WORD *)&newbrk))
	{
	    print("Illegal breakpoint\r\n");
	    return (FALSE);
	}
	breakpt = newbrk;
	brkdata = *breakpt;
	*breakpt = 8;
    }
    if (gotpc) savepc = newpc;
    return (TRUE);
}

/*  DISPLAY command */

dcmd()
{
    BYTE *newptr;
    WORD count;
    BYTE c;

    if (gethex((WORD *)&newptr))
	dispptr = newptr;
    else newptr = dispptr;
    for (count = 0; count < 64; count++)
    {
	if ((count & 0xf) == 0)
	{
	    phexw((WORD)dispptr);
	    print(" ");
	}
	print(" ");
	phexb(*dispptr++);
	if ((count & 0xf) == 0xf)
	{
	    print("   ");
	    while (newptr < dispptr)
	    {
		c = *newptr++ & 0x7f;
		if (c < ' ') c = '.';
		write(1,&c,1);
	    }
	    print("\r\n");
	}
    }
}

/* XCMD - print register values */

BYTE *flagchar = "SZ-A-P-C";

xcmd()
{
    WORD flags, count;

    print("A=");
    phexb(savepsw);
    print(" BC=");
    phexw(savebc);
    print(" DE=");
    phexw(savede);
    print(" HL=");
    phexw(savehl);
    print(" M=");
    phexb(*((BYTE *)savehl));
    print("\r\nPC=");
    phexw((WORD)savepc);
    print(" SP=");
    phexw((WORD)savesp);
    print(" FLAGS=");
    flags = savepsw;
    for (count = 0; count < 8; count++)
    {
	if (flags & 0x8000)
	    write(1,flagchar+count,1);
	else
	    print("-");
	flags <<= 1;
    }
    print("\r\n");
}


/* LCMD - disassemble stuff */

#if 0
WORD (*gtab[4])() = {group0,group1,group2,group3} ;
#endif

lcmd()
{
#if 0
    BYTE *newptr;
    WORD count,count1;
    WORD opsize;

    if (gethex((WORD *)&newptr))
	listptr = newptr;
    for (count = 0; count < 12; count++)
    {
	phexw(listptr);
	print("  ");
	newptr = listptr;
	opcode = *listptr;
	(*gtab[oprec >> 6) & 3])();
	print("\r\n");
    }
#else
    print("Not implemented\r\n");
#endif
}

/*  GETHEX - get a 16-bit hex number */

gethex(valptr)
WORD *valptr;
{
    WORD count,c;

    *valptr = 0;
    for (count = 0; ; count++,cptr++)
    {
	c = *cptr;
	if (c >= '0' && c <= '9') c -= '0';
	else if (c >= 'A' && c <= 'F') c -= 'A' - 10;
	else return (count);
	*valptr = (*valptr << 4) + c;
    }
}

/* PHEXW - print hex word */

phexw(w)
WORD w;
{
    phexb(w >> 8);
    phexb(w);
}

/* PHEXB - print hex byte */

phexb(b)
BYTE b;
{
    phexn(b >> 4);
    phexn(b);
}

/* PHEXN - print hex nibble */

phexn(n)
BYTE n;
{
    write(1,"0123456789ABCDEF"+(n & 15),1);
}
