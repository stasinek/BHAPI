AUTOMAKE_OPTIONS = 1.7

SUBDIRS = thread

EFENCE = 

thread_la = thread/libthread.la
kernel_la =

if ETK_OS_UNIX
SUBDIRS += $(EFENCE) unix
kernel_la += $(EFENCE) unix/libkernel-unix.la
endif

if ETK_OS_WIN32
SUBDIRS += $(EFENCE) win32
kernel_la += $(EFENCE) win32/libkernel-win32.la
endif

if ETK_OS_BEOS
SUBDIRS += $(EFENCE) beos
kernel_la += $(EFENCE) beos/libkernel-beos.la
endif

DEFAULT_INCLUDES = -I$(top_builddir)

INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@

noinst_LTLIBRARIES = libBHAPI_wrapper_kernel.la

libetk_kernel_la_SOURCES =			\
			Kernel.h		\
			Debug.cpp		\
			Debug.h			\
			OS.h			\
			BHAPI_wrapper_port.cpp

etk_kernelincludedir = $(includedir)/etkxx/etk/kernel
etk_kernelinclude_HEADERS =	\
	Kernel.h		\
	OS.h			\
	Debug.h

libetk_kernel_la_LIBADD = $(kernel_la) $(thread_la)
libetk_kernel_la_DEPENDENCIES = $(kernel_la) $(thread_la)

DISTCLEANFILES = Makefile.in

