## Process this file with automake to produce Makefile.in

TESTS = rblist rbacc
check_PROGRAMS = rblist rbacc

TESTCPPFLAGS = -DTEST -DSRCPATH="\"$(top_srcdir)/src/\""

rblist_CPPFLAGS = $(TESTCPPFLAGS)
rblist_LDADD = zmalloc.o rbacc.o rbutil.o

rbacc_CPPFLAGS = $(TESTCPPFLAGS)
rbacc_LDADD = zmalloc.o rblist.o

bin_PROGRAMS = zee

dist_pkgdata_DATA = 				\
	cua_bindings emacs_bindings lib.lua 	\
	tbl_vars.lua tbl_funcs.lua history.lua

dist_sysconf_DATA = $(PACKAGE)rc

info_TEXINFOS = zee.texi

zee_base_SOURCES =				\
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
	main.h					\
	tbl_opts.h				\
	tbl_keys.h				\
	basic.c					\
	bind.c					\
	buffer.c				\
	command.c				\
	completion.c				\
	file.c					\
	funcs.c					\
	help.c					\
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

zee_CPPFLAGS = -DPKGDATADIR="\"$(pkgdatadir)\"" -DSYSCONFDIR="\"$(sysconfdir)\""
zee_LDADD = @LIBOBJS@

# beep.wav is here because it is not in dist_pkgdata_DATA for non-Allegro builds
EXTRA_DIST =  							\
	mkfuncs.lua mkvars.lua mkopts.lua beep.wav

BUILT_TEXI = funcs.texi vars.texi opts.texi

BUILT_SOURCES =							\
	tbl_funcs.h tbl_funcs.lua				\
	$(PACKAGE)rc						\
	$(BUILT_TEXI)

CLEANFILES = $(BUILT_SOURCES)

if ALLEGRO
zee_SOURCES = $(zee_base_SOURCES) term_allegro.c
zee_LDFLAGS = `allegro-config --libs`
dist_pkgdata_DATA += beep.wav
else
zee_SOURCES = $(zee_base_SOURCES) term_ncurses.c
endif

$(PACKAGE).info $(PACKAGE).dvi: $(BUILT_TEXI)

tbl_funcs.h tbl_funcs.lua funcs.texi: $(zee_base_SOURCES) mkfuncs.lua
	LUA_INIT= LUA_PATH="$(top_srcdir)/src/?.lua" $(LUA) -e 'PACKAGE="$(PACKAGE)"' $(top_srcdir)/src/mkfuncs.lua $(top_srcdir)/src $(zee_base_SOURCES)

$(PACKAGE)rc vars.texi: tbl_vars.lua mkvars.lua
	LUA_INIT= LUA_PATH="$(top_srcdir)/src/?.lua" $(LUA) -e 'PACKAGE="$(PACKAGE)"' $(top_srcdir)/src/mkvars.lua $(top_srcdir)/src/tbl_vars.lua

opts.texi: tbl_opts.h mkopts.lua
	LUA_INIT= LUA_PATH="$(top_srcdir)/src/?.lua" $(LUA) -e 'PACKAGE="$(PACKAGE)"' $(top_srcdir)/src/mkopts.lua $(top_srcdir)/src/tbl_opts.h

loc:
	cloc --no3 $(zee_base_SOURCES) term_*.c *.lua
