# Makefile for building isis.exe using Borland Turbo C 2 and Turbo Assembler 2.

# Change TCDIR to point to your Turbo C base directory.

TCDIR=c:\profiles\admini~1\tc\tc

#DEBUG = -v
#LINKDEBUG = /v

.c.obj :
	tcc -c $(DEBUG) $*.c

.asm.obj :
	tasm -t -mx $*.asm

COMMONOBJ = isis.obj debug.obj movb.obj sio.obj sioutil.obj ctrlc.obj
COMMONLNK = isis debug movb sio sioutil ctrlc

all : isis.exe

isis.exe : istart.obj sim80.obj $(COMMONOBJ) makefile
	tlink istart sim80 $(COMMONLNK),isis,isis,$(TCDIR)\lib\cs.lib $(LINKDEBUG)

clean:
	del *.obj

test:
	cd ..\test
	setup
	make -fmakefile.dos
