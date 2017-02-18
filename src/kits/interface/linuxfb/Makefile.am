AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)

if ETK_GRAPHICS_LINUXFB_BUILT_IN
INCLUDES =	$(ETK_THREAD_CFLAGS) 		\
		$(ETK_EXTRA_CFLAGS)		\
		$(ETK_GRAPHICS_LINUXFB_CFLAGS)	\
		-DETK_COMPILATION 		\
		@ETK_DEBUG_FLAGS@
else
INCLUDES =	$(ETK_CFLAGS)	 		\
		$(ETK_THREAD_CFLAGS)		\
		$(ETK_GRAPHICS_LINUXFB_CFLAGS)	\
		@ETK_DEBUG_FLAGS@
endif

addonsdir = $(libdir)/add-ons/etkxx/graphics
noinst_LTLIBRARIES = libgraphics-linuxfb.la

libgraphics_linuxfb_la_SOURCES =

if ETK_GRAPHICS_LINUXFB_BUILT_IN
else
libgraphics_linuxfb_la_LDFLAGS =		\
			-avoid-version		\
			-export-dynamic		\
			-rpath $(addonsdir)
libgraphics_linuxfb_la_LIBADD =					\
	$(top_builddir)/etk/libetkxx.la				\
	$(top_builddir)/etk/interface/wm/libetk-interface-wm.la	\
	$(ETK_LIBS)						\
	$(ETK_THREAD_LIBS)					\
	$(ETK_GRAPHICS_LINUXFB_LIBS)
libgraphics_linuxfb_la_DEPENDENCIES =				\
	$(top_builddir)/etk/libetkxx.la				\
	$(top_builddir)/etk/interface/wm/libetk-interface-wm.la
endif

DISTCLEANFILES = Makefile.in

if ETK_GRAPHICS_LINUXFB_BUILT_IN
else
install-data-local: libgraphics-linuxfb.la
	$(mkinstalldirs) $(DESTDIR)$(addonsdir)
	$(LIBTOOL) --mode=install $(INSTALL) libgraphics-linuxfb.la $(DESTDIR)$(addonsdir)/libgraphics-linuxfb.la

uninstall-local:
	$(LIBTOOL) --mode=uninstall rm -f $(DESTDIR)$(addonsdir)/libgraphics-linuxfb.la
endif

