AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)

if ETK_GRAPHICS_BEOS_BUILT_IN
INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		$(ETK_GRAPHICS_BEOS_CFLAGS)		\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@
else
INCLUDES =	$(ETK_CFLAGS)	 			\
		$(ETK_THREAD_CFLAGS)			\
		$(ETK_GRAPHICS_BEOS_CFLAGS)		\
		@ETK_DEBUG_FLAGS@
endif

addonsdir = /boot/home/config/add-ons/etkxx/graphics
noinst_LTLIBRARIES = libgraphics-beos.la

libgraphics_beos_la_SOURCES =				\
		BHAPI_wrapper_beos-graphics.h			\
		BHAPI_wrapper_application.cpp			\
		BHAPI_wrapper_drawing.cpp				\
		BHAPI_wrapper_pixmap.cpp				\
		BHAPI_wrapper_window.cpp				\
		BHAPI_wrapper_beos-font.cpp

if ETK_GRAPHICS_BEOS_BUILT_IN
else
libgraphics_beos_la_LDFLAGS =				\
			-Wc,-nostart			\
			-no-undefined			\
			-avoid-version			\
			-export-dynamic			\
			-rpath $(addonsdir)
libgraphics_beos_la_LIBADD =				\
			$(top_builddir)/etk/libetkxx.la	\
			$(ETK_LIBS)			\
			$(ETK_THREAD_LIBS)		\
			$(ETK_GRAPHICS_BEOS_LIBS)
libgraphics_beos_la_DEPENDENCIES = $(top_builddir)/etk/libetkxx.la
endif

DISTCLEANFILES = Makefile.in

if ETK_GRAPHICS_BEOS_BUILT_IN
else
install-data-local: libgraphics-beos.la
	$(mkinstalldirs) $(DESTDIR)$(addonsdir)
	$(LIBTOOL) --mode=install $(INSTALL) libgraphics-beos.la $(DESTDIR)$(addonsdir)/libgraphics-beos.la

uninstall-local:
	$(LIBTOOL) --mode=uninstall rm -f $(DESTDIR)$(addonsdir)/libgraphics-beos.la
endif

