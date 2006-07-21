## Process this file with automake to produce Makefile.in

info_TEXINFOS = zee.texi

dist_sysconf_DATA = $(PACKAGE)rc

dist_pkgdata_DATA =				\
	FAQ

EXTRA_DIST =					\
	FAQ					\
	mkdoc.c

DOCSRCS = $(top_srcdir)/src/*.c
AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/src

GENERATED_TEXI = $(PACKAGE)_funcs.texi $(PACKAGE)_vars.texi $(PACKAGE)_opts.texi

mkdoc$(EXEEXT_FOR_BUILD): mkdoc.o
	$(CC_FOR_BUILD) $(CFLAGS) $(LIBS) $(LDFLAGS) -o $@ mkdoc.o

mkdoc.o: mkdoc.c $(top_srcdir)/src/tbl_vars.h $(top_srcdir)/src/tbl_opts.h \
        $(top_srcdir)/src/zmalloc.c $(top_srcdir)/src/astr.c
	$(CC_FOR_BUILD) -c -I$(top_srcdir)/doc -I$(top_srcdir) -I$(top_srcdir)/src $(CFLAGS) $(srcdir)/mkdoc.c

$(PACKAGE).info $(PACKAGE).dvi: $(GENERATED_TEXI)

$(PACKAGE)rc $(PACKAGE)_funcs.texi $(PACKAGE)_vars.texi $(PACKAGE)_opts.texi: $(DOCSRCS) mkdoc$(EXEEXT_FOR_BUILD)
	./mkdoc$(EXEEXT_FOR_BUILD) $(DOCSRCS) > $(top_builddir)/src/tbl_funcs.h

clean-local:
	rm -f mkdoc$(EXEEXT_FOR_BUILD) *.o

distclean-local:
	rm -f $(GENERATED_TEXI)
