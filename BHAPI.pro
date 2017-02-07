#-------------------------------------------------
#
# Project created by QtCreator 2017-02-05T15:33:45
#
#-------------------------------------------------

QT       -= core gui

TARGET = BHAPI
TEMPLATE = lib

DEFINES += BHAPI_LIBRARY

SOURCES += bhapi.cpp \
    src/add-ons/font/FontEngine.cpp \
    src/add-ons/font/FreeType2.cpp \
    src/add-ons/graphics/GraphicsEngine.cpp \
    src/add-ons/theme/DefaultTheme.cpp \
    src/add-ons/theme/ThemeEngine.cpp \
    src/app/Application.cpp \
    src/app/Clipboard.cpp \
    src/app/Cursor.cpp \
    src/app/Handler.cpp \
    src/app/Invoker.cpp \
    src/app/Looper.cpp \
    src/app/Message.cpp \
    src/app/MessageFilter.cpp \
    src/app/MessageQueue.cpp \
    src/app/MessageRunner.cpp \
    src/app/Messenger.cpp \
    src/interface/Alert.cpp \
    src/interface/Bitmap.cpp \
    src/interface/Box.cpp \
    src/interface/Button.cpp \
    src/interface/CheckBox.cpp \
    src/interface/ColorControl.cpp \
    src/interface/Control.cpp \
    src/interface/Font.cpp \
    src/interface/GraphicsDefs.cpp \
    src/interface/LimitedView.cpp \
    src/interface/ListItem.cpp \
    src/interface/ListView.cpp \
    src/interface/Menu.cpp \
    src/interface/MenuBar.cpp \
    src/interface/MenuField.cpp \
    src/interface/MenuItem.cpp \
    src/interface/OutlineListView.cpp \
    src/interface/Point.cpp \
    src/interface/Polygon.cpp \
    src/interface/PopUpMenu.cpp \
    src/interface/RadioButton.cpp \
    src/interface/Rect.cpp \
    src/interface/Region.cpp \
    src/interface/Screen.cpp \
    src/interface/ScrollBar.cpp \
    src/interface/ScrollView.cpp \
    src/interface/StatusBar.cpp \
    src/interface/StringView.cpp \
    src/interface/TabView.cpp \
    src/interface/TextControl.cpp \
    src/interface/TextEditable.cpp \
    src/interface/TextView.cpp \
    src/interface/Theme.cpp \
    src/interface/ToolTip.cpp \
    src/interface/View.cpp \
    src/interface/Window.cpp \
    src/kernel/Debug.cpp \
    src/kernel/etk-port.cpp \
    src/net/NetAddress.cpp \
    src/net/NetBuffer.cpp \
    src/net/NetDebug.cpp \
    src/net/NetEndpoint.cpp \
    src/private/Memory.cpp \
    src/private/MessageBody.cpp \
    src/private/Object.cpp \
    src/private/PrivateApplication.cpp \
    src/private/PrivateHandler.cpp \
    src/private/StandardIO.cpp \
    src/private/Token.cpp \
    src/render/ArcGenerator.cpp \
    src/render/LineGenerator.cpp \
    src/render/Pixmap.cpp \
    src/render/Render.cpp \
    src/storage/Directory.cpp \
    src/storage/Entry.cpp \
    src/storage/File.cpp \
    src/storage/FilePanel.cpp \
    src/storage/FindDirectory.cpp \
    src/storage/Node.cpp \
    src/storage/Path.cpp \
    src/storage/Volume.cpp \
    src/storage/VolumeRoster.cpp \
    src/support/Archivable.cpp \
    src/support/DataIO.cpp \
    src/support/Flattenable.cpp \
    src/support/List.cpp \
    src/support/Locker.cpp \
    src/support/SimpleLocker.cpp \
    src/support/StreamIO.cpp \
    src/support/StringArray.cpp \
    src/xml/SimpleXmlParser.cpp \
    tools/etkxx-doc.cpp \
    src/support/ByteOrder.c \
    src/interface/layout/LayoutContainer.cpp \
    src/interface/layout/LayoutForm.cpp \
    src/interface/layout/LayoutItem.cpp \
    src/support/StringMe.cpp

contains(QMAKE_COMPILER_DEFINES, BEOS) {
SOURCES +=  src/interface/beos/etk-application.cpp \
    src/interface/beos/etk-beos-font.cpp \
    src/interface/beos/etk-drawing.cpp \
    src/interface/beos/etk-pixmap.cpp \
    src/interface/beos/etk-window.cpp \
    src/kernel/beos/etk-area.cpp \
    src/kernel/beos/etk-image.cpp \
    src/kernel/beos/etk-os.cpp \
    src/kernel/beos/etk-timefuncs.cpp \
    src/kernel/thread/beos/etk-locker.cpp \
    src/kernel/thread/beos/etk-semaphore.cpp \
    src/kernel/thread/beos/etk-thread.cpp

HEADERS += src/interface/beos/etk-beos-graphics.h
}

contains(QMAKE_COMPILER_DEFINES, WIN32) {
SOURCES += src/interface/win32/etk-application.cpp \
    src/interface/win32/etk-drawing.cpp \
    src/interface/win32/etk-pixmap.cpp \
    src/interface/win32/etk-win32-font.cpp \
    src/interface/win32/etk-window.cpp \
    src/kernel/thread/win32/etk-locker.cpp \
    src/kernel/thread/win32/etk-semaphore.cpp \
    src/kernel/thread/win32/etk-thread.cpp \
    src/kernel/win32/etk-area.cpp \
    src/kernel/win32/etk-image.cpp \
    src/kernel/win32/etk-os.cpp \
    src/kernel/win32/etk-timefuncs.cpp

HEADERS += src/interface/win32/etk-win32gdi.h
}

contains(QMAKE_COMPILER_DEFINES, LINUX) {
SOURCES += src/interface/x11/etk-application.cpp \
    src/interface/x11/etk-drawing.cpp \
    src/interface/x11/etk-pixmap.cpp \
    src/interface/x11/etk-window.cpp \
    src/interface/x11/etk-x11-font.cpp \
    src/kernel/thread/posix/etk-locker.cpp \
    src/kernel/thread/posix/etk-semaphore-mach.cpp \
    src/kernel/thread/posix/etk-semaphore-umtx.cpp \
    src/kernel/thread/posix/etk-semaphore.cpp \
    src/kernel/thread/posix/etk-thread.cpp \
    src/kernel/unix/etk-area.cpp \
    src/kernel/unix/etk-image.cpp \
    src/kernel/unix/etk-os.cpp \
    src/kernel/unix/etk-timefuncs.cpp

HEADERS +=     src/interface/x11/etk-x11.h
}
contains(QMAKE_COMPILER_DEFINES, DIRECTFB) {
SOURCES +=    src/interface/directfb/etk-application.cpp \
    src/interface/directfb/etk-dfb-font.cpp \
    src/interface/directfb/etk-drawing.cpp \
    src/interface/directfb/etk-pixmap.cpp \
    src/interface/directfb/etk-window.cpp \
    src/interface/directfb/etk-wm.cpp

HEADERS += src/interface/directfb/etk-dfb.h
}


contains(QMAKE_COMPILER_DEFINES, MACOS) {
HEADERS += src/interface/carbon/etk-carbon.h
}

SOURCES += src/interface/wm/SimpleWM.cpp
HEADERS += src/interface/wm/SimpleWM.h \
    src/support/StringMe.h

HEADERS += bhapi.h\
    bhapi_global.h \
    src/add-ons/font/FontEngine.h \
    src/add-ons/graphics/GraphicsEngine.h \
    src/add-ons/theme/ThemeEngine.h \
    src/app/AppDefs.h \
    src/app/Application.h \
    src/app/Clipboard.h \
    src/app/Cursor.h \
    src/app/Handler.h \
    src/app/Invoker.h \
    src/app/Looper.h \
    src/app/Message.h \
    src/app/MessageFilter.h \
    src/app/MessageQueue.h \
    src/app/MessageRunner.h \
    src/app/Messenger.h \
    src/interface/layout/Layout.h \
    src/interface/Alert.h \
    src/interface/Bitmap.h \
    src/interface/Box.h \
    src/interface/Button.h \
    src/interface/CheckBox.h \
    src/interface/ColorControl.h \
    src/interface/Control.h \
    src/interface/Font.h \
    src/interface/GraphicsDefs.h \
    src/interface/Input.h \
    src/interface/InterfaceDefs.h \
    src/interface/LimitedView.h \
    src/interface/ListItem.h \
    src/interface/ListView.h \
    src/interface/Menu.h \
    src/interface/MenuBar.h \
    src/interface/MenuField.h \
    src/interface/MenuItem.h \
    src/interface/OutlineListView.h \
    src/interface/Point.h \
    src/interface/Polygon.h \
    src/interface/PopUpMenu.h \
    src/interface/RadioButton.h \
    src/interface/Rect.h \
    src/interface/Region.h \
    src/interface/Screen.h \
    src/interface/ScrollBar.h \
    src/interface/ScrollView.h \
    src/interface/StatusBar.h \
    src/interface/StringView.h \
    src/interface/TabView.h \
    src/interface/TextControl.h \
    src/interface/TextEditable.h \
    src/interface/TextView.h \
    src/interface/ToolTip.h \
    src/interface/View.h \
    src/interface/ViewPrivate.h \
    src/interface/Window.h \
    src/kernel/Debug.h \
    src/kernel/Kernel.h \
    src/kernel/OS.h \
    src/net/NetAddress.h \
    src/net/NetBuffer.h \
    src/net/NetDebug.h \
    src/net/NetEndpoint.h \
    src/private/Memory.h \
    src/private/MessageBody.h \
    src/private/Object.h \
    src/private/PrivateApplication.h \
    src/private/PrivateHandler.h \
    src/private/StandardIO.h \
    src/private/Token.h \
    src/render/ArcGenerator.h \
    src/render/LineGenerator.h \
    src/render/Pixmap.h \
    src/render/Render.h \
    src/storage/Directory.h \
    src/storage/Entry.h \
    src/storage/File.h \
    src/storage/FilePanel.h \
    src/storage/FindDirectory.h \
    src/storage/Node.h \
    src/storage/Path.h \
    src/storage/StorageDefs.h \
    src/storage/Volume.h \
    src/storage/VolumeRoster.h \
    src/support/Archivable.h \
    src/support/Autolock.h \
    src/support/ByteOrder.h \
    src/support/ClassInfo.h \
    src/support/DataIO.h \
    src/support/Errors.h \
    src/support/Flattenable.h \
    src/support/List.h \
    src/support/Locker.h \
    src/support/SimpleLocker.h \
    src/support/StreamIO.h \
    src/support/StringArray.h \
    src/support/SupportDefs.h \
    src/xml/SimpleXmlParser.h \
    src/AppKit.h \
    src/BE.h \
    src/InterfaceKit.h \
    src/KernelKit.h \
    src/NetKit.h \
    src/RenderKit.h \
    src/StorageKit.h \
    src/SupportKit.h \
    src/XmlKit.h

contains(QMAKE_COMPILER_DEFINES, __GNUC__) {
QMAKE_CXXFLAGS -= -pipe
QMAKE_CXXFLAGS += -save-temps
QMAKE_CXXFLAGS += -Wno-write-strings
QMAKE_CXXFLAGS += -Wno-unused-variable
QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-unused-label
QMAKE_CXXFLAGS += -Wno-unused-value
QMAKE_CXXFLAGS += -Wunknown-pragmas
QMAKE_CXXFLAGS += -Wattributes
QMAKE_CXXFLAGS += -fverbose-asm
QMAKE_CXXFLAGS += -fstrict-aliasing
QMAKE_CXXFLAGS += -dD
QMAKE_CXXFLAGS += -g
QMAKE_CXXFLAGS += -std=gnu++0x -pthread
QMAKE_CXXFLAGS += -Wno-multichar

QMAKE_CXXFLAGS += -Winline
QMAKE_CXXFLAGS += -Wshadow
QMAKE_CXXFLAGS += -Wall
QMAKE_CXXFLAGS += -malign-double
QMAKE_CXXFLAGS += -momit-leaf-frame-pointer
QMAKE_CXXFLAGS += -fwrapv
QMAKE_CXXFLAGS += -funroll-loops
QMAKE_CXXFLAGS += -m32 -mfpmath=sse -flto #-O1
QMAKE_CXXFLAGS += -mpreferred-stack-boundary=8
QMAKE_CXXFLAGS += -mmmx -msse -msse2 #-msse3

}

contains(QMAKE_COMPILER_DEFINES, __clang__) {
QMAKE_CXXFLAGS += -save-temps
QMAKE_CXXFLAGS += -Wno-write-strings
QMAKE_CXXFLAGS += -Wno-unused-variable
QMAKE_CXXFLAGS += -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-unused-label
QMAKE_CXXFLAGS += -Wno-unused-value
QMAKE_CXXFLAGS += -Wunknown-pragmas
QMAKE_CXXFLAGS += -Wattributes
QMAKE_CXXFLAGS += -fverbose-asm
QMAKE_CXXFLAGS += -fstrict-aliasing
QMAKE_CXXFLAGS += -dD
QMAKE_CXXFLAGS += -g
QMAKE_CXXFLAGS += -std=gnu++0x -pthread
QMAKE_CXXFLAGS += -Wno-multichar

QMAKE_CXXFLAGS += -Winline
QMAKE_CXXFLAGS += -Wshadow
QMAKE_CXXFLAGS += -Wall
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

INCLUDEPATH += ../../freetype/include
