AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)

INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@

noinst_LTLIBRARIES = libBHAPI_wrapper_xml.la

libetk_xml_la_SOURCES = SimpleXmlParser.cpp SimpleXmlParser.h
libetk_xml_la_LIBADD =
libetk_xml_la_DEPENDENCIES =

etk_xmlincludedir = $(includedir)/etkxx/etk/xml
etk_xmlinclude_HEADERS = SimpleXmlParser.h

DISTCLEANFILES = Makefile.in

