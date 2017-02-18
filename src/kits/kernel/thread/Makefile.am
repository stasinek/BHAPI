AUTOMAKE_OPTIONS = 1.7

THREAD_SUBDIR =
thread_la =

if ETK_THREAD_IMPL_POSIX
THREAD_SUBDIR += posix
thread_la += posix/libthread-posix.la
endif

if ETK_THREAD_IMPL_WIN32
THREAD_SUBDIR += win32
thread_la += win32/libthread-win32.la
endif

if ETK_THREAD_IMPL_BEOS
THREAD_SUBDIR += beos
thread_la += beos/libthread-beos.la
endif

SUBDIRS = $(THREAD_SUBDIR)

noinst_LTLIBRARIES = libthread.la

libthread_la_SOURCES =

libthread_la_LIBADD = $(thread_la)
libthread_la_DEPENDENCIES = $(thread_la)

DISTCLEANFILES =	\
	Makefile.in

