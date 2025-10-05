#include "globals.h"

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
