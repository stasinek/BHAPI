AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)

if ETK_GRAPHICS_X11_BUILT_IN
INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		$(ETK_GRAPHICS_X11_CFLAGS)		\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@
else
INCLUDES =	$(ETK_CFLAGS)	 			\
		$(ETK_THREAD_CFLAGS)			\
		$(ETK_GRAPHICS_X11_CFLAGS)		\
		@ETK_DEBUG_FLAGS@
endif

if ETK_OS_BEOS
addonsdir = /boot/home/config/add-ons/etkxx/graphics
else
addonsdir = $(libdir)/add-ons/etkxx/graphics
endif
noinst_LTLIBRARIES = libgraphics-x11.la

libgraphics_x11_la_SOURCES =			\
			BHAPI_wrapper_x11.h		\
			BHAPI_wrapper_application.cpp	\
			BHAPI_wrapper_window.cpp		\
			BHAPI_wrapper_pixmap.cpp		\
			BHAPI_wrapper_drawing.cpp		\
			BHAPI_wrapper_x11-font.cpp

if ETK_OS_WIN32
no_undefined = -no-undefined
endif

if ETK_OS_CYGWIN
no_undefined = -no-undefined
endif

if ETK_OS_BEOS
no_undefined = -no-undefined
no_start = -Wc,-nostart
endif

if ETK_GRAPHICS_X11_BUILT_IN
else
libgraphics_x11_la_LDFLAGS =				\
			-avoid-version			\
			-export-dynamic			\
			$(no_undefined)			\
			$(no_start)			\
			-rpath $(addonsdir)
libgraphics_x11_la_LIBADD =				\
			$(top_builddir)/etk/libetkxx.la	\
			$(ETK_LIBS)			\
			$(ETK_THREAD_LIBS)		\
			$(ETK_GRAPHICS_X11_LIBS)
libgraphics_x11_la_DEPENDENCIES = $(top_builddir)/etk/libetkxx.la
endif

DISTCLEANFILES = Makefile.in

if ETK_GRAPHICS_X11_BUILT_IN
else
install-data-local: libgraphics-x11.la
	$(mkinstalldirs) $(DESTDIR)$(addonsdir)
	$(LIBTOOL) --mode=install $(INSTALL) libgraphics-x11.la $(DESTDIR)$(addonsdir)/libgraphics-x11.la
if ETK_NATIVE_WIN32
	rm -f $(DESTDIR)$(addonsdir)/libgraphics-x11.dll.a
	rm -f $(DESTDIR)$(addonsdir)/libgraphics-x11.la
	mv -f $(DESTDIR)$(addonsdir)/../bin/libgraphics-x11.dll $(DESTDIR)$(addonsdir)/libgraphics-x11.dll
	-rmdir $(DESTDIR)$(addonsdir)/../bin
endif

uninstall-local:
if ETK_NATIVE_WIN32
	rm -f $(DESTDIR)$(addonsdir)/libgraphics-x11.dll
else
	$(LIBTOOL) --mode=uninstall rm -f $(DESTDIR)$(addonsdir)/libgraphics-x11.la
endif

endif # !ETK_GRAPHICS_X11_BUILT_IN

