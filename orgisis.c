/*  isis.c - ISIS-II simulator for IBM PC */

#include "portab.h"

#define TRACE FALSE	    /* TRUE if all ISIS-II calls are to be traced */


/***********************************************************************
*  External variables in START.ASM, RUN80.ASM/SIM80.ASM, and SIO.C
***********************************************************************/

EXTERN BYTE psp[256];	    /* program segment prefix */
EXTERN WORD stak;	    /* top of C stack */
EXTERN WORD savepsw;	    /* 8080 psw */
EXTERN WORD savebc;	    /* 8080 bc registers */
EXTERN WORD savede;	    /* 8080 de registers */
EXTERN WORD savehl;	    /* 8080 hl registers */
EXTERN WORD *savesp;	    /* 8080 stack register */
EXTERN BYTE *savepc;	    /* 8080 program counter */
EXTERN BYTE v20;	    /* true if using a NEC V20 CPU chip */
EXTERN WORD o1delay;	    /* no. of milliseconds to delay on serial output */

/***********************************************************************
* External PC-DOS 2.0 routines in START.ASM (Unix-like calls).
* These routines return -1 as error code, except for getenv,
* which returns NULL.
***********************************************************************/

EXTERN WORD read();
EXTERN WORD write();
EXTERN LONG lseek();
EXTERN WORD close();
EXTERN WORD unlink();	    /* delete a file */
EXTERN WORD rename();
EXTERN BYTE *getenv();

/***********************************************************************
* Code to place at RST 7 interrupt handler to cause a return from
* 8080 emulation on the V20.  Not used on 8088.
***********************************************************************/

BYTE rst7_v20[] =
{
	0xed,   /* RETEM - return from emulation */
	0xfd,
};

#define RST7	(BYTE *)0x38		/* address of RST 7 handler */

/***********************************************************************
* ISIS, MDS Monitor entry points and special locations
***********************************************************************/

#define ISIS	0x40		/* ISIS entry point */
#define	CI	0xF803		/* console input */
#define	CO	0xF809		/* console output */
#define	CSTS	0xF812		/* console input status */
#define	MEMCK	0xF81B		/* compute size of memory */

/* IOBYTE, and its bit definitions
 */
#define IOBYTE	(BYTE *)3	/* address of IOBYTE */
#define	CONBIT	3		/* console bit mask */
#define	CRTCON	1		/* console = crt */
#define	USRCON	3		/* console = user defined */


BYTE brk_v20[] = {	/* instructions to break emulation on V20 */
	0xff,		/* RST 7 */
	0xc9		/* RET */
};

BYTE brk_8088[] = {	/* instructions to break emulation on 8088 */
	0xfd,		/* illegal instruction */
	0xc9		/* RET */
};

/***********************************************************************
*  MAIN ROUTINE - initialize MDS memory, ISIS-II file system,
*    then load program specified on command line and run it.
***********************************************************************/

main()
{
    BYTE *brkinst;
    WORD sizebrk;

    if (v20)
    {
	/* Poke instruction to break emulation at RST 7 handler
	 */
	movb(rst7_v20,RST7,sizeof(rst7_v20));
	brkinst = brk_v20;
	sizebrk = sizeof(brk_v20);
    }
    else
    {
	brkinst = brk_8088;
	sizebrk = sizeof(brk_8088);
    }

    /* Poke instructions to break emulation at ISIS and monitor
     * entry points.  On the V20, these instruction are {RST 7, RET},
     * and the code at the RST 7 interrupt handler actually breaks
     * emulation.
     */
    movb(brkinst,(BYTE *)ISIS,sizebrk);
    movb(brkinst,(BYTE *)CI,sizebrk);
    movb(brkinst,(BYTE *)CO,sizebrk);
    movb(brkinst,(BYTE *)CSTS,sizebrk);
    movb(brkinst,(BYTE *)MEMCK,sizebrk);

    /* Initialize ISIS-II file system and command tail,
     * then load the program.
     */
    initisis();
#if TRACE
    debug();		    /* enter the debugger */
#endif

    /* Start the loaded program.
     */
    go8080();
}

/***********************************************************************
*  ISIS file control block structure
***********************************************************************/

#define FCB struct _fcb
FCB {
    WORD    f_fd;	    /* DOS file descriptor, or -1 if FCB not used */
    WORD    f_access;	    /* ISIS access code */
    WORD    f_led;	    /* true if this is a line edited file */
    WORD    f_count;	    /* number of bytes in line edit buffer */
    WORD    f_index;	    /* index into line edit buffer */
    BYTE    f_lbuf[122];    /* line edit buffer */
} ;

#define NFCBS 10	    /* allow up to 10 open files */
FCB fcbs[NFCBS];


/***********************************************************************
*  ISIS-II system call parameter block formats
***********************************************************************/

#define OBLK struct _oblk   /* OPEN call */
OBLK {
    WORD    *o_aft;	    /* pointer to AFTN */
    BYTE    *o_file;	    /* pointer to filename */
    WORD    o_access;	    /* read = 1, write = 2, update = 3 */
    WORD    o_echo;	    /* if nonzero, aftn of echo output file */
    WORD    *o_stat;	    /* pointer to status */
} ;

#define RBLK struct _rblk   /* READ call */
RBLK {
    WORD    r_aft;	    /* file aftn */
    BYTE    *r_buf;	    /* pointer to buffer */
    WORD    r_cnt;	    /* number of bytes to read */
    WORD    *r_act;	    /* pointer to actual no. of bytes read */
    WORD    *r_stat;	    /* pointer to status */
} ;

#define WBLK struct _wblk   /* WRITE call */
WBLK {
    WORD    w_aft;	    /* file aftn */
    BYTE    *w_buf;	    /* pointer to buffer */
    WORD    w_cnt;	    /* number of bytes to write */
    WORD    *w_stat;	    /* pointer to status */
} ;

#define SBLK struct _sblk   /* SEEK call */
SBLK {
    WORD    s_aft;	    /* file aftn */
    WORD    s_mode;	    /* type of seek */
    WORD    *s_blks;	    /* pointer to blks */
    WORD    *s_nbyte;	    /* pointer to nbyte */
    WORD    *s_stat;	    /* pointer to status */
} ;

#define RSBLK struct _rsblk /* RESCAN call */
RSBLK {
    WORD    rs_aft;	    /* file aftn */
    WORD    *rs_stat;	    /* pointer to status */
} ;

#define CBLK struct _cblk   /* CLOSE call */
CBLK {
    WORD    c_aft;	    /* file aftn */
    WORD    *c_stat;	    /* pointer to status */
} ;

#define DBLK struct _dblk   /* DELETE call */
DBLK {
    BYTE    *d_file;	    /* pointer to filename */
    WORD    *d_stat;	    /* pointer to status */
} ;

#define RNBLK struct _rnblk /* RENAME call */
RNBLK {
    BYTE    *rn_file1;	    /* old filename */
    BYTE    *rn_file2;	    /* pointer to new filename */
    WORD    *rn_stat;	    /* pointer to status */
} ;

#define LBLK struct _lblk   /* LOAD call */
LBLK {
    BYTE    *l_file;	    /* pointer to filename */
    WORD    l_bias;	    /* address bias */
    WORD    l_switch;	    /* control switch */
    BYTE    **l_enad;	    /* pointer to entry address */
    WORD    *l_stat;	    /* pointer to status */
} ;

#define EBLK struct _eblk   /* ERROR call */
EBLK {
    WORD    e_ernum;	    /* error number */
    WORD    *e_stat;	    /* pointer to status */
} ;

#define SPBLK struct _spblk /* SPATH call */
SPBLK {
    BYTE    *sp_file;	    /* pointer to filename */
    BYTE    *sp_buf;	    /* pointer to buffer */
    WORD    *sp_stat;	    /* pointer to status */
} ;

/***********************************************************************
*  MONITOR - handle MDS monitor calls
***********************************************************************/

monitor()
{
    WORD c;

    switch ((WORD)(savepc - 1))
    {
    case CI:	/* console input */
	if ((*IOBYTE & CONBIT) == USRCON)	/* user-define console?	*/
	    c = siordchar();			/* read from COM1:	*/
	else
	    c = bdos(7,0);			/* read from CON:	*/
	savepsw = (savepsw & 0xff00) | c;	/* return char in A reg	*/
	break;
    case CO:	/* console output */
	c = savebc & 0xff;			/* character in C reg	*/
	if ((*IOBYTE & CONBIT) == USRCON)	/* user-define console?	*/
	    siowrchar(c);			/* write to COM1:	*/
	else
	    bdos(2,c);				/* write to CON:	*/
	break;
    case CSTS:		/* console status */
	if ((*IOBYTE & CONBIT) == USRCON)	/* user-define console?	*/
	    c = siochkchar();			/* get status of COM1:	*/
	else
	    c = bdos(11,0);			/* get status of CON:	*/
	savepsw = (savepsw & 0xff00);		/* return char in A reg	*/
	if (c)
	    savepsw |= 0xff;
	break;
    case MEMCK:
	savebc = (savebc & 0xff) | 0xf500;	/* top of memory is F5FF */
	savepsw |= 0xff;			/* to keep MPPROM happy */
	break;
    }
}

/***********************************************************************
*  CVTFNAME - convert an ISIS-II filename to a DOS ASCIIZ filename
***********************************************************************/

BYTE fname1[65];
BYTE fname2[65];

cvtfname(name,dest,cvtdevice)
BYTE *name;
BYTE *dest;
WORD cvtdevice;		/* 0 - don't convert device name; 1-convert	*/
{
    WORD count;
    BYTE c;
    BYTE device[5];
    BYTE *env;

    while (*name == ' ') name++;
    count = 0;

    /* Check for ISIS device name :XX:.  If present, look it up in
     * the environment strings, and if found, use the found value.
     * This allows the user to define ISIS drive names, for example:
     *		set :f1:=\plm80\includes
     */
    if (name[0] == ':' && name[3] == ':' && cvtdevice)
    {
	/* Copy the device name, converting to upper case.
	 */
	device[0] = device[3] = ':';
	if (name[1] >= 'a' && name[1] <= 'z')
	    device[1] = name[1] - 'a' + 'A';
	else
	    device[1] = name[1];
	if (name[2] >= 'a' && name[2] <= 'z')
	    device[2] = name[2] - 'a' + 'A';
	else
	    device[2] = name[2];
	device[4] = 0;

	/* Look up the definition of the device name.  If found,
	 * copy its value.
	 */
	if ((env = getenv(device)) != 0)	/* is it defined?	*/
	{
	    strcpy(dest,env);		/* copy the definition		*/
	    dest += (count = strlen(dest)); /* look at the end		*/
	    if (*(dest-1) != '\\')	/* is last character a \ ?	*/
	    {
		*dest++ = '\\';		/* no - append one		*/
		count++;
	    }
	    name += 4;			/* skip over device name	*/
	}
    }

    /* copy the rest of the filename, up to the first non-filename
     * type character (e.g space).
     */
    while (count < sizeof(fname1))
    {
	c = *name++;
	if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
	if (c == ':' || c == '.' || (c >= 'A' && c <= 'Z')
	    || (c >= '0' && c <= '9'))
	{
	    *dest++ = c;
	    count++;
	}
	else		/* must be end of filename */
	{
	    *dest = 0;	 /* terminate converted filename with null */
	    return;
	}
    }
}


/***********************************************************************
*  SKIPDRV - skip past the drive part of an ISIS filename
***********************************************************************/

BYTE *skipdrv(fname)
BYTE *fname;
{
    if (*fname == ':')          /* skip past ISIS-II drive code :F0: etc. */
    {
	fname++;
	while (*fname)
	    if (*fname++ == ':') break;
    }
    return (fname);
}


/***********************************************************************
*  CLEANUP - clean up serial I/O vectors and exit if control-C is hit
***********************************************************************/

cleanup()
{
    siouninit();
    exit(0);
}

/***********************************************************************
*  SETSTAT - set ISIS-II function call return status, print a message
*    if tracing is enabled.
***********************************************************************/

void setstat(stat,val)
WORD *stat;
WORD val;
{
    *stat = val;
#if TRACE
    print(" status=");
    phexw(val);
    print("\r\n");
#endif
}


/***********************************************************************
*  GETFCB - check for valid open file, return FCB indexed by AFTN
***********************************************************************/

FCB *getfcb(aftn,stat)
WORD aftn;
WORD *stat;
{
    if (aftn < 0 || aftn >= NFCBS || fcbs[aftn].f_fd == -1)
    {
	setstat(stat, 2);	/* AFTN does not specify an open file */
	return (0);
    }
    return (&fcbs[aftn]);
}


/***********************************************************************
*  IOPEN - ISIS-II OPEN call
***********************************************************************/

void iopen(pblk)
OBLK *pblk;
{
    WORD aft;
    BYTE *fname;
    FCB *f;

    cvtfname(pblk->o_file,fname1,1);  /* convert filename */
#if TRACE
    print("OPEN: name=");
    print(fname1);
    print(" aftnptr=");
    phexw(pblk->o_aft);
    print(" access=");
    phexw(pblk->o_access);
    print(" echoaftn=");
    phexw(pblk->o_echo);
#endif
    if (strcmp(fname1,":CO:") == 0)
    {
	*pblk->o_aft = 0;
	*pblk->o_stat = 0;
#if TRACE
	print("\r\n");
#endif
	return;
    }
    if (strcmp(fname1,":CI:") == 0)
    {
	*pblk->o_aft = 1;
	*pblk->o_stat = 0;
#if TRACE
	print("\r\n");
#endif
	return;
    }

/* search for a free FCB */

    for (aft = 2; aft < 10; aft++)
	if (fcbs[aft].f_fd == -1) break;
    if (aft == 10)	    /* no more FCBs */
    {
	setstat(pblk->o_stat, 3);
	return;
    }
    f = &fcbs[aft];

/* call DOS to open the file */

    fname = skipdrv(fname1);
    switch (pblk->o_access)
    {
	case 1: 		    /* open for read */
	    f->f_fd = open(fname,0);
	    break;
	case 2: 		    /* open for write */
	    f->f_fd = creat(fname,0);
	    break;
	case 3: 		    /* open for update */
	    if ((f->f_fd = open(fname,2)) == -1) /* try to open existing file */
		f->f_fd = creat(fname,0);	/* create new file */
	    break;
	default:
	    setstat(pblk->o_stat, 33);
	    return;
    }
    if (f->f_fd == -1)
    {
	if (pblk->o_access > 1)
	{
	    setstat(pblk->o_stat, 14);	   /* unable to open for write */
	    return;
	}
	else
	{
	    setstat(pblk->o_stat, 13);    /* unable to open for read */
	    return;
	}
    }

/* set up other fields in FCB */

    f->f_access = pblk->o_access;   /* save access code */
    f->f_led = pblk->o_echo;	/* line-editing flag */
    f->f_count = f->f_index = 0;    /* line-editing buffer indexes */
    *pblk->o_aft = aft; 	/* return AFTN to caller */
    *pblk->o_stat = 0;		/* status is OK */
#if TRACE
    print(" aftn=");
    phexw(aft);
    print("\r\n");
#endif
}


/***********************************************************************
* IREAD - ISIS-II READ call
***********************************************************************/

void iread(pblk)
RBLK *pblk;
{
    FCB *f;
    WORD nchars;
    BYTE c;

#if TRACE
    print("READ: aftn=");
    phexw(pblk->r_aft);
    print(" buf=");
    phexw(pblk->r_buf);
    print(" count=");
    phexw(pblk->r_cnt);
    print(" actptr=");
    phexw(pblk->r_act);
    print(" statptr=");
    phexw(pblk->r_stat);
#endif
    if ((f = getfcb(pblk->r_aft,pblk->r_stat)) == 0) return;
    if (f->f_access != 1 && f->f_access != 3)
    {
	setstat(pblk->r_stat, 8);    /* attempt to read file open for output */
	return;
    }
    if (f->f_led)		 /* line-edited file? */
    {
	if (f->f_index == f->f_count)	/* line buffer empty */
	{
	    if (pblk->r_aft == 1)   /* :CI: has to be handled specially */
		nchars = read(0,f->f_lbuf,sizeof(f->f_lbuf));
	    else		/* normal disk file - read up to CR/LF */
	    {
		nchars = 0;
		c = 0;
		while (c != '\n' && nchars < sizeof(f->f_lbuf))
		{
		    if (read(f->f_fd,&c,1) != 1) break;
		    if (c == 0x1a) continue;	    /* ignore CTRL/Z */
		    f->f_lbuf[nchars++] = c;
		}
	    }
	    f->f_count = nchars;
	    f->f_index = 0;
	}
	if ((nchars = f->f_count - f->f_index) > pblk->r_cnt)
	    nchars = pblk->r_cnt;
	movb(&f->f_lbuf[f->f_index],pblk->r_buf,nchars);
	f->f_index += nchars;
	*pblk->r_stat = 0;
    }
    else		    /* not a line-edited file */
    {
	nchars = read(f->f_fd,pblk->r_buf,pblk->r_cnt);
	if (nchars == -1)   /* disk error */
	{
	    setstat(pblk->r_stat, 24);	   /* ISIS disk error code */
	    return;
	}
	else *pblk->r_stat = 0;
    }
#if TRACE
    print(" actual=");
    phexw(nchars);
    print("\r\n");
#endif
    *pblk->r_act = nchars;	    /* return actual no. of bytes read */
}


/***********************************************************************
* IWRITE - ISIS-II WRITE call
***********************************************************************/

void iwrite(pblk)
WBLK *pblk;
{
    FCB *f;
    WORD nchars;

#if TRACE
    print("WRITE: aftn=");
    phexw(pblk->w_aft);
    print(" buf=");
    phexw(pblk->w_buf);
    print(" count=");
    phexw(pblk->w_cnt);
    print(" statptr=");
    phexw(pblk->w_stat);
#endif
    if ((f = getfcb(pblk->w_aft,pblk->w_stat)) == 0) return;
    if (f->f_access != 2 && f->f_access != 3)
    {
	setstat(pblk->w_stat, 6);	/* attempt to write file open for input */
	return;
    }
    nchars = write(f->f_fd,pblk->w_buf,pblk->w_cnt);
#if TRACE
    print(" actual=");
    phexw(nchars);
    print("\r\n");
#endif
    if (nchars != pblk->w_cnt)
	*pblk->w_stat = 7;
    else
	*pblk->w_stat = 0;
}


/***********************************************************************
* ICLOSE - ISIS-II CLOSE call
***********************************************************************/

void iclose(pblk)
CBLK *pblk;
{
    FCB *f;

#if TRACE
    print("CLOSE: aftn=");
    phexw(pblk->c_aft);
    print(" statptr=");
    phexw(pblk->c_stat);
#endif
    if ((f = getfcb(pblk->c_aft,pblk->c_stat)) == 0) return;
    if (pblk->c_aft < 2)    /* ignore attempts to close console files */
    {
#if TRACE
	print("\r\n");
#endif
	*pblk->c_stat = 0;
	return;
    }
    if (close(f->f_fd) == -1)
    {
	setstat(pblk->c_stat, 24);
	return;
    }
    else
    {
#if TRACE
    print("\r\n");
#endif
	*pblk->c_stat = 0;
    }
    f->f_fd = -1;	    /* mark FCB as unused */
}


/***********************************************************************
* IDELETE - ISIS-II DELETE call
***********************************************************************/

void idelete(pblk)
DBLK *pblk;
{
    cvtfname(pblk->d_file,fname1,1);
#if TRACE
    print("DELETE: name=");
    print(fname1);
    print(" statptr=");
    phexw(pblk->d_stat);
#endif
    if (unlink(skipdrv(fname1)) == -1)
    {
	setstat(pblk->d_stat, 13);
	return;
    }
    else *pblk->d_stat = 0;
#if TRACE
    print("\r\n");
#endif
}


/***********************************************************************
* IRENAME - ISIS-II RENAME call
***********************************************************************/

void irename(pblk)
RNBLK *pblk;
{
    cvtfname(pblk->rn_file1,fname1,1);	/* convert old filename */
    cvtfname(pblk->rn_file2,fname2,1);	/* convert new filename */
#if TRACE
    print("DELETE: old=");
    print(fname1);
    print(" new=");
    print(fname2);
    print(" statptr=");
    phexw(pblk->rn_stat);
#endif
    if (rename(fname1,fname2) == -1)
    {
	setstat(pblk->rn_stat, 13);
	return;
    }
    else *pblk->rn_stat = 0;
#if TRACE
    print("\r\n");
#endif
}


/***********************************************************************
* ISEEK - ISIS-II SEEK call
***********************************************************************/

void iseek(pblk)
SBLK *pblk;
{
    FCB *f;
    LONG offset;	    /* PC-DOS lseek offset */
    WORD mode;		    /* PC-DOS lseek mode */

#if TRACE
    print("SEEK: aftn=");
    phexw(pblk->s_aft);
    print(" mode=");
    phexw(pblk->s_mode);
    print(" block=");
    phexw(*pblk->s_blks);
    print(" byteno=");
    phexw(*pblk->s_nbyte);
    print(" statptr=");
    phexw(pblk->s_stat);
    print("\r\n");
#endif
    if ((f = getfcb(pblk->s_aft,pblk->s_stat)) == 0) return;
    if (pblk->s_aft < 2)    /* ignore attempts to close console files */
    {
	setstat(pblk->s_stat, 19); /* attempt to seek on non-disk file */
	return;
    }

/* compute DOS offset from ISIS-II offset */

    offset = (*pblk->s_blks) & 0x7fff;
    offset = (offset << 7) + *pblk->s_nbyte;

/* convert ISIS-II seek mode to DOS seek mode */

    switch (pblk->s_mode)
    {
	case 0: 	/* return current position */
	    offset = 0;
	    mode = 1;
	    break;
	case 1: 	/* move marker backwards */
	    offset = -offset;
	    mode = 1;
	    break;
	case 2: 	/* move to specific position */
	    mode = 0;
	    break;
	case 3: 	/* move forward */
	    mode = 1;
	    break;
	case 4: 	/* move to end of file */
	    offset = 0;
	    mode = 2;
	    break;
	default:
	    setstat(pblk->s_stat, 27);   /* incorrect mode parameter */
	    return;
    }
#if TRACE
    print("Desired offset: ");
    phexw((WORD)(offset >> 16));
    phexw((WORD)offset);
#endif
    offset = lseek(f->f_fd,offset,mode);    /* get new file position */
#if TRACE
    print(" DOS mode=");
    phexw(mode);
    print(" result offset=");
    phexw((WORD)(offset >> 16));
    phexw((WORD)offset);
    print("\r\n");
#endif
    if (pblk->s_mode == 0)	    /* return current position to caller? */
    {
	*pblk->s_nbyte = offset & 0x7f;
	*pblk->s_blks = offset >> 7;
    }
    if (offset == -1)
	*pblk->s_stat = 20;	    /* bad seek */
    else
	*pblk->s_stat = 0;
}


/***********************************************************************
*  IRESCAN - ISIS-II RESCAN call
***********************************************************************/

void irescan(pblk)
RSBLK *pblk;
{
    FCB *f;

#if TRACE
    print("RESCAN: aftn=");
    phexw(pblk->rs_aft);
    print(" statptr=");
    phexw(pblk->rs_stat);
#endif
    if ((f = getfcb(pblk->rs_aft,pblk->rs_stat)) == 0) return;
    if (!f->f_led)	/* not line-edited file */
    {
	setstat(pblk->rs_stat, 21);
	return;
    }
    f->f_index = 0;
    *pblk->rs_stat = 0;
#if TRACE
    print("\r\n");
#endif
}


/***********************************************************************
*  ILOAD - ISIS-II LOAD call
***********************************************************************/

void iload(pblk)
LBLK *pblk;
{
    WORD fd;		/* file descriptor */
    BYTE *dataptr;	/* pointer to where data record is to load */
    BYTE *entrypt;	/* entry point */
    WORD bias;		/* copy of pblk->l_bias */
    WORD mode;		/* copy of pblk->l_switch */
    BYTE **enad;	/* copy of pblk->l_enad */
    WORD *stat; 	/* copy of pblk->l_stat */
    WORD count;
    BYTE type,done;
    WORD nchars;

/* copy important parameters from parameter block */

    bias = pblk->l_bias;
    mode = pblk->l_switch;
    enad = pblk->l_enad;
    stat = pblk->l_stat;

/* convert the filename */

    cvtfname(pblk->l_file,fname1,1);

#if TRACE
    print("\r\nLOAD: file=");
    print(fname1);
    print(" bias=");
    phexw(bias);
    print(" switch=");
    phexw(mode);
    print(" enptr=");
    phexw(enad);
    print("\r\nstatptr=");
    phexw(stat);
#endif
    if ((fd = open(skipdrv(fname1),0)) == -1)
    {
	setstat(stat, 13);
	return;
    }
    for (done = FALSE; !done; )
    {
	read(fd,&type,1);
	read(fd,(BYTE *)&count,2);
	switch (type)
	{
	    case 0x02:
	    case 0x08:
	    case 0x0e:
	    case 0x10:
	    case 0x12:
	    case 0x16:
	    case 0x18:
	    case 0x20:	    /* ignored record */
		lseek(fd,(LONG)count,1);
		break;
	    case 0x06:	    /* content record */
		read(fd,&type,1);
		read(fd,(BYTE *)&dataptr,2);
		if (dataptr < (BYTE *)0x3000)
		{
		    setstat(stat, 16);
		    done = TRUE;
		    break;
		}
		read(fd,dataptr+bias,count-4);
		read(fd,&type,1);	/* skip checksum - should check */
		break;
	    case 0x04:	    /* end record */
		read(fd,&type,1);	/* skip the module type */
		read(fd,&type,1);	/* skip the zero byte */
		read(fd,(BYTE *)&entrypt,2);	/* read the entry point */
		done = TRUE;
		if (mode == 0)		/* don't return status if chaining */
		    *stat = 0;
		break;
	    default:	    /* illegal record type */
		setstat(stat, 16);
		done = TRUE;
		break;
	}
    }
    close(fd);
    switch (mode)
    {
	case 0: 	    /* just return to caller */
	    *enad = entrypt;
	    break;
	case 1: 	    /* transfer control to loaded program */
	    savepc = entrypt;
	    savesp = &stak;
	    break;
	default:
	    setstat(stat, 33);     /* illegal system call parameter */
	    return;
    }
#if TRACE
    print(" entry=");
    phexw(entrypt);
    print(" last dataptr=");
    phexw(dataptr);
    print("\r\n");
#endif
}


/***********************************************************************
*  IERROR - ISIS-II ERROR call
***********************************************************************/

void ierror(pblk)
EBLK *pblk;
{
    print("\r\nISIS-II Error ");
    phexw(pblk->e_ernum);
    print(": ");
    switch (pblk->e_ernum)
    {
	case 2:
	    print("AFTN does not specify open file");
	    break;
	case 3:
	    print("Too many open files");
	    break;
	case 7:
	    print("Unable to write to disk");
	    break;
	case 8:
	    print("Attempt to read file open for output");
	    break;
	case 13:
	    print("No such file");
	    break;
	case 14:
	    print("Unable to open output file");
	case 16:
	    print("Illegal load record");
	    break;
	case 19:
	    print("Attempt to seek on non-disk file");
	    break;
	case 20:
	    print("Seek error");
	    break;
	case 21:
	    print("Attempt to rescan non-line-edited file");
	    break;
	case 24:
	    print("PC-DOS file I/O error");
	    break;
	case 27:
	    print("Incorrect MODE parm to SEEK");
	    break;
	case 33:
	    print("Illegal system call parm");
	    break;
	default:
	    print("Huh???");
	    break;
    }
    print("\r\n");
    *pblk->e_stat = 0;
}


/***********************************************************************
*  ISPATH - ISIS-II SPATH call
***********************************************************************/

void ispath(pblk)
SPBLK *pblk;
{
    BYTE *buf;
    BYTE *fname;

    cvtfname(pblk->sp_file,fname1,0); /* convert the filename */
#if TRACE
    print("SPATH: file=");
    print(fname1);
    print(" buf=");
    phexw(pblk->sp_buf);
    print(" statptr=");
    phexw(pblk->sp_stat);
    print("\r\n");
#endif
    buf = pblk->sp_buf; 	    /* point to first byte in buffer */
    fname = fname1;		    /* point to converted filename */
    if (strcmp(fname,":CI") == 0)
    {
	buf[0] = 8;		    /* device number 8 for crt input */
	buf[10] = 0;		    /* device type 0 for seq input */
    }
    else if (strcmp(fname,":CO:") == 0)
    {
	buf[0] = 9;		    /* device number 9 for crt output */
	buf[10] = 1;		    /* device type 0 for seq output */
    }
    else				/* must be a disk file */
    {
	buf[0] = 0;			/* device number 0 for drive 0 */
	buf[10] = 3;		    	/* device type 3 for random access */
	buf[11] = 4;		    	/* drive type 4 is hard disk */
	if (fname[0] == ':' && fname[3] == ':')
	{
	    if (fname[1] == 'F' || fname[1] == 'f')
		buf[0] = fname[2] - '0';	/* use actual drive no. */
	    fname += 4;			/* skip over device name */
	}
    }
    buf++;			    /* pointer to filename part of buffer */
    setb(0,buf,9);		    /* zero fill filename portion of buffer */
    while (*fname && *fname != '.') /* copy the filename into buffer */
	*buf++ = *fname++;
    if (*fname) fname++;	    /* skip past the '.' */
    buf = pblk->sp_buf + 7;	    /* point to extension part of buffer */
    while (*fname)		    /* copy the extension into the buffer */
	*buf++ = *fname++;
    *pblk->sp_stat = 0; 	    /* return good status */
}


/***********************************************************************
*   PRINT - print a null terminated string on the console
***********************************************************************/

print(s)
BYTE *s;
{
    write(1,s,strlen(s));
}


/***********************************************************************
*   STRLEN - return the length of a null-terminated string
***********************************************************************/

strlen(s)
BYTE *s;
{
    WORD count;

    count = 0;
    while (*s++) count++;
    return (count);
}


/***********************************************************************
*   STRCMP - compare two strings
***********************************************************************/

strcmp(s,t) /* return < 0 if s<t, 0 if s==t, >0 if s>t */
BYTE *s,*t;
{
    for ( ; *s == *t; s++,t++)
	if (*s == 0) return (0);
    return (*s - *t);
}

/***********************************************************************
*   STRCPY - copy one string to another
***********************************************************************/

strcpy(d,s)	/* copy s to d */
BYTE *d,*s;
{
    while ((*d++ = *s++) != 0)
	;
}

/***********************************************************************
*   STRCAT - concatenate two strings
***********************************************************************/

strcat(d,s) /* concatenate s to the end of d */
BYTE *d,*s;
{
    strcpy(d+strlen(d),s);
}



/***********************************************************************
* ISIS - handle RETEM (return from 8080 emulation) (ISIS or MONITOR call)
*
* This routine is called from run80 or sim80 after the 8080 program
* returns from emulation with a RETEM.  This happens when the
* the RETEM instruction at 38H is executed as a result of a RST 7
* instruction.  There are RST 7 instructions at the ISIS entry
* point at 40H, as well as all MDS monitor entry points at F8xxH.
***********************************************************************/

void isis()
{
    BYTE opcode;

    if ((WORD)savepc != 0x41)			/* not an ISIS-II call?	*/
    {
	monitor();			/* must be an MDS monitor call */
	return;
    }
    switch(savebc & 0xff)   /* it's an ISIS-II trap - do the right thing */
    {
	case 0:
	    iopen((OBLK *)savede);	/* OPEN call */
	    break;
	case 1:
	    iclose((CBLK *)savede);	/* CLOSE call */
	    break;
	case 2:
	    idelete((DBLK *)savede);	/* DELETE call */
	    break;
	case 3:
	    iread((RBLK *)savede);	/* READ call */
	    break;
	case 4:
	    iwrite((WBLK *)savede);	/* WRITE call */
	    break;
	case 5:
	    iseek((SBLK *)savede);	/* SEEK call */
	    break;
	case 6:
	    iload((LBLK *)savede);	/* LOAD call */
	    break;
	case 7:
	    irename((RNBLK *)savede);	/* RENAME call */
	    break;
	case 9:
	    cleanup();
	    break;
	case 11:
	    irescan((RSBLK *)savede);	/* RESCAN call */
	    break;
	case 12:		/* ERROR call */
	    ierror((EBLK *)savede);
	    break;
	case 14:		/* SPATH call */
	    ispath((SPBLK *)savede);
	    break;
	default:
	    print("\r\nIllegal ISIS-II function call ");
	    phexw(savebc);
	    print("\r\n");
	    break;
    }
}

/***********************************************************************
*  INITISIS - initialize the first two FCBs for :CI: and :CO:,
*     clear the rest of the FCBs
***********************************************************************/

initisis()
{
    FCB *f;
    WORD aft;
    LBLK pblk;
    WORD stat;
    WORD count;
    BYTE *buf;
    BYTE *env;
    WORD comport;

/* Figures out whether COM1 or COM2 is to be used as the alternate
 * console device by looking at the environment variable :I1: or
 * :O1:.  Call sioinit() with the proper port number (0 for com1,
 * 1 for com2) as parameter.  Also set up a control-C handler
 * to clean up if control-C is hit.
 */
    comport = 0;			/* default com port is COM1: */
    if ((env = getenv(":I1:")) == 0)
	env  = getenv(":O1:");
    if (env != 0)
    {
	if (strcmp(env,"COM1:") == 0 || strcmp(env,"com1:") == 0)
	    comport = 0;
	else
	    if (strcmp(env,"COM2:") == 0 || strcmp(env,"com2:") == 0)
		comport = 1;
    }
    sioinit(comport);
    ctrlc(cleanup);

    /* Look at the enviroment variable O1DELAY to find out how many
     * milliseconds to delay after each character.  If not defined,
     * use zero.
     */
    o1delay = 0;
    if ((env = getenv("O1DELAY")) != 0)
    {
	for (; *env; env++)
	    o1delay = (o1delay * 10) + *env - '0';
    }

/* set up FCB 0 for console output file */

    f = &fcbs[0];	/* pointer to FCB for :CO: */
    f->f_fd = 1;	/* DOS file handle = 1 */
    f->f_access = 2;	/* write only */
    f->f_led = 0;	/* not line-edited */

/* set up FCB 1 for console input file */

    f = &fcbs[1];	/* point to FCB for :CO: */
    f->f_fd = 0;	/* DOS file handle = 0 */
    f->f_access = 1;	/* read only */
    f->f_led = 0xff00;	/* it's line-edited */
    count = psp[128];	/* number of bytes in command tail */
    buf = &psp[129];	/* address of command tail */
    while (count && (*buf == ' '))    /* skip leading spaces */
    {
	count--;
	buf++;
    }
    movb(buf,f->f_lbuf,count);
    movb("\r\n",&f->f_lbuf[count],2);
    f->f_count = count + 2;
    f->f_index = 0;

/* mark remaining FCBs as unused */

    for (aft = 2; aft < 10; aft++)
	fcbs[aft].f_fd = -1;	/* mark it as unused */

/* skip past leading spaces on command line to find program name */

    while (f->f_lbuf[f->f_index] == ' ')
	f->f_index++;
    pblk.l_file = &f->f_lbuf[f->f_index];

/* skip read pointer past filename to command to tail */

    while (f->f_lbuf[f->f_index] != ' ' && f->f_lbuf[f->f_index] != '\r')
	f->f_index++;

/* load program specified on command line */

    pblk.l_bias = 0;
    pblk.l_switch = 0;
    pblk.l_enad = &savepc;
    pblk.l_stat = &stat;
    iload(&pblk);
    if (stat != 0)
    {
	print("\r\Missing or non-existent ISIS-II program specified\r\n");
	cleanup();
    }
}

