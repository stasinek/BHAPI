AUTOMAKE_OPTIONS = 1.7

SUBDIRS = icons

DEFAULT_INCLUDES = -I$(top_builddir)

INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		-DETK_COMPILATION 			\
		-DPREFIX_DIR=\"$(prefix)\"		\
		-DBIN_DIR=\"$(bindir)\"			\
		-DLIB_DIR=\"$(libdir)\"			\
		-DETC_DIR=\"$(sysconfdir)\"		\
		-DDATA_DIR=\"$(datadir)\"		\
		@ETK_DEBUG_FLAGS@

noinst_LTLIBRARIES = libBHAPI_wrapper_storage.la

libetk_storage_la_SOURCES =		\
		StorageDefs.h		\
		Path.h			\
		Path.cpp		\
		Entry.h			\
		Entry.cpp		\
		Directory.h		\
		Directory.cpp		\
		Node.h			\
		Node.cpp		\
		Volume.h		\
		Volume.cpp		\
		VolumeRoster.h		\
		VolumeRoster.cpp	\
		File.h			\
		File.cpp		\
		FindDirectory.h		\
		FindDirectory.cpp	\
		FilePanel.h		\
		FilePanel.cpp

etk_storageincludedir=$(includedir)/etkxx/etk/storage
etk_storageinclude_HEADERS =		\
		StorageDefs.h		\
		Path.h			\
		Entry.h			\
		Directory.h		\
		Volume.h		\
		VolumeRoster.h		\
		File.h			\
		FindDirectory.h		\
		FilePanel.h

DISTCLEANFILES =	\
	Makefile.in

