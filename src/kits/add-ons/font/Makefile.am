AUTOMAKE_OPTIONS = 1.7

SUBDIRS =

DEFAULT_INCLUDES = -I$(top_builddir)

INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		$(FREETYPE2_CFLAGS)			\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@

noinst_LTLIBRARIES = libBHAPI_wrapper_add-ons-font.la

libetk_add_ons_font_la_SOURCES =	\
		FontEngine.cpp		\
		FontEngine.h

if HAVE_FT2
libetk_add_ons_font_la_SOURCES += FreeType2.cpp
endif

etk_add_ons_fontincludedir = $(includedir)/etkxx/etk/add-ons/font
etk_add_ons_fontinclude_HEADERS = FontEngine.h

DISTCLEANFILES = Makefile.in

