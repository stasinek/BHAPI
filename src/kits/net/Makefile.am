AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)

INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@

noinst_LTLIBRARIES = libBHAPI_wrapper_net.la

libetk_net_la_SOURCES =		\
	NetAddress.cpp		\
	NetAddress.h		\
	NetBuffer.cpp		\
	NetBuffer.h		\
	NetDebug.cpp		\
	NetDebug.h		\
	NetEndpoint.cpp		\
	NetEndpoint.h

libetk_net_la_LIBADD =
libetk_net_la_DEPENDENCIES =

etk_netincludedir = $(includedir)/etkxx/etk/net
etk_netinclude_HEADERS = NetAddress.h NetBuffer.h NetDebug.h NetEndpoint.h

DISTCLEANFILES = Makefile.in

