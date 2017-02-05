AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)

INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@

noinst_LTLIBRARIES = libetk-support.la

libetk_support_la_SOURCES =	\
		Errors.h		\
		SupportDefs.h		\
		ClassInfo.h		\
		List.cpp		\
		List.h			\
		String.cpp		\
		String.h		\
		StringArray.cpp		\
		StringArray.h		\
		SimpleLocker.cpp	\
		SimpleLocker.h		\
		Locker.cpp		\
		Locker.h		\
		Autolock.h		\
		Archivable.cpp		\
		Archivable.h		\
		ByteOrder.c		\
		ByteOrder.h		\
		DataIO.cpp		\
		DataIO.h		\
		StreamIO.cpp		\
		StreamIO.h		\
		Flattenable.cpp		\
		Flattenable.h

etk_supportincludedir=$(includedir)/etkxx/etk/support
etk_supportinclude_HEADERS =	\
		Errors.h	\
		SupportDefs.h	\
		ClassInfo.h	\
		List.h		\
		String.h	\
		StringArray.h	\
		SimpleLocker.h	\
		Locker.h	\
		Autolock.h	\
		Archivable.h	\
		ByteOrder.h	\
		DataIO.h	\
		StreamIO.h	\
		Flattenable.h

DISTCLEANFILES =	\
	Makefile.in

