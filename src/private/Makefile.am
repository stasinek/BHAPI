AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)

INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@

noinst_LTLIBRARIES = libetk-private.la

libetk_private_la_SOURCES =		\
		Memory.cpp		\
		Memory.h		\
		Object.cpp		\
		Object.h		\
		Token.cpp		\
		Token.h			\
		PrivateApplication.cpp	\
		PrivateApplication.h	\
		PrivateHandler.cpp	\
		PrivateHandler.h	\
		StandardIO.cpp		\
		StandardIO.h		\
		MessageBody.cpp		\
		MessageBody.h

DISTCLEANFILES =	\
	Makefile.in

