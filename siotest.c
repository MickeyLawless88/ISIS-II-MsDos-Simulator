/* siotest.c - test sio routines
 *
 * With Datalight C, compile and link with one command:
 *	dlc -msi siotest.c sio.c sioutil.asm ctrlc.asm
 */
#include <stdio.h>

trap()
{
    printf("\nYikes! A control-c was hit!\n");
    printf("Cleaning up.\n");
    siouninit();
    exit(0);
}

main()
{
    int c;

    sioinit(1);		/* use com2 */
    ctrlc(trap);
    while ((c = siordchar()) != 3)
    {
        printf("Character: 0x%2x\n",c);
	siowrchar(c);
    }
    siouninit();
}
