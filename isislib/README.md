# ISISLIB — Developer Reference & Annotated Function Guide

This document provides detailed technical documentation for each function in the modular ISIS-II simulator library,
including annotated source excerpts and explanations of syntax and function behavior.

---

## 📁 globals.h — Shared Header

Defines shared types, externs, and includes used by all modules.

```c
typedef unsigned char  BYTE;   /* 8-bit unsigned value */
typedef unsigned short WORD;   /* 16-bit unsigned value */

#include <setjmp.h>

extern jmp_buf cmdloop_env;    /* Jump buffer for returning to cmdloop() */

extern void print(BYTE *s);
extern WORD readcmd(BYTE *buf, WORD maxlen);
extern BYTE *parsecmd(BYTE *cmdline, BYTE **argstart);
extern void setupcmdtail(BYTE *progname, BYTE *args);
extern WORD loadprog(BYTE *progname);
extern void closeallfiles(void);
extern void cmdloop(void);
extern void cleanup(void);
```

### Explanation
This defines 8-bit and 16-bit data types used by the emulator to mimic the 8080 processor’s data width.
The header declares prototypes for all global functions, ensuring consistent linkage between source files.

---

## 🖥️ readcmd.c — Console Command Reader

### Purpose
Reads one command line from the user with echo and editing support.

```c
WORD readcmd(BYTE *buf, WORD maxlen)
{
    WORD count = 0;    /* number of characters read */
    BYTE c;

    while (count < maxlen - 1)
    {
        c = bdos(7, 0);       /* read character without echo */

        if (c == '\r' || c == '\n') {  /* user pressed Enter */
            bdos(2, '\r');             /* echo CR */
            bdos(2, '\n');             /* echo LF */
            buf[count] = '\0';         /* terminate string */
            return count;
        }
        else if (c == 8 || c == 127) {  /* backspace or DEL */
            if (count > 0) {
                count--;
                bdos(2, 8);            /* move cursor left */
                bdos(2, ' ');          /* erase char */
                bdos(2, 8);            /* move cursor back */
            }
        }
        else if (c >= ' ' && c < 127) { /* printable ASCII */
            buf[count++] = c;
            bdos(2, c);               /* echo char */
        }
    }

    buf[count] = '\0';
    return count;
}
```

### Explanation
This function uses BDOS calls to implement character-by-character input with basic editing.
It behaves similarly to the ISIS console input handler, returning a null-terminated string.

---

## 🧩 parsecmd.c — Command Line Parser

### Purpose
Extracts the program name and its arguments from a command line string.

```c
BYTE *parsecmd(BYTE *cmdline, BYTE **argstart)
{
    BYTE *progname;
    BYTE *p = cmdline;

    while (*p == ' ' || *p == '\t') p++;  /* skip whitespace */

    if (*p == '\0') {                     /* empty input */
        *argstart = (BYTE *)0;
        return (BYTE *)0;
    }

    progname = p;                          /* start of program name */
    while (*p && *p != ' ' && *p != '\t') p++;

    if (*p == '\0') {
        *argstart = p;
    } else {
        *p = '\0';                        /* terminate name */
        p++;
        while (*p == ' ' || *p == '\t') p++;
        *argstart = p;
    }

    return progname;
}
```

### Explanation
This modifies the buffer directly, inserting null terminators to separate fields.
`progname` points to the executable, and `*argstart` to its parameters.

---

## ⚙️ setupcmdtail.c — Command Tail and FCB Builder

### Purpose
Builds the memory environment (command tail, FCB, and input buffer) expected by ISIS-II programs.

```c
void setupcmdtail(BYTE *progname, BYTE *args)
{
    FCB *f = &fcbs[1];
    BYTE *cmdtail = (BYTE *)0x80;
    BYTE *fcb = (BYTE *)0x5C;
    BYTE fullcmd[128];
    BYTE fname[9], fext[4];
    BYTE *p;
    int i;
    WORD count;

    strcpy(fullcmd, progname);
    if (args && *args) {
        strcat(fullcmd, " ");
        strcat(fullcmd, args);
    }

    for (i = 0; fullcmd[i]; i++)
        if (fullcmd[i] >= 'a' && fullcmd[i] <= 'z')
            fullcmd[i] -= ('a' - 'A');

    setb(0, fcb, 36);

    p = args;
    while (p && (*p == ' ' || *p == '\t')) p++;

    for (i = 0; i < 8 && *p && *p != '.' && *p != ' ' && *p != '\t'; i++, p++)
        fname[i] = (*p >= 'a' && *p <= 'z') ? (*p - 'a' + 'A') : *p;
    while (i < 8) fname[i++] = ' ';

    if (*p == '.') {
        p++;
        for (i = 0; i < 3 && *p && *p != ' ' && *p != '\t'; i++, p++)
            fext[i] = (*p >= 'a' && *p <= 'z') ? (*p - 'a' + 'A') : *p;
    }
    while (i < 3) fext[i++] = ' ';

    fcb[0] = 0;
    movb(fname, &fcb[1], 8);
    movb(fext, &fcb[9], 3);

    count = strlen(fullcmd);
    cmdtail[0] = count + 1;
    cmdtail[1] = ' ';
    movb(fullcmd, &cmdtail[2], count);
    cmdtail[count + 2] = '\r';

    movb(fullcmd, f->f_lbuf, count);
    movb("\r\n", &f->f_lbuf[count], 2);
    f->f_count = count + 2;
    f->f_index = 0;
}
```

### Explanation
Simulates how ISIS constructs memory at 0x80 (command tail) and 0x5C (FCB).  
`f_lbuf` is used for programs reading from `:CI:`.  
All names are uppercased to match ISIS conventions.

---

## 💾 loadprog.c — Program Loader

```c
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
```

### Explanation
This wraps the `iload()` function, setting up the ISIS load block.
After successful load, the entry address is written to `savepc`, and status returned.

---

## 🧹 closeallfiles.c — File Cleanup

```c
void closeallfiles()
{
    WORD aft;
    for (aft = 2; aft < NFCBS; aft++) {
        if (fcbs[aft].f_fd != -1) {
            close(fcbs[aft].f_fd);
            fcbs[aft].f_fd = -1;
        }
    }
}
```

### Explanation
Closes all user file descriptors (beyond `:CI:` and `:CO:`) to reset state.

---

## 🔁 cleanup.c — Controlled Program Exit

```c
cleanup()
{
    siouninit();
    closeallfiles();
    print("\r\n[Program terminated]\r\n");
    longjmp(cmdloop_env, 1);
}
```

### Explanation
Shuts down I/O and jumps back to the command loop.  
`longjmp()` restores the stack frame saved by `setjmp()` in `cmdloop()`, returning control safely.

---

## 💻 cmdloop.c — Main Emulator Loop

```c
cmdloop()
{
    BYTE cmdline[128];
    BYTE *progname;
    BYTE *args;
    WORD stat;

    print("\r\nISIS-II Command Line Emulator\r\n");
    print("Type EXIT to quit\r\n\r\n");

    if (setjmp(cmdloop_env)) {
        print("\r\n");
    }

    while (1)
    {
        print("=");

        if (readcmd(cmdline, sizeof(cmdline)) == 0)
            continue;

        progname = parsecmd(cmdline, &args);
        if (progname == (BYTE *)0)
            continue;

        if (strcmp(progname, "EXIT") == 0 || strcmp(progname, "exit") == 0)
        {
            print("\r\n");
            break;
        }

        setupcmdtail(progname, args);
        stat = loadprog(progname);

        if (stat != 0)
        {
            print("\r\nCannot load program: ");
            print(progname);
            print("\r\n");
            continue;
        }

        go8080();
        closeallfiles();
        print("\r\n");
    }
}
```

### Explanation
Implements the entire interactive lifecycle of an ISIS-II session:  
Input → Parse → Load → Execute → Return to prompt.

---

## 🧠 Execution Summary

| Step | Function | Purpose |
|------|-----------|----------|
| 1 | `cmdloop()` | Show prompt and read command |
| 2 | `readcmd()` | Collect user input |
| 3 | `parsecmd()` | Extract program and args |
| 4 | `setupcmdtail()` | Build command environment |
| 5 | `loadprog()` | Load binary into memory |
| 6 | `go8080()` | Run emulated program |
| 7 | `cleanup()` | Return to command loop |

---
