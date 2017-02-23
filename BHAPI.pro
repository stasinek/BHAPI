#-------------------------------------------------
#
# Project created by QtCreator 2017-02-05T15:33:45
#
#-------------------------------------------------

TARGET = be
TEMPLATE = lib
DEFINES += BHAPI_LIBRARY
DEFINES += BHAPI_BUILD_LIBRARY
DEFINES += BHAPI_OS_WIN32
DEFINES += BHAPI_GRAPHICS_WIN32_BUILT_IN

CONFIG += windows shared
CONFIG -= app_bundle
CONFIG -= qt
INCLUDEPATH += ..
INCLUDEPATH += ../BHAPI/src ../freetype/include
QT -= core gui

SOURCES += bhapi.cpp \
    src/kits/add-ons/font/FontEngine.cpp \
    src/kits/add-ons/font/FreeType2.cpp \
    src/kits/add-ons/graphics/GraphicsEngine.cpp \
    src/kits/add-ons/theme/DefaultTheme.cpp \
    src/kits/add-ons/theme/ThemeEngine.cpp \
    src/kits/private/Memory.cpp \
    src/kits/private/MessageBody.cpp \
    src/kits/private/Object.cpp \
    src/kits/private/PrivateApplication.cpp \
    src/kits/private/PrivateHandler.cpp \
    src/kits/private/StandardIO.cpp \
    src/kits/private/Token.cpp \
    src/kits/render/ArcGenerator.cpp \
    src/kits/render/LineGenerator.cpp \
    src/kits/render/Pixmap.cpp \
    src/kits/render/Render.cpp \
    src/kits/storage/Directory.cpp \
    src/kits/storage/Entry.cpp \
    src/kits/storage/File.cpp \
    src/kits/storage/FilePanel.cpp \
    src/kits/storage/FindDirectory.cpp \
    src/kits/storage/Node.cpp \
    src/kits/storage/Path.cpp \
    src/kits/storage/Volume.cpp \
    src/kits/storage/VolumeRoster.cpp \
    src/kits/support/Archivable.cpp \
    src/kits/support/DataIO.cpp \
    src/kits/support/Flattenable.cpp \
    src/kits/support/Locker.cpp \
    src/kits/support/StringMe.cpp \
    src/kits/support/List.cpp \
    src/kits/support/SimpleLocker.cpp \
    src/kits/support/StreamIO.cpp \
    src/kits/support/StringArray.cpp \
    src/kits/support/ByteOrder.c \
    src/kits/app/Application.cpp \
    src/kits/app/Clipboard.cpp \
    src/kits/app/Cursor.cpp \
    src/kits/app/Handler.cpp \
    src/kits/app/Invoker.cpp \
    src/kits/app/Looper.cpp \
    src/kits/app/Message.cpp \
    src/kits/app/MessageFilter.cpp \
    src/kits/app/MessageQueue.cpp \
    src/kits/app/MessageRunner.cpp \
    src/kits/app/Messenger.cpp \
    src/kits/kernel/Debug.cpp \
    src/kits/kernel/etk-port.cpp \
    src/kits/net/NetAddress.cpp \
    src/kits/net/NetBuffer.cpp \
    src/kits/net/NetDebug.cpp \
    src/kits/net/NetEndpoint.cpp \
    src/kits/xml/SimpleXmlParser.cpp \
    src/kits/interface/Alert.cpp \
    src/kits/interface/Bitmap.cpp \
    src/kits/interface/Box.cpp \
    src/kits/interface/Button.cpp \
    src/kits/interface/CheckBox.cpp \
    src/kits/interface/ColorControl.cpp \
    src/kits/interface/Control.cpp \
    src/kits/interface/Font.cpp \
    src/kits/interface/GraphicsDefs.cpp \
    src/kits/interface/LimitedView.cpp \
    src/kits/interface/ListItem.cpp \
    src/kits/interface/ListView.cpp \
    src/kits/interface/Menu.cpp \
    src/kits/interface/MenuBar.cpp \
    src/kits/interface/MenuField.cpp \
    src/kits/interface/MenuItem.cpp \
    src/kits/interface/OutlineListView.cpp \
    src/kits/interface/Point.cpp \
    src/kits/interface/Polygon.cpp \
    src/kits/interface/PopUpMenu.cpp \
    src/kits/interface/RadioButton.cpp \
    src/kits/interface/Rect.cpp \
    src/kits/interface/Region.cpp \
    src/kits/interface/Screen.cpp \
    src/kits/interface/ScrollBar.cpp \
    src/kits/interface/ScrollView.cpp \
    src/kits/interface/StatusBar.cpp \
    src/kits/interface/StringView.cpp \
    src/kits/interface/TabView.cpp \
    src/kits/interface/TextControl.cpp \
    src/kits/interface/TextEditable.cpp \
    src/kits/interface/TextView.cpp \
    src/kits/interface/Theme.cpp \
    src/kits/interface/ToolTip.cpp \
    src/kits/interface/View.cpp \
    src/kits/interface/Window.cpp \
    src/kits/interface/layout/LayoutContainer.cpp \
    src/kits/interface/layout/LayoutForm.cpp \
    src/kits/interface/layout/LayoutItem.cpp


HEADERS += bhapi.h\
    bhapi_global.h \
    src/kits/add-ons/font/FontEngine.h \
    src/kits/add-ons/graphics/GraphicsEngine.h \
    src/kits/add-ons/theme/ThemeEngine.h \
    src/kits/app/AppDefs.h \
    src/kits/app/Application.h \
    src/kits/app/Clipboard.h \
    src/kits/app/Cursor.h \
    src/kits/app/Handler.h \
    src/kits/app/Invoker.h \
    src/kits/app/Looper.h \
    src/kits/app/Message.h \
    src/kits/app/MessageFilter.h \
    src/kits/app/MessageQueue.h \
    src/kits/app/MessageRunner.h \
    src/kits/app/Messenger.h \
    src/kits/interface/layout/Layout.h \
    src/kits/interface/Alert.h \
    src/kits/interface/Bitmap.h \
    src/kits/interface/Box.h \
    src/kits/interface/Button.h \
    src/kits/interface/CheckBox.h \
    src/kits/interface/ColorControl.h \
    src/kits/interface/Control.h \
    src/kits/interface/Font.h \
    src/kits/interface/GraphicsDefs.h \
    src/kits/interface/Input.h \
    src/kits/interface/InterfaceDefs.h \
    src/kits/interface/LimitedView.h \
    src/kits/interface/ListItem.h \
    src/kits/interface/ListView.h \
    src/kits/interface/Menu.h \
    src/kits/interface/MenuBar.h \
    src/kits/interface/MenuField.h \
    src/kits/interface/MenuItem.h \
    src/kits/interface/OutlineListView.h \
    src/kits/interface/Point.h \
    src/kits/interface/Polygon.h \
    src/kits/interface/PopUpMenu.h \
    src/kits/interface/RadioButton.h \
    src/kits/interface/Rect.h \
    src/kits/interface/Region.h \
    src/kits/interface/Screen.h \
    src/kits/interface/ScrollBar.h \
    src/kits/interface/ScrollView.h \
    src/kits/interface/StatusBar.h \
    src/kits/interface/StringView.h \
    src/kits/interface/TabView.h \
    src/kits/interface/TextControl.h \
    src/kits/interface/TextEditable.h \
    src/kits/interface/TextView.h \
    src/kits/interface/ToolTip.h \
    src/kits/interface/View.h \
    src/kits/interface/ViewPrivate.h \
    src/kits/interface/Window.h \
    src/kits/kernel/Debug.h \
    src/kits/kernel/Kernel.h \
    src/kits/kernel/OS.h \
    src/kits/net/NetAddress.h \
    src/kits/net/NetBuffer.h \
    src/kits/net/NetDebug.h \
    src/kits/net/NetEndpoint.h \
    src/kits/private/Memory.h \
    src/kits/private/MessageBody.h \
    src/kits/private/Object.h \
    src/kits/private/PrivateApplication.h \
    src/kits/private/PrivateHandler.h \
    src/kits/private/StandardIO.h \
    src/kits/private/Token.h \
    src/kits/render/ArcGenerator.h \
    src/kits/render/LineGenerator.h \
    src/kits/render/Pixmap.h \
    src/kits/render/Render.h \
    src/kits/storage/Directory.h \
    src/kits/storage/Entry.h \
    src/kits/storage/File.h \
    src/kits/storage/FilePanel.h \
    src/kits/storage/FindDirectory.h \
    src/kits/storage/Node.h \
    src/kits/storage/Path.h \
    src/kits/storage/StorageDefs.h \
    src/kits/storage/Volume.h \
    src/kits/storage/VolumeRoster.h \
    src/kits/support/Archivable.h \
    src/kits/support/Autolock.h \
    src/kits/support/ByteOrder.h \
    src/kits/support/List.h \
    src/kits/support/ClassInfo.h \
    src/kits/support/DataIO.h \
    src/kits/support/Errors.h \
    src/kits/support/StringMe.h \
    src/kits/support/Flattenable.h \
    src/kits/support/Locker.h \
    src/kits/support/SimpleLocker.h \
    src/kits/support/StreamIO.h \
    src/kits/support/StringArray.h \
    src/kits/support/SupportDefs.h \
    src/kits/xml/SimpleXmlParser.h \
    src/kits/BE.h \
    src/kits/AppKit.h \
    src/kits/InterfaceKit.h \
    src/kits/KernelKit.h \
    src/kits/NetKit.h \
    src/kits/RenderKit.h \
    src/kits/StorageKit.h \
    src/kits/SupportKit.h \
    src/kits/XmlKit.h

LIBS -= -lfreetype
LIBS += -L"../freetype/lib/debug/libfreetype.a"

contains(QMAKE_COMPILER_DEFINES, BEOS) {
SOURCES +=  src/kits/interface/beos/etk-application.cpp \
    src/kits/interface/beos/etk-beos-font.cpp \
    src/kits/interface/beos/etk-drawing.cpp \
    src/kits/interface/beos/etk-pixmap.cpp \
    src/kits/interface/beos/etk-window.cpp \
    src/kits/kernel/beos/etk-area.cpp \
    src/kits/kernel/beos/etk-image.cpp \
    src/kits/kernel/beos/etk-os.cpp \
    src/kits/kernel/beos/etk-timefuncs.cpp \
    src/kits/kernel/thread/beos/etk-locker.cpp \
    src/kits/kernel/thread/beos/etk-semaphore.cpp \
    src/kits/kernel/thread/beos/etk-thread.cpp

HEADERS += src/kits/interface/beos/etk-beos-graphics.h
}

contains(QMAKE_COMPILER_DEFINES, WIN32) {
SOURCES += src/kits/interface/win32/etk-application.cpp \
    src/kits/interface/win32/etk-drawing.cpp \
    src/kits/interface/win32/etk-pixmap.cpp \
    src/kits/interface/win32/etk-win32-font.cpp \
    src/kits/interface/win32/etk-window.cpp \
    src/kits/kernel/thread/win32/etk-locker.cpp \
    src/kits/kernel/thread/win32/etk-semaphore.cpp \
    src/kits/kernel/thread/win32/etk-thread.cpp \
    src/kits/kernel/win32/etk-area.cpp \
    src/kits/kernel/win32/etk-image.cpp \
    src/kits/kernel/win32/etk-os.cpp \
    src/kits/kernel/win32/etk-timefuncs.cpp

HEADERS += src/kits/interface/win32/etk-win32gdi.h

LIBS += -lkernel32 -lgdi32 -lshell32 -luser32 -lcomctl32 -luserenv
LIBS += -lws2_32 -lwsock32 -lwinmm -limm32 -lole32 -loleaut32
}

contains(QMAKE_COMPILER_DEFINES, LINUX) {
SOURCES += src/kits/interface/x11/etk-application.cpp \
    src/kits/interface/x11/etk-drawing.cpp \
    src/kits/interface/x11/etk-pixmap.cpp \
    src/kits/interface/x11/etk-window.cpp \
    src/kits/interface/x11/etk-x11-font.cpp \
    src/kits/kernel/thread/posix/etk-locker.cpp \
    src/kits/kernel/thread/posix/etk-semaphore-mach.cpp \
    src/kits/kernel/thread/posix/etk-semaphore-umtx.cpp \
    src/kits/kernel/thread/posix/etk-semaphore.cpp \
    src/kits/kernel/thread/posix/etk-thread.cpp \
    src/kits/kernel/unix/etk-area.cpp \
    src/kits/kernel/unix/etk-image.cpp \
    src/kits/kernel/unix/etk-os.cpp \
    src/kits/kernel/unix/etk-timefuncs.cpp

HEADERS +=     src/kits/interface/x11/etk-x11.h
}

contains(QMAKE_COMPILER_DEFINES, DIRECTFB) {
SOURCES +=    src/kits/interface/directfb/etk-application.cpp \
    src/kits/interface/directfb/etk-dfb-font.cpp \
    src/kits/interface/directfb/etk-drawing.cpp \
    src/kits/interface/directfb/etk-pixmap.cpp \
    src/kits/interface/directfb/etk-window.cpp \
    src/kits/interface/directfb/etk-wm.cpp

HEADERS += src/kits/interface/directfb/etk-dfb.h
}

contains(QMAKE_COMPILER_DEFINES, MACOS) {
HEADERS += src/kits/interface/carbon/etk-carbon.h
}

contains(QMAKE_COMPILER_DEFINES, __GNUC__) {
QMAKE_CXXFLAGS += -Wno-write-strings -Wno-multichar
QMAKE_CFLAGS   += -Wno-write-strings -Wno-multichar
QMAKE_CXXFLAGS += -Wno-unused-variable -Wno-unused-parameter -Wno-unused-value -Wno-unused-label
QMAKE_CFLAGS   += -Wno-unused-variable -Wno-unused-parameter -Wno-unused-value -Wno-unused-label
QMAKE_CXXFLAGS += -Wno-unused-but-set-variable
QMAKE_CFLAGS   += -Wno-unused-but-set-variable
QMAKE_CXXFLAGS += -Wattributes -Winline -Wshadow -Wall
QMAKE_CFLAGS   += -Wattributes -Winline -Wshadow -Wall
QMAKE_CXXFLAGS += -Wunknown-pragmas
QMAKE_CFLAGS   += -Wunknown-pragmas

QMAKE_CXXFLAGS -= -pipe
QMAKE_CXXFLAGS += -save-temps
QMAKE_CXXFLAGS += -fverbose-asm
QMAKE_CXXFLAGS += -fstrict-aliasing
QMAKE_CXXFLAGS += -dD
QMAKE_CXXFLAGS += -g
QMAKE_CXXFLAGS += -std=gnu++11 -pthread
QMAKE_CXXFLAGS += -malign-double
QMAKE_CXXFLAGS += -momit-leaf-frame-pointer
QMAKE_CXXFLAGS += -fwrapv
QMAKE_CXXFLAGS += -funroll-loops
QMAKE_CXXFLAGS += -m32 -mfpmath=sse -flto #-O1
QMAKE_CXXFLAGS += -mpreferred-stack-boundary=8
QMAKE_CXXFLAGS += -mmmx -msse -msse2 #-msse3

#QMAKE_CXXFLAGS += -fno-leading-underscore
#QMAKE_CFLAGS   += -fno-leading-underscore
}

contains(QMAKE_COMPILER_DEFINES, __clang__) {
QMAKE_CXXFLAGS += -Wno-write-strings -Wno-multichar
QMAKE_CFLAGS   += -Wno-write-strings -Wno-multichar
QMAKE_CXXFLAGS += -Wno-unused-variable -Wno-unused-parameter -Wno-unused-value -Wno-unused-label
QMAKE_CFLAGS   += -Wno-unused-variable -Wno-unused-parameter -Wno-unused-value -Wno-unused-label
QMAKE_CXXFLAGS += -Wno-unused-but-set-variable
QMAKE_CFLAGS   += -Wno-unused-but-set-variable
QMAKE_CXXFLAGS += -Wattributes -Winline -Wshadow -Wall
QMAKE_CFLAGS   += -Wattributes -Winline -Wshadow -Wall
QMAKE_CXXFLAGS += -Wunknown-pragmas
QMAKE_CFLAGS   += -Wunknown-pragmas

QMAKE_CXXFLAGS += -save-temps
QMAKE_CXXFLAGS += -fverbose-asm
QMAKE_CXXFLAGS += -fstrict-aliasing
QMAKE_CXXFLAGS += -dD
QMAKE_CXXFLAGS += -g
QMAKE_CXXFLAGS += -std=gnu++0x -pthread
QMAKE_CXXFLAGS += -malign-double
QMAKE_CXXFLAGS += -momit-leaf-frame-pointer
QMAKE_CXXFLAGS += -fwrapv
QMAKE_CXXFLAGS += -funroll-loops
QMAKE_CXXFLAGS += -m32 --32 -mfpmath=sse -flto #-O1
QMAKE_CXXFLAGS += -mpreferred-stack-boundary=8
QMAKE_CXXFLAGS += -mmmx -msse -msse2 #-msse3
QMAKE_CXXFLAGS += -Qunused-arguments -Wno-error=unused-command-line-argument-hard-error-in-future
QMAKE_CXXFLAGS -= -fno-keep-inline-dllexport
QMAKE_CXXFLAGS -= -finline-small-functions
QMAKE_CXXFLAGS -= -pipe

QMAKE_LFLAGS += -Qunused-arguments -Wno-error=unused-command-line-argument-hard-error-in-future
QMAKE_LFLAGS -= -mthreads
}

unix {
    target.path = /usr/lib
    INSTALLS += target
}
