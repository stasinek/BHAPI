AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)

INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@

noinst_LTLIBRARIES = libthread-beos.la

libthread_beos_la_SOURCES =			\
			BHAPI_wrapper_thread.cpp		\
			BHAPI_wrapper_locker.cpp		\
			BHAPI_wrapper_semaphore.cpp

libthread_beos_la_LIBADD =
libthread_beos_la_DEPENDENCIES =

DISTCLEANFILES =	\
	Makefile.in

