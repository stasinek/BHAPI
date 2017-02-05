AUTOMAKE_OPTIONS = foreign 1.7 dist-bzip2

DISTCHECK_CONFIGURE_FLAGS = "--enable-debug=medium --enable-mem-trace=yes --enable-freetype=yes"
EFENCE = 

SUBDIRS = build etk tools manual example

if ETK_COMPILE_TEST
SUBDIRS += tests
endif

ADDONSDIRS =

if ETK_GRAPHICS_X11
if ETK_GRAPHICS_X11_BUILT_IN
else
ADDONSDIRS += $(EFENCE) $(top_builddir)/etk/interface/x11
endif
endif

if ETK_GRAPHICS_BEOS
if ETK_GRAPHICS_BEOS_BUILT_IN
else
ADDONSDIRS += $(EFENCE) $(top_builddir)/etk/interface/beos
endif
endif

if ETK_GRAPHICS_WIN32
if ETK_GRAPHICS_WIN32_BUILT_IN
else
ADDONSDIRS += $(EFENCE) $(top_builddir)/etk/interface/win32
endif
endif

if ETK_GRAPHICS_DIRECTFB
if ETK_GRAPHICS_DIRECTFB_BUILT_IN
else
ADDONSDIRS += $(EFENCE) $(top_builddir)/etk/interface/directfb
endif
endif

if ETK_GRAPHICS_LINUXFB
if ETK_GRAPHICS_LINUXFB_BUILT_IN
else
ADDONSDIRS += $(EFENCE) $(top_builddir)/etk/interface/linuxfb
endif
endif

if ETK_GRAPHICS_CARBON
if ETK_GRAPHICS_CARBON_BUILT_IN
else
ADDONSDIRS += $(EFENCE) $(top_builddir)/etk/interface/carbon
endif
endif

installed_etkdoc = 		\
	README			\
	COPYING			\
	AUTHORS			\
	TODO			\
	ChangeLog.Pre-Alpha	\
	ChangeLog

EXTRA_DIST = autogen.sh $(installed_etkdoc)

etkdocdir = $(datadir)/doc/etkxx
etkdoc_DATA = $(installed_etkdoc)

etkxxincludedir=$(includedir)/etkxx
etkxxinclude_HEADERS = etkxx.h

DISTCLEANFILES =		\
		aclocal.m4	\
		libtool.m4	\
		ltsugar.m4	\
		config.guess	\
		config.sub	\
		configure	\
		install-sh	\
		mkinstalldirs	\
		missing		\
		stamp-h.in	\
		ltmain.sh	\
		Makefile.in	\
		depcomp

distclean-local:
	-rm -rf autom4te.cache

.PHONY: snapshot addons

snapshot:
	$(MAKE) dist-bzip2 distdir=$(PACKAGE)-$(VERSION)-snap-`date +"%Y%m%d"`

addons:
	-for f in $(ADDONSDIRS) .none; do \
		(test $$f = .none || (cd $$f && $(MAKE))) \
	done

install-data-local: addons
	-for f in $(ADDONSDIRS) .none; do \
		(test $$f = .none || (cd $$f && $(MAKE) install)) \
	done

uninstall-local:
	-for f in $(ADDONSDIRS) .none; do \
		(test $$f = .none || (cd $$f && $(MAKE) uninstall)) \
	done

clean-local:
	-for f in $(ADDONSDIRS) .none; do \
		(test $$f = .none || (cd $$f && $(MAKE) clean)) \
	done


