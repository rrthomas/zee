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

dist_pkgdata_DATA = cua_bindings emacs_bindings

info_TEXINFOS = zee.texi

dist_sysconf_DATA = $(PACKAGE)rc

zee_doc_SOURCES =				\
	tbl_opts.h				\
	tbl_vars.h				\
	tbl_keys.h				\
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

EXTRA_DIST = mkfuncs.lua mkvars.lua mkopts.lua default_bindings.c

$(PACKAGE).info $(PACKAGE).dvi: $(GENERATED_TEXI)

tbl_funcs.h funcs.texi: $(zee_doc_SOURCES) mkfuncs.lua
	$(LUA) -e 'PACKAGE="$(PACKAGE)"' mkfuncs.lua $(zee_doc_SOURCES)

$(PACKAGE)rc vars.texi: tbl_vars.h mkvars.lua
	$(LUA) -e 'PACKAGE="$(PACKAGE)"' mkvars.lua

opts.texi: tbl_opts.h mkopts.lua
	$(LUA) -e 'PACKAGE="$(PACKAGE)"' mkopts.lua

default_bindings.c: mkbindings.lua
	$(LUA) -e 'PACKAGE="$(PACKAGE)"' mkbindings.lua < cua_bindings

distclean-local:
	rm -f $(GENERATED_TEXI) tbl_funcs.h $(PACKAGE)rc

sloccount:
	sloccount $(zee_base_SOURCES) term_*.c *.lua
