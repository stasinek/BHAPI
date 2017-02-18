AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)

if ETK_GRAPHICS_WIN32_BUILT_IN
INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		$(ETK_GRAPHICS_WIN32_CFLAGS)		\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@
else
INCLUDES =	$(ETK_CFLAGS)	 			\
		$(ETK_THREAD_CFLAGS)			\
		$(ETK_GRAPHICS_WIN32_CFLAGS)		\
		@ETK_DEBUG_FLAGS@
endif

addonsdir = $(libdir)/add-ons/etkxx/graphics
noinst_LTLIBRARIES = libgraphics-win32.la

libgraphics_win32_la_SOURCES =			\
			etk-win32gdi.h		\
			etk-application.cpp	\
			etk-window.cpp		\
			etk-pixmap.cpp		\
			etk-drawing.cpp		\
			etk-win32-font.cpp

no_undefined = -no-undefined

if ETK_GRAPHICS_WIN32_BUILT_IN
else
libgraphics_win32_la_LDFLAGS =				\
			-avoid-version			\
			-export-dynamic			\
			$(no_undefined)			\
			-rpath $(addonsdir)
libgraphics_win32_la_LIBADD =				\
			$(top_builddir)/etk/libetkxx.la	\
			$(ETK_LIBS)			\
			$(ETK_THREAD_LIBS)		\
			$(ETK_GRAPHICS_WIN32_LIBS)
libgraphics_win32_la_DEPENDENCIES = $(top_builddir)/etk/libetkxx.la
endif

DISTCLEANFILES = Makefile.in

if ETK_GRAPHICS_WIN32_BUILT_IN
else
install-data-local: libgraphics-win32.la
	$(mkinstalldirs) $(DESTDIR)$(addonsdir)
	$(LIBTOOL) --mode=install $(INSTALL) libgraphics-win32.la $(DESTDIR)$(addonsdir)/libgraphics-win32.la
	rm -f $(DESTDIR)$(addonsdir)/libgraphics-win32.dll.a
	rm -f $(DESTDIR)$(addonsdir)/libgraphics-win32.la
	mv -f $(DESTDIR)$(addonsdir)/../bin/libgraphics-win32.dll $(DESTDIR)$(addonsdir)/libgraphics-win32.dll
	-rmdir $(DESTDIR)$(addonsdir)/../bin

uninstall-local:
	rm -f $(DESTDIR)$(addonsdir)/libgraphics-win32.dll
endif

