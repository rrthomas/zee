## Process this file with automake to produce Makefile.in

if HAVE_REGEX_H
regex =
else
regex = regex.o
endif

TESTS = astr
check_PROGRAMS = astr

astr_CPPFLAGS = -DTEST -DSRCPATH="\"$(top_srcdir)/src/\""
astr_LDADD = zmalloc.o vector.o

bin_PROGRAMS = zee

zeedir = $(datadir)/$(PACKAGE_NAME)
zee_DATA =					\
	key_bindings.el

zee_base_SOURCES =				\
	vector.c				\
	vector.h				\
	list.c					\
	list.h					\
	astr.c					\
	astr.h					\
	zmalloc.h				\
	zmalloc.c				\
	parser.c				\
	basic.c					\
	bind.c					\
	buffer.c				\
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

zee_LDADD = $(regex) @LIBOBJS@

EXTRA_DIST = regex.c regex.h key_bindings.el

distclean-local:
	rm -f paths.h tbl_funcs.h

help.o: paths.h

paths.h:
	@echo "/* Automatically generated file.  NO NOT EDIT! */" >paths.h.tmp
	@echo "" >>paths.h.tmp
	@echo '#define PATH_PREFIX "$(prefix)"' >>paths.h.tmp
	@echo '#define PATH_DATA "$(pkgdatadir)"' >>paths.h.tmp
	if test ! -f paths.h -o				\
	        ! "cmp paths.h paths.h.tmp -s" ; then	\
		mv paths.h.tmp paths.h ;		\
	fi

sloccount:
	sloccount $(zee_base_SOURCES) $(EXTRA_DIST) term_*.c
