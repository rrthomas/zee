## Process this file with automake to produce Makefile.in

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
	lisp.txt				\
	mkdoc.c					\
	config.h

man_MANS = zee.1

AUTODOCSRCS = $(top_srcdir)/src/*.c
AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/src

GENERATED_TEXI = dotzee.texi zee_funcs.texi zee_vars.texi

mkdoc$(EXEEXT_FOR_BUILD): mkdoc.o
	$(CC_FOR_BUILD) $(CFLAGS) $(LDFLAGS) -o $@ mkdoc.o

mkdoc.o: mkdoc.c config.h $(top_srcdir)/src/tbl_funcs.h $(top_srcdir)/src/tbl_vars.h \
         $(top_srcdir)/src/strrstr.c $(top_srcdir)/src/vasprintf.c $(top_srcdir)/src/zmalloc.c $(top_srcdir)/src/astr.c

	$(CC_FOR_BUILD) -c -I$(top_srcdir)/doc -I$(top_srcdir) -I$(top_srcdir)/src $(CFLAGS) $(srcdir)/mkdoc.c

zee.texi: $(GENERATED_TEXI)

zee_funcs.texi zee_vars.texi AUTODOC: $(AUTODOCSRCS) mkdoc
	./mkdoc $(AUTODOCSRCS) >AUTODOC

dotzee.texi: dotzee.sample
	@echo "@c Automatically generated file: DO NOT EDIT!" >$@
	@echo "@setfilename dotzee.info" >>$@
	@echo "@example" >>$@
	expand $< >>$@
	@echo "@end example" >>$@

all-local: AUTODOC

clean-local:
	rm -f mkdoc$(EXEEXT_FOR_BUILD) AUTODOC *.o
	rm -f zee.1

distclean-local:
	rm -f $(GENERATED_TEXI)
