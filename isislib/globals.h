#ifndef GLOBALS_H
#define GLOBALS_H

typedef unsigned char  BYTE;
typedef unsigned short WORD;

#include <setjmp.h>

extern jmp_buf cmdloop_env;

extern void print(BYTE *s);
extern WORD readcmd(BYTE *buf, WORD maxlen);
extern BYTE *parsecmd(BYTE *cmdline, BYTE **argstart);
extern void setupcmdtail(BYTE *progname, BYTE *args);
extern WORD loadprog(BYTE *progname);
extern void closeallfiles(void);
extern void cmdloop(void);
extern void cleanup(void);

#endif
