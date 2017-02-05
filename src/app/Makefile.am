AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)

INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@

noinst_LTLIBRARIES = libetk-app.la

libetk_app_la_SOURCES =			\
		AppDefs.h		\
		Message.cpp		\
		Message.h		\
		MessageQueue.cpp	\
		MessageQueue.h		\
		Handler.cpp		\
		Handler.h		\
		Looper.cpp		\
		Looper.h		\
		Messenger.cpp		\
		Messenger.h		\
		MessageRunner.cpp	\
		MessageRunner.h		\
		MessageFilter.cpp	\
		MessageFilter.h		\
		Invoker.cpp		\
		Invoker.h		\
		Clipboard.cpp		\
		Clipboard.h		\
		Cursor.cpp		\
		Cursor.h		\
		Application.cpp		\
		Application.h

etk_appincludedir = $(includedir)/etkxx/etk/app
etk_appinclude_HEADERS =		\
		AppDefs.h	\
		Message.h	\
		MessageQueue.h	\
		Handler.h	\
		Looper.h	\
		Messenger.h	\
		MessageRunner.h	\
		MessageFilter.h	\
		Invoker.h	\
		Clipboard.h	\
		Cursor.h	\
		Application.h

DISTCLEANFILES = Makefile.in

