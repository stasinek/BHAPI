AUTOMAKE_OPTIONS = 1.7

SUBDIRS = layout icons wm
EFENCE = 
graphics_la = layout/libBHAPI_wrapper_interface-layout.la

if ETK_GRAPHICS_X11_BUILT_IN
SUBDIRS += $(EFENCE) x11
graphics_la += $(EFENCE) x11/libgraphics-x11.la
endif

if ETK_GRAPHICS_BEOS_BUILT_IN
SUBDIRS += $(EFENCE) beos
graphics_la += $(EFENCE) beos/libgraphics-beos.la
endif

if ETK_GRAPHICS_WIN32_BUILT_IN
SUBDIRS += $(EFENCE) win32
graphics_la += $(EFENCE) win32/libgraphics-win32.la
endif

if ETK_GRAPHICS_DIRECTFB_BUILT_IN
SUBDIRS += $(EFENCE) directfb
graphics_la += $(EFENCE) directfb/libgraphics-directfb.la
endif

if ETK_GRAPHICS_LINUXFB_BUILT_IN
SUBDIRS += $(EFENCE) linuxfb
graphics_la += $(EFENCE) wm/libBHAPI_wrapper_interface-wm.la linuxfb/libgraphics-linuxfb.la
endif

if ETK_GRAPHICS_CARBON_BUILT_IN
SUBDIRS += $(EFENCE) carbon
graphics_la += $(EFENCE) carbon/libgraphics-carbon.la
endif

DEFAULT_INCLUDES = -I$(top_builddir)

INCLUDES =	$(ETK_THREAD_CFLAGS) 			\
		$(ETK_EXTRA_CFLAGS)			\
		-DETK_COMPILATION 			\
		@ETK_DEBUG_FLAGS@

noinst_LTLIBRARIES = libBHAPI_wrapper_interface.la

libetk_interface_la_SOURCES =		\
		GraphicsDefs.cpp	\
		GraphicsDefs.h		\
		InterfaceDefs.h		\
		Point.cpp		\
		Point.h			\
		Rect.cpp		\
		Rect.h			\
		Polygon.cpp		\
		Polygon.h		\
		Region.cpp		\
		Region.h		\
		Font.cpp		\
		Font.h			\
		Theme.cpp		\
		Screen.cpp		\
		Screen.h		\
		ViewPrivate.h		\
		Window.cpp		\
		Window.h		\
		View.cpp		\
		View.h			\
		ScrollBar.cpp		\
		ScrollBar.h		\
		ScrollView.cpp		\
		ScrollView.h		\
		Control.cpp		\
		Control.h		\
		ColorControl.cpp	\
		ColorControl.h		\
		Button.cpp		\
		Button.h		\
		Alert.cpp		\
		Alert.h			\
		Menu.cpp		\
		Menu.h			\
		MenuItem.cpp		\
		MenuItem.h		\
		PopUpMenu.cpp		\
		PopUpMenu.h		\
		MenuBar.cpp		\
		MenuBar.h		\
		MenuField.cpp		\
		MenuField.h		\
		TextEditable.cpp	\
		TextEditable.h		\
		TextControl.cpp		\
		TextControl.h		\
		StringView.cpp		\
		StringView.h		\
		Box.cpp			\
		Box.h			\
		StatusBar.cpp		\
		StatusBar.h		\
		CheckBox.cpp		\
		CheckBox.h		\
		RadioButton.cpp		\
		RadioButton.h		\
		ListItem.cpp		\
		ListItem.h		\
		ListView.cpp		\
		ListView.h		\
		OutlineListView.cpp	\
		OutlineListView.h	\
		TabView.cpp		\
		TabView.h		\
		TextView.cpp		\
		TextView.h		\
		Bitmap.cpp		\
		Bitmap.h		\
		LimitedView.cpp		\
		LimitedView.h		\
		ToolTip.cpp		\
		ToolTip.h		\
		Input.h

libetk_interface_la_LIBADD = $(graphics_la)
libetk_interface_la_DEPENDENCIES = $(graphics_la)

etk_interfaceincludedir = $(includedir)/etkxx/etk/interface
etk_interfaceinclude_HEADERS =		\
		GraphicsDefs.h		\
		InterfaceDefs.h		\
		Point.h			\
		Rect.h			\
		Polygon.h		\
		Region.h		\
		Font.h			\
		Screen.h		\
		Window.h		\
		View.h			\
		ScrollBar.h		\
		ScrollView.h		\
		Control.h		\
		ColorControl.h		\
		Button.h		\
		Alert.h			\
		Menu.h			\
		MenuItem.h		\
		PopUpMenu.h		\
		MenuBar.h		\
		MenuField.h		\
		TextEditable.h		\
		TextControl.h		\
		StringView.h		\
		Box.h			\
		StatusBar.h		\
		CheckBox.h		\
		RadioButton.h		\
		ListItem.h		\
		ListView.h		\
		OutlineListView.h	\
		TabView.h		\
		TextView.h		\
		Bitmap.h		\
		LimitedView.h		\
		ToolTip.h		\
		Input.h

DISTCLEANFILES = Makefile.in


