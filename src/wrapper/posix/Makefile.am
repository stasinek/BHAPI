AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)

INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@

noinst_LTLIBRARIES = libthread-posix.la

libthread_posix_la_SOURCES =			\
			BHAPI_wrapper_thread.cpp		\
			BHAPI_wrapper_locker.cpp

if ETK_OS_DARWIN
libthread_posix_la_SOURCES += BHAPI_wrapper_semaphore-mach.cpp
else
if ETK_OS_FREEBSD
libthread_posix_la_SOURCES += BHAPI_wrapper_semaphore-umtx.cpp
else
libthread_posix_la_SOURCES += BHAPI_wrapper_semaphore.cpp
endif
endif

libthread_posix_la_LIBADD =
libthread_posix_la_DEPENDENCIES =

DISTCLEANFILES =	\
	Makefile.in

