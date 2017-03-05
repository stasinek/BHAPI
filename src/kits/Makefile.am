AUTOMAKE_OPTIONS = 1.7

DEFAULT_INCLUDES = -I$(top_builddir)
INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@

SUBDIRS = private kernel support storage xml app render interface net add-ons m4macros

if ETK_INSTALL_PKG_CONFIG_FILES
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = etkxx.pc
endif

$(pkgconfig_DATA): $(top_builddir)/config.status

if ETK_OS_WIN32
noinst_DATA = etkxx.lib

install-ms-lib:
	$(INSTALL_DATA) etkxx.lib $(DESTDIR)$(libdir)/etkxx.lib

uninstall-ms-lib:
	-rm $(DESTDIR)$(libdir)/etkxx.lib
else
install-ms-lib:
uninstall-ms-lib:
endif

lib_LTLIBRARIES = libetkxx.la
libetkxx_la_SOURCES = ETKBuild.h ETKBuild.cpp

ETK_OBJECTS =					\
	private/libBHAPI_wrapper_private.la		\
	kernel/libBHAPI_wrapper_kernel.la			\
	support/libBHAPI_wrapper_support.la		\
	storage/libBHAPI_wrapper_storage.la		\
	xml/libBHAPI_wrapper_xml.la			\
	app/libBHAPI_wrapper_app.la			\
	render/libBHAPI_wrapper_render.la			\
	interface/libBHAPI_wrapper_interface.la		\
	net/libBHAPI_wrapper_net.la			\
	add-ons/libBHAPI_wrapper_add-ons.la

etkxxincludedir=$(includedir)/etkxx/etk
etkxxinclude_HEADERS =				\
			ETK.h			\
			ETKBuild.h		\
			SupportKit.h		\
			KernelKit.h		\
			StorageKit.h		\
			AppKit.h		\
			RenderKit.h		\
			InterfaceKit.h		\
			XmlKit.h		\
			NetKit.h

if ETK_OS_WIN32

noinst_LTLIBRARIES = libetkxx-pre.la
libetkxx_pre_la_SOURCES =
libetkxx_pre_la_LIBADD = $(ETK_OBJECTS) $(ETK_WIN32_RESOURCE)
libetkxx_pre_la_DEPENDENCIES = $(ETK_OBJECTS) $(ETK_WIN32_RESOURCE)
libetkxx_la_LIBADD = libetkxx-pre.la $(ETK_EXTRA_LIBS) $(ETK_THREAD_LIBS)
libetkxx_la_DEPENDENCIES = libetkxx-pre.la $(ETK_DEF)

$(ETK_WIN32_RESOURCE): etkxx.rc
	$(LIBTOOL) --tag=disable-static --mode=compile $(WINDRES) -o $@ $<

if MS_LIB_AVAILABLE
etkxx.lib: libetkxx.la $(ETK_DEF)
	lib -name:libetkxx-$(LT_CURRENT_MINUS_AGE).dll -def:$(ETK_DEF) -out:$@
else # !MS_LIB_AVAILABLE
etkxx.lib: libetkxx.la $(ETK_DEF)
	$(DLLTOOL) -k --dllname libetkxx-$(LT_CURRENT_MINUS_AGE).dll --output-lib $@ --def $(ETK_DEF)
endif # MS_LIB_AVAILABLE

$(ETK_DEF): libetkxx-pre.la
	$(DLLTOOL) --export-all --output-def $@ .libs/libetkxx-pre.a .libs/*.o

else # !ETK_OS_WIN32

libetkxx_la_LIBADD = $(ETK_OBJECTS) $(ETK_EXTRA_LIBS) $(ETK_THREAD_LIBS)
libetkxx_la_DEPENDENCIES = $(ETK_OBJECTS)

endif # ETK_OS_WIN32

if ETK_OS_WIN32
no_undefined = -no-undefined
export_symbols = -export-symbols $(ETK_DEF)
endif

if ETK_OS_CYGWIN
no_undefined = -no-undefined
endif

if ETK_OS_BEOS
no_undefined = -no-undefined
no_start = -Wc,-nostart
endif

libetkxx_la_LDFLAGS = @STRIP_BEGIN@					\
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) 		\
	-release $(LT_RELEASE)						\
	-export-dynamic 						\
	-rpath $(libdir)						\
	$(no_undefined)							\
	$(no_start)							\
	$(export_symbols)						\
	@STRIP_END@

EXTRA_DIST =	config.h

CLEANFILES =				\
		etkxx.def		\
		etkxx.lib		\
		etkxx.exp		\
		etkxx.aps

DISTCLEANFILES =			\
		Makefile.in		\
		config.h.general.in	\
		config.h.general.in~	\
		etkxx.pc		\
		etkxx.rc		\
		etkxx.aps		\
		config.h.msvc.win32	\
		etkxx.def		\
		etkxx.lib		\
		etkxx.exp		\
		ETKBuild.h

install-data-local: install-ms-lib

uninstall-local: uninstall-ms-lib

dist-hook:
	-rm $(distdir)/ETKBuild.h

