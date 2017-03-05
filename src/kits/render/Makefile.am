AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)

INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@

noinst_LTLIBRARIES = libBHAPI_wrapper_render.la

libetk_render_la_SOURCES =			\
		LineGenerator.cpp		\
		LineGenerator.h			\
		ArcGenerator.cpp		\
		ArcGenerator.h			\
		Render.cpp			\
		Render.h			\
		Pixmap.cpp			\
		Pixmap.h

libetk_render_la_LIBADD =
libetk_render_la_DEPENDENCIES =

etk_renderincludedir = $(includedir)/etkxx/etk/render
etk_renderinclude_HEADERS = LineGenerator.h ArcGenerator.h Render.h Pixmap.h

DISTCLEANFILES = Makefile.in

