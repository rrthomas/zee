## Process this file with automake to produce Makefile.in

TESTS = vector astr rblist
check_PROGRAMS = vector astr rblist

vector_CPPFLAGS = -DTEST -DSRCPATH="\"$(top_srcdir)/src/\""
vector_LDADD = zmalloc.o

astr_CPPFLAGS = -DTEST -DSRCPATH="\"$(top_srcdir)/src/\""
astr_LDADD = zmalloc.o rblist.o

rblist_CPPFLAGS = -DTEST -DSRCPATH="\"$(top_srcdir)/src/\""
rblist_LDADD = zmalloc.o vector.o astr.o

bin_PROGRAMS = zee

dist_pkgdata_DATA = cua_bindings

info_TEXINFOS = zee.texi

dist_sysconf_DATA = $(PACKAGE)rc

EXTRA_DIST = mkdoc.c

zee_base_SOURCES =				\
	rblist.c				\
	rblist.h				\
	nonstd.h				\
	vector.c				\
	vector.h				\
	list.c					\
	list.h					\
	astr.c					\
	astr.h					\
	zmalloc.h				\
	zmalloc.c				\
	basic.c					\
	bind.c					\
	buffer.c				\
	command.c				\
	completion.c				\
	display.c				\
	extern.h				\
	file.c					\
	funcs.c					\
	help.c					\
	history.c				\
	keys.c					\
	killring.c				\
	line.c					\
	macro.c					\
	main.c					\
	minibuf.c				\
	point.c					\
	search.c				\
	tbl_keys.h				\
	tbl_opts.h				\
	tbl_vars.h				\
	undo.c					\
	variables.c				\
	main.h

if ALLEGRO
zee_SOURCES = $(zee_base_SOURCES) term_allegro.c
zee_LDFLAGS = `allegro-config --libs`
else
zee_SOURCES = $(zee_base_SOURCES) term_ncurses.c
endif

zee_CPPFLAGS = -DPKGDATADIR="\"$(pkgdatadir)\"" -DSYSCONFDIR="\"$(sysconfdir)\""
zee_LDADD = @LIBOBJS@

DOCSRCS = $(top_srcdir)/src/*.c

GENERATED_TEXI = funcs.texi vars.texi opts.texi

mkdoc$(EXEEXT_FOR_BUILD): mkdoc.o
	$(CC_FOR_BUILD) $(CFLAGS) $(LIBS) $(LDFLAGS) -o $@ mkdoc.o

mkdoc.o: mkdoc.c $(top_srcdir)/src/tbl_vars.h $(top_srcdir)/src/tbl_opts.h \
        $(top_srcdir)/src/zmalloc.c $(top_srcdir)/src/astr.c
	$(CC_FOR_BUILD) -c -I$(top_builddir) -I$(top_srcdir)/src $(CFLAGS) $(srcdir)/mkdoc.c

$(PACKAGE).info $(PACKAGE).dvi: $(GENERATED_TEXI)

$(PACKAGE)rc funcs.texi vars.texi opts.texi: $(DOCSRCS) mkdoc$(EXEEXT_FOR_BUILD)
	./mkdoc$(EXEEXT_FOR_BUILD) $(DOCSRCS) > $(top_builddir)/src/tbl_funcs.h

clean-local:
	rm -f mkdoc$(EXEEXT_FOR_BUILD) *.o

distclean-local:
	rm -f tbl_funcs.h $(GENERATED_TEXI) $(PACKAGE)rc

sloccount:
	sloccount $(zee_base_SOURCES) term_*.c
