## Process this file with automake to produce Makefile.in
## $Id$

if HAVE_REGEX_H
regex =
else
regex = regex.o
endif

TESTS = astr tests
check_PROGRAMS = astr

astr_CPPFLAGS = -DTEST -DSRCPATH="\"$(top_srcdir)/src\""
astr_LDADD = zmalloc.o strrstr.o

bin_PROGRAMS = zee

zee_base_SOURCES =				\
	list.c					\
	list.h					\
	astr.c					\
	astr.h					\
	vector.c				\
	vector.h				\
	zmalloc.c				\
	eval.c					\
	eval.h					\
	lists.c					\
	lists.h					\
	parser.c				\
	parser.h				\
	vars.c					\
	vars.h					\
	lisp.c					\
	basic.c					\
	bind.c					\
	buffer.c				\
	completion.c				\
	display.c				\
	editfns.c				\
	extern.h				\
	file.c					\
	funcs.c					\
	glue.c					\
	help.c					\
	history.c				\
	keys.c					\
	killring.c				\
	line.c					\
	macro.c					\
	main.c					\
	marker.c				\
	minibuf.c				\
	point.c					\
	search.c				\
	tbl_funcs.h				\
	tbl_vars.h				\
	undo.c					\
	variables.c				\
	window.c				\
	term_minibuf.c				\
	term_display.c				\
	zee.h

if ALLEGRO
zee_SOURCES = $(zee_base_SOURCES) term_allegro.c
zee_LDFLAGS = `allegro-config --libs`
else
if EPOCEMX
zee_SOURCES = $(zee_base_SOURCES) term_epocemx.c
else
zee_SOURCES = $(zee_base_SOURCES) term_termcap.c
endif
endif

zee_LDADD = $(regex) @LIBOBJS@

EXTRA_DIST = regex.c regex.h tests vasprintf.c strrstr.c

distclean-local:
	rm -f paths.h

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
