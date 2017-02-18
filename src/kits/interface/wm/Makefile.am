AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)

INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@

noinst_LTLIBRARIES = libetk-interface-wm.la

libetk_interface_wm_la_SOURCES =	\
	SimpleWM.cpp			\
	SimpleWM.h

libetk_interface_wm_la_LIBADD =
libetk_interface_wm_la_DEPENDENCIES =

DISTCLEANFILES = Makefile.in

