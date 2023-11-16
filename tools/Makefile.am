AUTOMAKE_OPTIONS = 1.7

INCLUDES = -I$(top_builddir) $(ETK_CFLAGS) $(ETK_THREAD_CFLAGS)
LIBS = $(top_builddir)/etk/libetkxx.la $(ETK_THREAD_LIBS)

noinst_PROGRAMS = etkxx-doc
bin_SCRIPTS = etkxx-config

etkxx_doc_SOURCES = etkxx-doc.cpp

EXTRA_DIST = check-copying
DISTCLEANFILES = Makefile.in etkxx-config

