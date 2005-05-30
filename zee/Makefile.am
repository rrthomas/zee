## Process this file with automake to produce Makefile.in

SUBDIRS = doc src

EXTRA_DIST =					\
	AUTHORS					\
	COPYING					\
	INSTALL					\
	NEWS					\
	README					\
	THANKS					\
	$(PACKAGE_NAME).pkg

CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src

dist-epoc:
	emxsis $(PACKAGE_NAME).pkg
	cp README Readme.txt
	zip $(PACKAGE_NAME).zip $(PACKAGE_NAME).sis Readme.txt
	rm Readme.txt
