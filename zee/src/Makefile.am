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

zee_doc_SOURCES =				\
	basic.c					\
	bind.c					\
	buffer.c				\
	command.c				\
	completion.c				\
	display.c				\
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
	undo.c					\
	variables.c

zee_nondoc_SOURCES =				\
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
	extern.h				\
	tbl_keys.h				\
	tbl_opts.h				\
	tbl_vars.h				\
	main.h

zee_base_SOURCES = $(zee_doc_SOURCES) $(zee_nondoc_SOURCES)

if ALLEGRO
zee_SOURCES = $(zee_base_SOURCES) term_allegro.c
zee_LDFLAGS = `allegro-config --libs`
else
zee_SOURCES = $(zee_base_SOURCES) term_ncurses.c
endif

zee_CPPFLAGS = -DPKGDATADIR="\"$(pkgdatadir)\"" -DSYSCONFDIR="\"$(sysconfdir)\""
zee_LDADD = @LIBOBJS@

GENERATED_TEXI = funcs.texi vars.texi opts.texi

EXTRA_DIST = mkdoc.c

mkdoc$(EXEEXT_FOR_BUILD): mkdoc.o
	$(CC_FOR_BUILD) $(CFLAGS) $(LIBS) $(LDFLAGS) -o $@ mkdoc.o

mkdoc.o: mkdoc.c tbl_vars.h tbl_opts.h zmalloc.c astr.c vector.c rblist.c
	$(CC_FOR_BUILD) -c -I$(top_builddir) -I$(top_srcdir)/src $(CFLAGS) mkdoc.c

$(PACKAGE).info $(PACKAGE).dvi: $(GENERATED_TEXI)

$(PACKAGE)rc funcs.texi vars.texi opts.texi: $(zee_doc_SRCS) mkdoc$(EXEEXT_FOR_BUILD)
	./mkdoc$(EXEEXT_FOR_BUILD) $(zee_doc_SRCS) > tbl_funcs.h

clean-local:
	rm -f mkdoc$(EXEEXT_FOR_BUILD) *.o

distclean-local:
	rm -f tbl_funcs.h $(GENERATED_TEXI) $(PACKAGE)rc

sloccount:
	sloccount $(zee_base_SOURCES) term_*.c
