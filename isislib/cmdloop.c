#include "globals.h"

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
