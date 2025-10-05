#include "globals.h"

WORD readcmd(BYTE *buf, WORD maxlen)
{
    WORD count = 0;
    BYTE c;

    while (count < maxlen - 1)
    {
        c = bdos(7, 0);
        if (c == '\r' || c == '\n') {
            bdos(2, '\r');
            bdos(2, '\n');
            buf[count] = '\0';
            return count;
        } else if (c == 8 || c == 127) {
            if (count > 0) {
                count--;
                bdos(2, 8);
                bdos(2, ' ');
                bdos(2, 8);
            }
        } else if (c >= ' ' && c < 127) {
            buf[count++] = c;
            bdos(2, c);
        }
    }
    buf[count] = '\0';
    return count;
}
