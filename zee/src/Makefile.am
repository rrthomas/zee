## Process this file with automake to produce Makefile.in

TESTS = rblist rbacc
check_PROGRAMS = rblist rbacc

TESTCPPFLAGS = -DTEST -DSRCPATH="\"$(top_srcdir)/src/\""

rblist_CPPFLAGS = $(TESTCPPFLAGS)
rblist_LDADD = zmalloc.o rbacc.o

rbacc_CPPFLAGS = $(TESTCPPFLAGS)
rbacc_LDADD = zmalloc.o rblist.o

bin_PROGRAMS = zee

dist_pkgdata_DATA = cua_bindings emacs_bindings beep.wav

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
	term.c					\
	undo.c					\
	variables.c

zee_nondoc_SOURCES =			        \
	rblist.c			        \
	rblist.h			        \
	rbacc.c			        	\
	rbacc.h					\
        rbutil.c				\
        rbutil.h				\
	nonstd.h			        \
	list.c				        \
	list.h				        \
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

EXTRA_DIST =  								\
		lib.lua							\
		mkfuncs.lua mkvars.lua mkopts.lua mkbindings.lua	\
		cua_bindings emacs_bindings

BUILT_TEXI = funcs.texi vars.texi opts.texi

BUILT_SOURCES =								\
		tbl_funcs.h 						\
		default_bindings.c					\
		$(PACKAGE)rc						\
		$(BUILT_TEXI)

CLEANFILES = $(BUILT_SOURCES)

$(PACKAGE).info $(PACKAGE).dvi: $(BUILT_TEXI)

tbl_funcs.h funcs.texi: $(zee_doc_SOURCES) mkfuncs.lua
	LUA_INIT= LUA_PATH="$(top_srcdir)/src/?.lua" $(LUA) -e 'PACKAGE="$(PACKAGE)"' $(top_srcdir)/src/mkfuncs.lua $(top_srcdir)/src $(zee_doc_SOURCES)

$(PACKAGE)rc vars.texi: tbl_vars.h mkvars.lua
	LUA_INIT= LUA_PATH="$(top_srcdir)/src/?.lua" $(LUA) -e 'PACKAGE="$(PACKAGE)"' $(top_srcdir)/src/mkvars.lua $(top_srcdir)/src/tbl_vars.h

opts.texi: tbl_opts.h mkopts.lua
	LUA_INIT= LUA_PATH="$(top_srcdir)/src/?.lua" $(LUA) -e 'PACKAGE="$(PACKAGE)"' $(top_srcdir)/src/mkopts.lua $(top_srcdir)/src/tbl_opts.h

default_bindings.c: mkbindings.lua cua_bindings
	LUA_INIT= LUA_PATH="$(top_srcdir)/src/?.lua" $(LUA) -e 'PACKAGE="$(PACKAGE)"' $(top_srcdir)/src/mkbindings.lua < $(top_srcdir)/src/cua_bindings

bind.c: default_bindings.c

loc:
	cloc $(zee_base_SOURCES) term_*.c *.lua
