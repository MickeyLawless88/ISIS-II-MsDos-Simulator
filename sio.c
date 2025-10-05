/* sio.c - routines to talk to serial I/O ports, using serial input interrupts.
 */

/*----------------------------------------------------------------------
 *  Literal definitions
 */

#define	TRUE	1
#define FALSE	0

#define BUFSIZE	2048	/* size of serial input buffer	*/

/* literals for address of 8250 ports relative to base address */

#define DLL	0	/* divisor latch (least significant byte) */
#define DLM	1	/* divisor latch (most significant byte) */
#define IER	1	/* interrupt enable register */
#define IIR	2	/* interrupt identification register */
#define LCR	3	/* line control register */
#define MCR	4	/* modem control register */
#define LSR	5	/* line status register */
#define MSR	6	/* modem status register */

#define RXBUFFER	0		/* receive buffer register */
#define TXBUFFER	0		/* transmit buffer register */


/* Definitions for various bits in 8250 Status ports */

#define DLAB	0x80		/* LCR - enable divisor latch */
#define OUT2	8		/* MCR - enable interrupts */
#define DTR 	1		/* MCR - data terminal ready */
#define CTS 	0x10		/* MSR - clear to send */
#define DSR 	0x20		/* MSR - data set ready */
#define RI  	0x40		/* MSR - ring indicator */
#define RLSD	0x80		/* MSR - carrier detect */
#define THRE	0x20		/* LSR - transmitter hold. reg. empty */
#define TSRE	0x40		/* LSR - transmitter shift reg. empty */
#define DATAREADY	1	/* LSR - receive data ready */

/*----------------------------------------------------------------------
 *  External assembly language functions
 */

extern int
    siosetvec(),			/* set serial interrupt vector	*/
    input(),				/* input from port		*/
    output(),				/* output to a port		*/
    enable(),				/* enable interrupts		*/
    disable();				/* disable interrupts		*/

/*----------------------------------------------------------------------
 *  VARIABLES
 */

/*  Global variable that says how many milliseconds to delay
 *  after outputting a character to serial port.
 */
int o1delay = 0;

/*  Local variables */

int comline;		/* 0 if COM1, 1 if COM2 */

/*  Base address of UART, indexed by value in 'comline' variable */

static int siobase[2] = {0x3f8, 0x2f8};

/*----------------------------------------------------------------------
 *  Serial I/O routines
 */

/*----------------------------------------------------------------------
 * SIOSETBITS - turn specific bits of UART register on
 */
siosetbits (port,bits)
int	port;		/* UART port address relative to base	*/
char	bits;		/* mask of bits to be set to 1		*/
{
    port += siobase[comline];
    output(port,input(port) | bits);
}

/*----------------------------------------------------------------------
 * SIOCLRBITS - turn specific bits of UART register off
 */
sioclrbits (port,bits)
int	port;		/* UART port address relative to base	*/
char	bits;		/* mask of bits to be set to 0		*/
{
    port += siobase[comline];
    output(port,input(port) & ~bits);
}

/*----------------------------------------------------------------------
 * SIOWRCHAR - write a character to the serial line
 */
siowrchar (c)
char c;			/* character to write to serial line */
{
    int port;

    port = siobase[comline];
    while ((input(port+LSR) & THRE) == 0)
	;
    if (o1delay)
    {
	while ((input(port+LSR) & TSRE) == 0)
	    ;				/* wait for last character to go out */
	milliwait(o1delay);		/* n milliseconds delay */
    }
    output(port+TXBUFFER, c);
}

/*----------------------------------------------------------------------
 * milliwait - delay n milliseconds on a V20
 *
 * this should really be redone to be accurate on all machines,
 * but that's a lot harder.
 */
milliwait(n)
int n;		/* no. of milliseconds on a V20 */
{
    int i;

    while (n--)
	for (i = 0; i < 110; i++)
		;
}

/*----------------------------------------------------------------------
 * SIORDCHAR - read a character from the serial line
 */
siordchar()
{
    char c;

    while (getbuf(&c) == FALSE)
	;
    return (c & 0xff);
}

/*----------------------------------------------------------------------
 * SIOCHKCHAR - return TRUE if a character is available on serial line
 */
siochkchar()
{
    return (chkbuf());
}

/*----------------------------------------------------------------------
 * SIOINIT - initialize things for reading/writing to a serial line
 * port parameter:	0 - COM1
 *			1 - COM2
 */
sioinit(line)
int line;
{
    comline = line;
    clearbuf();
    enableint();
}

/*----------------------------------------------------------------------
 * SIOUNINIT - disable interrupts
 */
siouninit()
{
     disableint();
}

/*----------------------------------------------------------------------
 *  Buffer management routines
 */

/* buffer structure -- could be an array of structures
 * if you wanted to accept input from more than one serial
 * input line at a time.
 */

struct
{
    int empty;			/* true if buffer is empty	*/
    int readindex;		/* read pointer			*/
    int writeindex;		/* write pointer		*/
    char bufchar [BUFSIZE];
} buf;

/*----------------------------------------------------------------------
 * CLEARBUF - clear the specified character buffer
 */

clearbuf()
{
    disable();
    buf.readindex = buf.writeindex = 0;
    buf.empty = TRUE;
    enable();
}

/*----------------------------------------------------------------------
 * NEXTINDEX - return incremented buffer index.
 * This used to be a function, now it's a macro for speed and simplicity.
 */

#define nextindex(i) (i == sizeof(buf.bufchar) ? 0 : i + 1)

/*----------------------------------------------------------------------
 * PUTBUF - put a character into the specified buffer. Interrupts must
 * be disabled before calling this routine.
 */

putbuf(c)
char c;
{
    int nextind;

    nextind = nextindex(buf.writeindex);
    if (nextind == buf.readindex)
	return;
    buf.bufchar[buf.writeindex] = c;
    buf.writeindex = nextind;
    buf.empty = FALSE;
}

/*----------------------------------------------------------------------
 * GETBUF - get a character from the buffer, if available.  False
 *	is returned if no character available, and true otherwise.
 *	The character is stored at the byte pointed by CHARPTR.
 */
getbuf(charptr)
char *charptr;
{
    disable();
    if (buf.empty)
    {
	enable();
	return (FALSE);
    }
    *charptr = buf.bufchar[buf.readindex];
    if ((buf.readindex = nextindex(buf.readindex)) == buf.writeindex)
	buf.empty = TRUE;
    enable();
    return (TRUE);
}

/*----------------------------------------------------------------------
 * CHKBUF - check if a character is available in buffer, don't return it
 *	Return TRUE if buffer is not empty, FALSE otherwise.
 */
chkbuf()
{
    return (!buf.empty);
}

/*----------------------------------------------------------------------
 * SIOINT - interrupt function for primary serial port
 */

sioint()
{
    char intid;

    output(0x20,0x20);			/* end of interrupt to 8259 chip */
    intid = input(siobase[comline]+IIR);    /* get interrupt id register */
    if (intid & 1)			/* no interrupt pending		*/
	return;				/* do nothing			*/
    if ((intid & 6) == 4)		/* was it receiver interrupt?	*/
	    putbuf(input(siobase[comline]+RXBUFFER));
					/* get the character, save it	*/
}

/*----------------------------------------------------------------------
 * ENABLEINT - initialize interrupt registers and vectors
 */

static char mask[2] = {0xef,0xf7};	/* 8259 interrupt masks		*/

enableint()
{
    static int  irq[3] = {4,3};		/* interrupt numbers		*/

    siosetvec(irq[comline]);		/* set IRQ4 or IRQ3 vector	*/
    output(siobase[comline]+IER, 1);	/* data available interrupt	*/
    siosetbits(MCR,OUT2);		/* enable interrupts on Asynch card */
    output(0x21, input(0x21) & mask[comline]);  /* unmask interrupt at 8259 */
}

/*----------------------------------------------------------------------
 * DISABLEINT - disable serial I/O interrupts
 */

disableint()
{
    siosetbits(MCR,OUT2);		/* disable interrupts on card	*/
    output(0x21, input(0x21) | ~mask[comline]);  /* unmask interrupt at 8259 */
}
