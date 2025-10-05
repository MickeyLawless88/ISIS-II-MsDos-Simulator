#include "globals.h"

cleanup()
{
    siouninit();
    closeallfiles();
    print("\r\n[Program terminated]\r\n");
    longjmp(cmdloop_env, 1);
}
