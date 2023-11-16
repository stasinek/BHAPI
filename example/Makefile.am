AUTOMAKE_OPTIONS = 1.7

INCLUDES = -I$(top_builddir) $(ETK_CFLAGS) $(ETK_THREAD_CFLAGS)
LIBS = $(top_builddir)/etk/libetkxx.la $(ETK_THREAD_LIBS)

installed_example =		\
	window-wmaction.cpp	\
	view-drawing.cpp	\
	menu.cpp

EXTRA_DIST = Makefile.example $(installed_example)

exampledir = $(datadir)/doc/etkxx/example
example_DATA = $(installed_example)

noinst_PROGRAMS =			\
	view-drawing			\
	window-wmaction			\
	menu

view_drawing_SOURCES = view-drawing.cpp
window_wmaction_SOURCES = window-wmaction.cpp
menu_SOURCES = menu.cpp

install-makefile-example:
	$(mkinstalldirs) $(DESTDIR)$(exampledir)
	$(INSTALL_DATA) Makefile.example $(DESTDIR)$(exampledir)/Makefile

uninstall-makefile-example:
	-rm $(DESTDIR)$(exampledir)/Makefile

DISTCLEANFILES = Makefile.in

install-data-local: install-makefile-example
uninstall-local: uninstall-makefile-example

