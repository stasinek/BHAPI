AUTOMAKE_OPTIONS = foreign 1.7 dist-bzip2

SUBDIRS = kernel render interface

EXTRA_DIST = autogen.sh ChangeLog

DISTCLEANFILES =		\
		aclocal.m4	\
		config.guess	\
		config.sub	\
		configure	\
		install-sh	\
		mkinstalldirs	\
		missing		\
		Makefile.in	\
		depcomp		\
		config.h.in	\
		config.h.in~

distclean-local:
	-rm -rf autom4te.cache

.PHONY: snapshot

snapshot:
	$(MAKE) dist-bzip2 distdir=$(PACKAGE)-$(VERSION)-snap-`date +"%Y%m%d"`

