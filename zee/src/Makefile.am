## Process this file with automake to produce Makefile.in

TESTS = rblist rbacc
check_PROGRAMS = rblist rbacc

TESTCPPFLAGS = -DTEST -DSRCPATH="\"$(srcdir)/\""

rblist_CPPFLAGS = $(TESTCPPFLAGS)
rblist_LDADD = zmalloc.o rbacc.o rbutil.o

rbacc_CPPFLAGS = $(TESTCPPFLAGS)
rbacc_LDADD = zmalloc.o rblist.o

bin_PROGRAMS = zee

LUA_RUNTIME = 					\
	lib.lua					\
	tbl_vars.lua				\
	tbl_funcs.lua				\
	tbl_keys.h				\
	bind.lua				\
	completion.lua 				\
	history.lua				\
	keys.lua				\
	minibuf.lua				\
	texinfo.lua

LUA_RUNTIME_EXTRA = std.lua

dist_pkgdata_DATA = 				\
	$(LUA_RUNTIME)				\
	$(LUA_RUNTIME_EXTRA)			\
	cua_bindings emacs_bindings

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
	zmalloc.h			        \
	zmalloc.c			        \
	term.h				        \
	extern.h			        \
	main.h					\
	tbl_opts.h				\
	tbl_keys.h				\
	copyright.h				\
	basic.c					\
	bind.c					\
	buffer.c				\
	file.c					\
	funcs.c					\
	help.c					\
	keys.c					\
	killring.c				\
	line.c					\
	lua.c					\
	macro.c					\
	main.c					\
	minibuf.c				\
	point.c					\
	search.c				\
	term.c					\
	undo.c

zee_CPPFLAGS = -DPKGDATADIR="\"$(pkgdatadir)\"" -DSYSCONFDIR="\"$(sysconfdir)\""
zee_LDADD = @LIBOBJS@ ../bitlib/bit.la

LUA_BUILDTIME = mkfuncs.lua mkvars.lua mkopts.lua mkkeys.lua mkcopyright.lua

# beep.wav is here because it is not in dist_pkgdata_DATA for non-Allegro builds
EXTRA_DIST = $(LUA_BUILDTIME) beep.wav clue.h

BUILT_TEXI = funcs.texi vars.texi opts.texi keys.texi copyright.texi

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
	LUA_INIT= LUA_PATH="$(srcdir)/?.lua" $(LUA) -e 'PACKAGE="$(PACKAGE)"' -e 'PACKAGE_NAME="$(PACKAGE_NAME)"' $(srcdir)/mkfuncs.lua $(srcdir) $(zee_base_SOURCES)

$(PACKAGE)rc vars.texi: tbl_vars.lua mkvars.lua
	LUA_INIT= LUA_PATH="$(srcdir)/?.lua" $(LUA) -e 'PACKAGE="$(PACKAGE)"' -e 'PACKAGE_NAME="$(PACKAGE_NAME)"' $(srcdir)/mkvars.lua $(srcdir)/tbl_vars.lua

opts.texi: tbl_opts.h mkopts.lua
	LUA_INIT= LUA_PATH="$(srcdir)/?.lua" $(LUA) -e 'PACKAGE="$(PACKAGE)"' -e 'PACKAGE_NAME="$(PACKAGE_NAME)"' $(srcdir)/mkopts.lua $(srcdir)/tbl_opts.h

keys.texi: tbl_keys.h mkkeys.lua
	LUA_INIT= LUA_PATH="$(srcdir)/?.lua" $(LUA) -e 'PACKAGE="$(PACKAGE)"' -e 'PACKAGE_NAME="$(PACKAGE_NAME)"' $(srcdir)/mkkeys.lua $(srcdir)/tbl_keys.h

copyright.texi: copyright.h mkcopyright.lua
	LUA_INIT= LUA_PATH="$(srcdir)/?.lua" $(LUA) -e 'PACKAGE="$(PACKAGE)"' -e 'PACKAGE_NAME="$(PACKAGE_NAME)"' $(srcdir)/mkcopyright.lua $(srcdir)/copyright.h

loc:
	cloc --no3 --force_lang=m4,ac --force_lang=Make,am $(zee_base_SOURCES) term_*.c Makefile.am ../Makefile.am ../configure.ac \
		$(LUA_BUILDTIME) $(LUA_RUNTIME)
