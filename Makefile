# Basic generalized Makefile
# Svend Sorensen <ssorensen@fastmail.fm>
# GNU Coding Standards: Makefile Conventions (section 7.2)
#   http://www.gnu.org/prep/standards_50.html#SEC50

TARGET=cueconvert cuebreakpoints cuerename
prefix=/usr/local

# CC          C compiler command
# CFLAGS      C compiler flags
# LDFLAGS     linker flags, e.g. -L<lib dir> if you have libraries in a
#             nonstandard directory <lib dir>
# CPPFLAGS    C/C++ preprocessor flags, e.g. -I<include dir> if you have
#             headers in a nonstandard directory <include dir>
# CPP         C preprocessor
#
# LIBS        library files, e.g. -l<libname>

CC=gcc
CFLAGS+=-g -Wall
CPPFLAGS+=
LDFLAGS+=
LIBS+=

OBJS=msf.o cue.o cd.o cdtext.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) $(LDFLAGS) $@.c

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

install:
	install -d $(DESTDIR)$(prefix)/bin
	install -m 755 $(TARGET) $(DESTDIR)$(prefix)/bin/

clean:
	rm -f *.o *.a $(TARGET)
