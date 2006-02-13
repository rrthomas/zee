## Process this file with automake to produce Makefile.in

info_TEXINFOS = zee.texi

zeedir = $(datadir)/$(PACKAGE_NAME)
zee_DATA =					\
	AUTODOC					\
	FAQ					\
	dot$(PACKAGE_NAME).sample

EXTRA_DIST =					\
	FAQ					\
	dot$(PACKAGE_NAME).sample		\
	lisp.txt				\
	mkdoc.c					\
	config.h

AUTODOCSRCS = $(top_srcdir)/src/*.c
AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/src

GENERATED_TEXI = dot$(PACKAGE_NAME).texi $(PACKAGE_NAME)_funcs.texi $(PACKAGE_NAME)_vars.texi $(PACKAGE_NAME)_opts.texi

mkdoc$(EXEEXT_FOR_BUILD): mkdoc.o
	$(CC_FOR_BUILD) $(CFLAGS) $(LIBS) $(LDFLAGS) -o $@ mkdoc.o

mkdoc.o: mkdoc.c config.h $(top_srcdir)/src/tbl_vars.h $(top_srcdir)/src/tbl_opts.h \
        $(top_srcdir)/src/zmalloc.c $(top_srcdir)/src/astr.c
	$(CC_FOR_BUILD) -c -I$(top_srcdir)/doc -I$(top_srcdir) -I$(top_srcdir)/src $(CFLAGS) $(srcdir)/mkdoc.c

$(PACKAGE_NAME).info $(PACKAGE_NAME).dvi: $(GENERATED_TEXI)

$(PACKAGE_NAME)_funcs.texi $(PACKAGE_NAME)_vars.texi AUTODOC: $(AUTODOCSRCS) mkdoc$(EXEEXT_FOR_BUILD)
	./mkdoc$(EXEEXT_FOR_BUILD) $(AUTODOCSRCS) >AUTODOC && mv tbl_funcs.h $(top_builddir)/src/ && mv optstring.h $(top_builddir)/src/

dot$(PACKAGE_NAME).texi: dot$(PACKAGE_NAME).sample
	@echo "@c Automatically generated file: DO NOT EDIT!" >$@
	@echo "@setfilename dot$(PACKAGE_NAME).info" >>$@
	@echo "@example" >>$@
	expand $? >>$@
	@echo "@end example" >>$@

all-local: AUTODOC

clean-local:
	rm -f mkdoc$(EXEEXT_FOR_BUILD) AUTODOC *.o

distclean-local:
	rm -f $(GENERATED_TEXI)
