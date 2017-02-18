AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)

INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@

noinst_LTLIBRARIES = libthread-win32.la

libthread_win32_la_SOURCES =			\
			etk-thread.cpp		\
			etk-locker.cpp		\
			etk-semaphore.cpp

libthread_win32_la_LIBADD =
libthread_win32_la_DEPENDENCIES =

DISTCLEANFILES =	\
	Makefile.in

