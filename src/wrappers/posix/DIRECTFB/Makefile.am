AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)

if ETK_GRAPHICS_DIRECTFB_BUILT_IN
INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		$(ETK_GRAPHICS_DIRECTFB_CFLAGS)		\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@
else
INCLUDES =	$(ETK_CFLAGS)	 			\
		$(ETK_THREAD_CFLAGS)			\
		$(ETK_GRAPHICS_DIRECTFB_CFLAGS)		\
		@ETK_DEBUG_FLAGS@
endif

addonsdir = $(libdir)/add-ons/etkxx/graphics
noinst_LTLIBRARIES = libgraphics-directfb.la

libgraphics_directfb_la_SOURCES =		\
			BHAPI_wrapper_dfb.h		\
			BHAPI_wrapper_application.cpp	\
			BHAPI_wrapper_window.cpp		\
			BHAPI_wrapper_wm.cpp		\
			BHAPI_wrapper_pixmap.cpp		\
			BHAPI_wrapper_drawing.cpp		\
			BHAPI_wrapper_dfb-font.cpp

if ETK_GRAPHICS_DIRECTFB_BUILT_IN
else
libgraphics_directfb_la_LDFLAGS =			\
			-avoid-version			\
			-export-dynamic			\
			-rpath $(addonsdir)
libgraphics_directfb_la_LIBADD =			\
			$(top_builddir)/etk/libetkxx.la	\
			$(ETK_LIBS)			\
			$(ETK_THREAD_LIBS)		\
			$(ETK_GRAPHICS_DIRECTFB_LIBS)
libgraphics_directfb_la_DEPENDENCIES = $(top_builddir)/etk/libetkxx.la
endif

DISTCLEANFILES = Makefile.in

if ETK_GRAPHICS_DIRECTFB_BUILT_IN
else
install-data-local: libgraphics-directfb.la
	$(mkinstalldirs) $(DESTDIR)$(addonsdir)
	$(LIBTOOL) --mode=install $(INSTALL) libgraphics-directfb.la $(DESTDIR)$(addonsdir)/libgraphics-directfb.la

uninstall-local:
	$(LIBTOOL) --mode=uninstall rm -f $(DESTDIR)$(addonsdir)/libgraphics-directfb.la
endif

