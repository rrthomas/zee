## Process this file with automake to produce Makefile.in

SUBDIRS = doc src

EXTRA_DIST =					\
	AUTHORS					\
	COPYING					\
	INSTALL					\
	NEWS					\
	README					\
	THANKS					\
	zee.pkg

CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src

dist-epoc:
	emxsis zee.pkg
	cp README Readme.txt
	zip zee.zip zee.sis Readme.txt
	rm Readme.txt
