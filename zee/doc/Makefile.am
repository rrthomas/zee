## Process this file with automake to produce Makefile.in
## $Id$

info_TEXINFOS = zee.texi

zeedir = $(datadir)/zee
zee_DATA =					\
	AUTODOC					\
	FAQ					\
	TUTORIAL				\
	dotzee.sample

EXTRA_DIST =					\
	FAQ					\
	TUTORIAL				\
	dotzee.sample				\
	dotzee.texi				\
	lisp.txt				\
	zlint					\
	zee.1.in				\
	mkdoc.c					\
	config.h

man_MANS = zee.1

AUTODOCSRCS = $(top_srcdir)/src/*.c
AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/src

mkdoc$(EXEEXT_FOR_BUILD): mkdoc.o
	$(CC_FOR_BUILD) $(CFLAGS) $(LDFLAGS) -o $@ mkdoc.o

mkdoc.o: mkdoc.c config.h $(top_srcdir)/src/tbl_funcs.h $(top_srcdir)/src/tbl_vars.h \
         $(top_srcdir)/src/strrstr.c $(top_srcdir)/src/vasprintf.c $(top_srcdir)/src/zmalloc.c $(top_srcdir)/src/astr.c

	$(CC_FOR_BUILD) -c -I$(top_srcdir)/doc -I$(top_srcdir) -I$(top_srcdir)/src $(CFLAGS) $(srcdir)/mkdoc.c

AUTODOC: $(AUTODOCSRCS) mkdoc
	./mkdoc $(AUTODOCSRCS) >AUTODOC

zee.info: dotzee.texi

dotzee.texi: dotzee.sample
	@echo "@c Automatically generated file: NO DOT EDIT!" >$@
	@echo "@setfilename dotzee.info" >>$@
	@echo "@example" >>$@
	expand $< >>$@
	@echo "@end example" >>$@

zee.1: zee.1.in
	rm -f $@
	sed s+'@'pkgdatadir@+$(pkgdatadir)+g $< > $@

all-local: HELP AUTODOC

clean-local:
	rm -f mkdoc$(EXEEXT_FOR_BUILD) AUTODOC HELP *.o
	rm -f zee.1
