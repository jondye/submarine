SHELL=/bin/sh

#Macros
SRCS = main.c misc.c tank.c

CFLAGS  = -g -O2 -funroll-loops -ansi -pedantic -ffast-math -D_SVID_SOURCE -D_BSD_SOURCE -I/usr/X11R6/include -DSHM

XLIBS = -L/usr/X11/lib -L/usr/X11R6/lib -lX11 -lXext -lXmu -lXt -lXi -lSM -lICE

GL_LIBS = -lglut -lGLU -lGL -lm $(XLIBS) 

#Rules
default: can-28420
	./can-28420

can-28420: Makefile main.c
	gcc -o can-28420 $(CFLAGS) main.c $(GL_LIBS)

clean:
	rm -f core can-28420

