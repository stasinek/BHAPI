# Microsoft Developer Studio Project File - Name="etkxx" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=etkxx - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "etkxx.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "etkxx.mak" CFG="etkxx - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "etkxx - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "etkxx - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "etkxx - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ETKXX_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GR /GX /O2 /I "$(OutDir)\..\..\..\..\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ETK_COMPILATION" /D "ETK_DISABLE_MORE_CHECKS" /D "ETK_DISABLE_CHECKS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 msvcrt.lib kernel32.lib user32.lib gdi32.lib advapi32.lib imm32.lib ws2_32.lib /nologo /dll /machine:I386 /nodefaultlib /out:"Release/libetkxx.dll"

!ELSEIF  "$(CFG)" == "etkxx - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ETKXX_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GR /GX /ZI /Od /I "$(OutDir)\..\..\..\..\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ETK_COMPILATION" /D "ETK_ENABLE_DEBUG" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 msvcrtd.lib kernel32.lib user32.lib gdi32.lib advapi32.lib imm32.lib ws2_32.lib /nologo /dll /debug /machine:I386 /nodefaultlib /out:"Debug/libetkxx.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "etkxx - Win32 Release"
# Name "etkxx - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "interface"

# PROP Default_Filter ""
# Begin Group "built-in graphics"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\etk\interface\win32\BHAPI_wrapper_application.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\..\etk\interface\win32\BHAPI_wrapper_drawing.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\..\etk\interface\win32\BHAPI_wrapper_pixmap.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\..\etk\interface\win32\BHAPI_wrapper_win32-font.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\..\etk\interface\win32\BHAPI_wrapper_window.cpp"
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\etk\interface\Alert.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Bitmap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Box.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Button.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\CheckBox.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\ColorControl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Control.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Font.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\GraphicsDefs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\layout\LayoutContainer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\layout\LayoutForm.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\layout\LayoutItem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\LimitedView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\ListItem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\ListView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Menu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\MenuBar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\MenuField.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\MenuItem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\OutlineListView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Point.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Polygon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\PopUpMenu.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\RadioButton.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Rect.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Region.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Screen.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\ScrollBar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\ScrollView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\StatusBar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\StringView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\TabView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\TextControl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\TextEditable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\TextView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Theme.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\View.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\ToolTip.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Window.cpp
# End Source File
# End Group
# Begin Group "private"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\etk\private\PrivateApplication.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\private\PrivateHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\private\Memory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\private\Object.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\private\Token.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\private\StandardIO.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\private\MessageBody.cpp
# End Source File
# End Group
# Begin Group "support"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\etk\support\Archivable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\ByteOrder.c
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\DataIO.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\StreamIO.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\List.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\Locker.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\SimpleLocker.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\String.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\StringArray.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\Flattenable.cpp
# End Source File
# End Group
# Begin Group "kernel"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\etk\kernel\Debug.cpp
# End Source File
# Begin Source File

SOURCE="..\..\..\etk\kernel\win32\BHAPI_wrapper_area.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\..\etk\kernel\win32\BHAPI_wrapper_image.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\..\etk\kernel\thread\win32\BHAPI_wrapper_locker.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\..\etk\kernel\win32\BHAPI_wrapper_os.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\..\etk\kernel\BHAPI_wrapper_port.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\..\etk\kernel\thread\win32\BHAPI_wrapper_semaphore.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\..\etk\kernel\thread\win32\BHAPI_wrapper_thread.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\..\etk\kernel\win32\BHAPI_wrapper_timefuncs.cpp"
# End Source File
# End Group
# Begin Group "render"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\etk\render\ArcGenerator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\render\LineGenerator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\render\Pixmap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\render\Render.cpp
# End Source File
# End Group
# Begin Group "storage"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\etk\storage\Directory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\storage\Entry.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\storage\File.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\storage\FilePanel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\storage\FindDirectory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\storage\Node.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\storage\Path.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\storage\Volume.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\storage\VolumeRoster.cpp
# End Source File
# End Group
# Begin Group "net"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\etk\net\NetAddress.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\net\NetBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\net\NetDebug.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\net\NetEndpoint.cpp
# End Source File
# End Group
# Begin Group "xml"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\etk\xml\SimpleXmlParser.cpp
# End Source File
# End Group
# Begin Group "app"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\etk\app\Application.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\Clipboard.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\Cursor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\Handler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\Invoker.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\Looper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\Message.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\MessageFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\MessageQueue.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\MessageRunner.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\Messenger.cpp
# End Source File
# End Group
# Begin Group "add-ons"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\etk\add-ons\theme\DefaultTheme.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\..\etk\add-ons\font\FontEngine.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\..\etk\add-ons\graphics\GraphicsEngine.cpp"
# End Source File
# Begin Source File

SOURCE="..\..\..\etk\add-ons\theme\ThemeEngine.cpp"
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\etk\ETKBuild.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Private"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\etk\private\Token.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\private\Memory.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\private\Object.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\private\PrivateApplication.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\private\PrivateHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\private\StandardIO.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\private\MessageBody.h
# End Source File
# End Group
# Begin Group "SupportKit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\etk\support\Archivable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\Autolock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\ByteOrder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\ClassInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\DataIO.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\StreamIO.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\Errors.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\List.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\Locker.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\SimpleLocker.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\String.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\StringArray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\Flattenable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\support\SupportDefs.h
# End Source File
# End Group
# Begin Group "KernelKit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\etk\kernel\Debug.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\kernel\Kernel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\kernel\OS.h
# End Source File
# End Group
# Begin Group "StorageKit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\etk\storage\Directory.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\storage\Entry.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\storage\File.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\storage\FilePanel.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\storage\FindDirectory.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\storage\Node.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\storage\Path.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\storage\StorageDefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\storage\Volume.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\storage\VolumeRoster.h
# End Source File
# End Group
# Begin Group "NetKit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\etk\net\NetAddress.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\net\NetBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\net\NetDebug.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\net\NetEndpoint.h
# End Source File
# End Group
# Begin Group "AppKit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\etk\app\AppDefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\Application.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\Clipboard.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\Cursor.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\Handler.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\Invoker.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\Looper.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\Message.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\MessageFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\MessageQueue.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\MessageRunner.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\app\Messenger.h
# End Source File
# End Group
# Begin Group "XmlKit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\etk\xml\SimpleXmlParser.h
# End Source File
# End Group
# Begin Group "InterfaceKit"

# PROP Default_Filter ""
# Begin Group "Private"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\etk\interface\win32\BHAPI_wrapper_win32gdi.h"
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\ViewPrivate.h
# End Source File
# End Group
# Begin Group "Layout"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\etk\interface\layout\Layout.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\etk\interface\Alert.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Bitmap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Box.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Button.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\CheckBox.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\ColorControl.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Control.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Font.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\GraphicsDefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Input.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\InterfaceDefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\LimitedView.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\ListItem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\ListView.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Menu.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\MenuBar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\MenuField.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\MenuItem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\OutlineListView.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Point.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Polygon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\PopUpMenu.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\RadioButton.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Rect.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Region.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Screen.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\ScrollBar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\ScrollView.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\StatusBar.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\StringView.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\TabView.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\TextControl.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\TextEditable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\TextView.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\ToolTip.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\View.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\interface\Window.h
# End Source File
# End Group
# Begin Group "RenderKit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\etk\render\ArcGenerator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\render\LineGenerator.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\render\Pixmap.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\render\Render.h
# End Source File
# End Group
# Begin Group "Addons"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\etk\add-ons\font\FontEngine.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\etk\add-ons\graphics\GraphicsEngine.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\etk\add-ons\theme\ThemeEngine.h"
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\etk\AppKit.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\config.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\ETK.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\ETKBuild.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etkxx.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\InterfaceKit.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\KernelKit.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\NetKit.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\RenderKit.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\StorageKit.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\SupportKit.h
# End Source File
# Begin Source File

SOURCE=..\..\..\etk\XmlKit.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\..\etk\etkxx.rc
# End Source File
# End Group
# End Target
# End Project
