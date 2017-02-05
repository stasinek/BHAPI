AUTOMAKE_OPTIONS = 1.7

EXTRA_DIST =			\
	window-wmaction.dsp	\
	view-drawing.dsp	\
	menu.dsp		\
	window-wmaction.vcproj	\
	view-drawing.vcproj	\
	menu.vcproj

CLEANFILES =			\
	window-wmaction.plg	\
	view-drawing.plg	\
	menu.plg

DISTCLEANFILES = Makefile.in

clean-local:
	-rm -f *.vcproj.*.user
	-rm -rf Debug Release

