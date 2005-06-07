## Process this file with automake to produce Makefile.in

info_TEXINFOS = zee.texi

zeedir = $(datadir)/$(PACKAGE_NAME)
zee_DATA =					\
	AUTODOC					\
	FAQ					\
	dot$(PACKAGE_NAME).sample

EXTRA_DIST =					\
	FAQ					\
	dot$(PACKAGE_NAME).sample		\
	lisp.txt				\
	mkdoc.c					\
	BANNER.EPOC				\
	config.h

AUTODOCSRCS = $(top_srcdir)/src/*.c
AM_CFLAGS = -I$(top_srcdir) -I$(top_srcdir)/lib -I$(top_srcdir)/src

GENERATED_TEXI = dot$(PACKAGE_NAME).texi $(PACKAGE_NAME)_funcs.texi $(PACKAGE_NAME)_vars.texi

mkdoc$(EXEEXT_FOR_BUILD): mkdoc.o
	$(CC_FOR_BUILD) $(CFLAGS) $(LDFLAGS) -o $@ mkdoc.o

mkdoc.o: mkdoc.c config.h $(top_srcdir)/src/tbl_funcs.h $(top_srcdir)/src/tbl_vars.h \
         $(top_srcdir)/src/strrstr.c $(top_srcdir)/src/vasprintf.c $(top_srcdir)/src/zmalloc.c $(top_srcdir)/src/astr.c

	$(CC_FOR_BUILD) -c -I$(top_srcdir)/doc -I$(top_srcdir) -I$(top_srcdir)/src $(CFLAGS) $(srcdir)/mkdoc.c

$(PACKAGE_NAME).info: $(GENERATED_TEXI)

$(PACKAGE_NAME)_funcs.texi $(PACKAGE_NAME)_vars.texi AUTODOC: $(AUTODOCSRCS) mkdoc
	./mkdoc $(AUTODOCSRCS) >AUTODOC

dot$(PACKAGE_NAME).texi: dot$(PACKAGE_NAME).sample
	@echo "@c Automatically generated file: DO NOT EDIT!" >$@
	@echo "@setfilename dot$(PACKAGE_NAME).info" >>$@
	@echo "@example" >>$@
	expand $? >>$@
	@echo "@end example" >>$@

all-local: AUTODOC

clean-local:
	rm -f mkdoc$(EXEEXT_FOR_BUILD) AUTODOC *.o

distclean-local:
	rm -f $(GENERATED_TEXI)

BANNER.EPOC:
	@echo '$(TEXT_NAME) $(VERSION)' >>BANNER.EPOC.tmp
	@echo '© Sandro Sigala 1998-2004' >>BANNER.EPOC.tmp
	@echo '© Reuben Thomas 2003-2005' >>BANNER.EPOC.tmp
	@echo '© David A. Capello 2004' >>BANNER.EPOC.tmp
	@echo 'http://$(PACKAGE_NAME).sourceforge.net/' >>BANNER.EPOC.tmp
	@echo 'All rights reserved. This software has no warranty.' >>BANNER.EPOC.tmp
	@echo '$(TEXT_NAME) is licensed under the GNU GPL.' >>BANNER.EPOC.tmp
	@echo 'The full license is at' >>BANNER.EPOC.tmp
	@echo 'http://www.gnu.org/licenses/gpl.html' >>BANNER.EPOC.tmp
	@echo 'Proceed with installation?' >>BANNER.EPOC.tmp
	if test ! -f BANNER.EPOC -o				\
	        ! "cmp BANNER.EPOC BANNER.EPOC.tmp -s" ; then	\
		mv BANNER.EPOC.tmp BANNER.EPOC ;		\
	fi
