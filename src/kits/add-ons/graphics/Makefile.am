AUTOMAKE_OPTIONS = 1.7

SUBDIRS =

DEFAULT_INCLUDES = -I$(top_builddir)

INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@

noinst_LTLIBRARIES = libBHAPI_wrapper_add-ons-graphics.la

libetk_add_ons_graphics_la_SOURCES = GraphicsEngine.cpp	GraphicsEngine.h

etk_add_ons_graphicsincludedir = $(includedir)/etkxx/etk/add-ons/graphics
etk_add_ons_graphicsinclude_HEADERS = GraphicsEngine.h

DISTCLEANFILES = Makefile.in

