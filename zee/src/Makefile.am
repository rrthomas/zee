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

zee_nondoc_SOURCES =			        \
	rblist.c			        \
	rblist.h			        \
	nonstd.h			        \
	vector.c			        \
	vector.h			        \
	list.c				        \
	list.h				        \
	astr.c				        \
	astr.h				        \
	zmalloc.h			        \
	zmalloc.c			        \
	term.h				        \
	extern.h			        \
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

# FIXME: The generated files are all added below so as not to need to
# run Lua scripts in make checkdist (the Lua script rules don't work
# in VPATH builds, and for some reason aren't even called). Really, we
# should make VPATH builds work, but for now I give up.
EXTRA_DIST =  								\
		tbl_funcs.h funcs.texi 					\
		vars.texi						\
		opts.texi						\
		default_bindings.c					\
		lib.lua							\
		mkfuncs.lua mkvars.lua mkopts.lua mkbindings.lua	\
		cua_bindings emacs_bindings

$(PACKAGE).info $(PACKAGE).dvi: $(GENERATED_TEXI)

basic.o: tbl_funcs.h

tbl_funcs.h funcs.texi: $(zee_doc_SOURCES) mkfuncs.lua
	LUA_INIT= LUA_PATH="$(top_srcdir)/src/?.lua" $(LUA) -e 'PACKAGE="$(PACKAGE)"' mkfuncs.lua $(zee_doc_SOURCES)

$(PACKAGE)rc vars.texi: tbl_vars.h mkvars.lua
	LUA_INIT= LUA_PATH="$(top_srcdir)/src/?.lua" $(LUA) -e 'PACKAGE="$(PACKAGE)"' mkvars.lua

opts.texi: tbl_opts.h mkopts.lua
	LUA_INIT= LUA_PATH="$(top_srcdir)/src/?.lua" $(LUA) -e 'PACKAGE="$(PACKAGE)"' mkopts.lua

default_bindings.c: mkbindings.lua cua_bindings
	LUA_INIT= LUA_PATH="$(top_srcdir)/src/?.lua" $(LUA) -e 'PACKAGE="$(PACKAGE)"' mkbindings.lua < cua_bindings

bind.c: default_bindings.c

distclean-local:
	rm -f $(GENERATED_TEXI) tbl_funcs.h $(PACKAGE)rc

sloccount:
	sloccount $(zee_base_SOURCES) term_*.c *.lua
