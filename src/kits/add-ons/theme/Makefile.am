AUTOMAKE_OPTIONS = 1.7

SUBDIRS =

DEFAULT_INCLUDES = -I$(top_builddir)

INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@

noinst_LTLIBRARIES = libetk-add-ons-theme.la

libetk_add_ons_theme_la_SOURCES =	\
		ThemeEngine.cpp		\
		ThemeEngine.h		\
		DefaultTheme.cpp

etk_add_ons_themeincludedir = $(includedir)/etkxx/etk/add-ons/theme
etk_add_ons_themeinclude_HEADERS = ThemeEngine.h

DISTCLEANFILES = Makefile.in

