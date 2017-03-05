AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)

if ETK_GRAPHICS_CARBON_BUILT_IN
INCLUDES =	$(ETK_THREAD_CFLAGS) 		\
		$(ETK_EXTRA_CFLAGS)		\
		$(ETK_GRAPHICS_CARBON_CFLAGS)	\
		-DETK_COMPILATION 		\
		@ETK_DEBUG_FLAGS@
else
INCLUDES =	$(ETK_CFLAGS)	 		\
		$(ETK_THREAD_CFLAGS)		\
		$(ETK_GRAPHICS_CARBON_CFLAGS)	\
		@ETK_DEBUG_FLAGS@
endif

addonsdir = $(libdir)/add-ons/etkxx/graphics
noinst_LTLIBRARIES = libgraphics-carbon.la

libgraphics_carbon_la_SOURCES =			\
	BHAPI_wrapper_carbon.h

if ETK_GRAPHICS_CARBON_BUILT_IN
else
libgraphics_carbon_la_LDFLAGS =			\
			-avoid-version		\
			-export-dynamic		\
			-rpath $(addonsdir)
libgraphics_carbon_la_LIBADD =					\
	$(top_builddir)/etk/libetkxx.la				\
	$(ETK_LIBS)						\
	$(ETK_THREAD_LIBS)					\
	$(ETK_GRAPHICS_CARBON_LIBS)
libgraphics_carbon_la_DEPENDENCIES =				\
	$(top_builddir)/etk/libetkxx.la
endif

DISTCLEANFILES = Makefile.in

if ETK_GRAPHICS_CARBON_BUILT_IN
else
install-data-local: libgraphics-carbon.la
	$(mkinstalldirs) $(DESTDIR)$(addonsdir)
	$(LIBTOOL) --mode=install $(INSTALL) libgraphics-carbon.la $(DESTDIR)$(addonsdir)/libgraphics-carbon.la

uninstall-local:
	$(LIBTOOL) --mode=uninstall rm -f $(DESTDIR)$(addonsdir)/libgraphics-carbon.la
endif


