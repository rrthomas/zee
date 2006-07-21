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

dist_pkgdata_DATA =				\
	key_bindings.el

zee_base_SOURCES =				\
	rblist.c				\
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

distclean-local:
	rm -f tbl_funcs.h

sloccount:
	sloccount $(zee_base_SOURCES) term_*.c
