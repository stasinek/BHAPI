#-------------------------------------------------
#
# Project created by QtCreator 2017-02-05T15:33:45
#
#-------------------------------------------------

TARGET = be
TEMPLATE = lib
DEFINES += __INTEL__
DEFINES += BHAPI_BUILD_LIBRARY
DEFINES += BHAPI_OS_WIN32
DEFINES += BHAPI_GRAPHICS_WIN32_BUILT_IN

CONFIG += windows shared staticlib precompile_header
CONFIG -= app_bundle
CONFIG -= qt

#Use Precompiled headers PCH
#QMAKE_CXX = ccache g++
#PRECOMPILED_HEADER += src/kits/app/AppDefs.h
#PRECOMPILED_HEADER += src/kits/interface/GraphicsDefs.h
#PRECOMPILED_HEADER += src/kits/support/SupportDefs.h
#PRECOMPILED_HEADER += src/kits/storage/StorageDefs.h
#PRECOMPILED_HEADER += src/kits/interface/InterfaceDefs.h
#PRECOMPILED_HEADER  = bhapi.h

INCLUDEPATH += ..
INCLUDEPATH += ../BHAPI/src
INCLUDEPATH += ../BHAPI/src/kits
INCLUDEPATH += ../BHAPI/src/kits/app
INCLUDEPATH += ../BHAPI/src/kits/arch
INCLUDEPATH += ../BHAPI/src/kits/add-ons
INCLUDEPATH += ../BHAPI/src/kits/add-ons/font
INCLUDEPATH += ../BHAPI/src/kits/add-ons/graphics
INCLUDEPATH += ../BHAPI/src/kits/add-ons/input_server
INCLUDEPATH += ../BHAPI/src/kits/add-ons/mail_daemon
INCLUDEPATH += ../BHAPI/src/kits/add-ons/network_settings
INCLUDEPATH += ../BHAPI/src/kits/add-ons/registrar
INCLUDEPATH += ../BHAPI/src/kits/add-ons/screen_saver
INCLUDEPATH += ../BHAPI/src/kits/add-ons/theme
INCLUDEPATH += ../BHAPI/src/kits/add-ons/tracker
INCLUDEPATH += ../BHAPI/src/kits/bluetooth
INCLUDEPATH += ../BHAPI/src/kits/debug
INCLUDEPATH += ../BHAPI/src/kits/debugger
INCLUDEPATH += ../BHAPI/src/kits/device
INCLUDEPATH += ../BHAPI/src/kits/drivers
INCLUDEPATH += ../BHAPI/src/kits/game
INCLUDEPATH += ../BHAPI/src/kits/interface
INCLUDEPATH += ../BHAPI/src/kits/kernel
INCLUDEPATH += ../BHAPI/src/kits/locale
INCLUDEPATH += ../BHAPI/src/kits/mail
INCLUDEPATH += ../BHAPI/src/kits/media
INCLUDEPATH += ../BHAPI/src/kits/midi
INCLUDEPATH += ../BHAPI/src/kits/midi2
INCLUDEPATH += ../BHAPI/src/kits/net
INCLUDEPATH += ../BHAPI/src/kits/network
INCLUDEPATH += ../BHAPI/src/kits/package
INCLUDEPATH += ../BHAPI/src/kits/print
INCLUDEPATH += ../BHAPI/src/kits/shared
INCLUDEPATH += ../BHAPI/src/kits/storage
INCLUDEPATH += ../BHAPI/src/kits/support
INCLUDEPATH += ../BHAPI/src/kits/tracker
INCLUDEPATH += ../BHAPI/src/kits/textencoding
INCLUDEPATH += ../BHAPI/src/kits/translation
INCLUDEPATH += ../BHAPI/src/kits/xml
INCLUDEPATH += ../BHAPI/src/private
INCLUDEPATH += ../BHAPI/src/private/app
INCLUDEPATH += ../BHAPI/src/private/audio
INCLUDEPATH += ../BHAPI/src/private/binary_compatbility
INCLUDEPATH += ../BHAPI/src/private/device
INCLUDEPATH += ../BHAPI/src/private/drivers
INCLUDEPATH += ../BHAPI/src/private/debug
INCLUDEPATH += ../BHAPI/src/private/debugger
INCLUDEPATH += ../BHAPI/src/private/graphics
INCLUDEPATH += ../BHAPI/src/private/interface
INCLUDEPATH += ../BHAPI/src/private/kernel
INCLUDEPATH += ../BHAPI/src/private/locale
INCLUDEPATH += ../BHAPI/src/private/mail
INCLUDEPATH += ../BHAPI/src/private/media
INCLUDEPATH += ../BHAPI/src/private/midi
INCLUDEPATH += ../BHAPI/src/private/midi2
INCLUDEPATH += ../BHAPI/src/private/net
INCLUDEPATH += ../BHAPI/src/private/package
INCLUDEPATH += ../BHAPI/src/private/shared
INCLUDEPATH += ../BHAPI/src/private/storage
INCLUDEPATH += ../BHAPI/src/private/support
INCLUDEPATH += ../BHAPI/src/private/tracker
INCLUDEPATH += ../BHAPI/src/private/translation
INCLUDEPATH += ../BHAPI/src/libs
INCLUDEPATH += ../BHAPI/src/libs/musl
INCLUDEPATH += ../BHAPI/src/libs/musl/arch/i386
INCLUDEPATH += ../BHAPI/src/libs/musl/arch/generic
INCLUDEPATH += ../BHAPI/src/libs/musl/include
INCLUDEPATH += ../BHAPI/src/libs/glut
INCLUDEPATH += ../BHAPI/src/libs/json
INCLUDEPATH += ../BHAPI/src/libs/lz4/include
INCLUDEPATH += ../BHAPI/src/libs/freetype/include
INCLUDEPATH += ../BHAPI/src/libs/zlib/include
INCLUDEPATH += ../BHAPI/src/wrappers/beos
INCLUDEPATH += ../BHAPI/src/wrappers/beos/ui
INCLUDEPATH += ../BHAPI/src/wrappers/macos
INCLUDEPATH += ../BHAPI/src/wrappers/macos/carbon
INCLUDEPATH += ../BHAPI/src/wrappers/win32
INCLUDEPATH += ../BHAPI/src/wrappers/win32/GDI
INCLUDEPATH += ../BHAPI/src/wrappers/win32/pthread
INCLUDEPATH += ../BHAPI/src/wrappers/posix
INCLUDEPATH += ../BHAPI/src/wrappers/posix/X11
INCLUDEPATH += ../BHAPI/src/wrappers/posix/DIRECTFB

QT -= core gui

SOURCES += \
    bhapi.cpp \
    src/kits/BE.cpp \
    src/kits/add-ons/font/FontEngine.cpp \
    src/kits/add-ons/font/FreeType2.cpp \
    src/kits/add-ons/graphics/GraphicsEngine.cpp \
    src/kits/add-ons/theme/DefaultTheme.cpp \
    src/kits/add-ons/theme/ThemeEngine.cpp \
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
    src/kits/support/List.cpp \
    src/kits/support/SimpleLocker.cpp \
    src/kits/support/StreamIO.cpp \
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
    src/kits/kernel/Memory.cpp \
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
    src/kits/interface/layout/LayoutItem.cpp \
    src/kits/support/SupportDefs.cpp \
    src/kits/support/Architecture.cpp \
    src/kits/support/Base64.cpp \
    src/kits/support/Beep.cpp \
    src/kits/support/BlockCache.cpp \
    src/kits/support/BufferedDataIO.cpp \
    src/kits/support/BufferIO.cpp \
    src/kits/support/DateTime.cpp \
    src/kits/support/Job.cpp \
    src/kits/support/JobQueue.cpp \
    src/kits/support/Referenceable.cpp \
    src/kits/support/StopWatch.cpp \
    src/kits/support/ByteOrder.cpp \
    src/kits/support/StringArray.cpp \
    src/kits/support/StringList.cpp \
    src/kits/support/StandardIO.cpp \
    src/kits/support/UTF8.cpp \
    src/private/app/MessageBody.cpp \
    src/private/app/PrivateApplication.cpp \
    src/private/app/PrivateHandler.cpp \
    src/private/support/Object.cpp \
    src/private/support/Token.cpp \
    src/private/support/StringPrivate.cpp \
    src/private/support/CompressionAlgorithm.cpp \
    src/private/support/ZlibCompressionAlgorithm.cpp \
    src/private/support/Uuid.cpp \
    src/private/support/DataPositionIOWrapper.cpp \
    src/kits/support/StringClass.cpp \
    src/kits/kernel/Kernel.cpp \
    src/kits/app/AppDefs.cpp \
    src/kits/app/AppMisc.cpp \
#    src/kits/app/AppServerLink.cpp \
#    src/kits/app/DesktopLink.cpp \
#    src/kits/app/DirectMessageTarget.cpp \
#    src/kits/app/InitTerminateLibBe.cpp \
#    src/kits/app/Key.cpp \
#    src/kits/app/KeyStore.cpp \
#    src/kits/app/LaunchRoster.cpp \
#    src/kits/app/LinkReceiver.cpp \
#    src/kits/app/LinkSender.cpp \
#    src/kits/app/LooperList.cpp \
#    src/kits/app/MessageAdapter.cpp \
#    src/kits/app/MessageUtils.cpp \
#    src/kits/app/Notification.cpp \
    src/kits/app/PortLink.cpp \
    src/kits/app/PropertyInfo.cpp \
    src/kits/app/RegistrarDefs.cpp \
    src/kits/app/Roster.cpp \
    src/kits/app/RosterPrivate.cpp \
    src/kits/app/Server.cpp \
    src/kits/app/ServerLink.cpp \
    src/kits/app/ServerMemoryAllocator.cpp \
    src/kits/app/TokenSpace.cpp \
    src/kits/app/TypeConstants.cpp \
##    src/kits/bluetooth/UI/ConnectionIncoming.cpp \
##    src/kits/bluetooth/UI/PincodeWindow.cpp \
##    src/kits/bluetooth/CommandManager.cpp \
##    src/kits/bluetooth/DeviceClass.cpp \
##    src/kits/bluetooth/DiscoveryAgent.cpp \
##    src/kits/bluetooth/DiscoveryListener.cpp \
##    src/kits/bluetooth/KitSupport.cpp \
##    src/kits/bluetooth/LocalDevice.cpp \
##    src/kits/bluetooth/RemoteDevice.cpp \
#    src/kits/debug/debug_support.cpp \
#    src/kits/debug/DebugContext.cpp \
#    src/kits/debug/DebugEventStream.cpp \
#    src/kits/debug/DebugLooper.cpp \
#    src/kits/debug/DebugMessageHandler.cpp \
#    src/kits/debug/Image.cpp \
#    src/kits/debug/SymbolLookup.cpp \
#    src/kits/debug/TeamDebugger.cpp \
#    src/kits/debugger/arch/Architecture.cpp \
#    src/kits/debugger/arch/CpuState.cpp \
#    src/kits/debugger/arch/InstructionInfo.cpp \
#    src/kits/debugger/arch/Register.cpp \
#    src/kits/debugger/arch/RegisterMap.cpp \
#    src/kits/debugger/controllers/DebugReportGenerator.cpp \
#    src/kits/debugger/controllers/TeamDebugger.cpp \
#    src/kits/debugger/controllers/ThreadHandler.cpp \
#    src/kits/debugger/debug_info/loading_state_handlers/DwarfLoadingStateHandler.cpp \
#    src/kits/debugger/debug_info/BasicFunctionDebugInfo.cpp \
#    src/kits/debugger/debug_info/DebuggerImageDebugInfo.cpp \
#    src/kits/debugger/debug_info/DebuggerTeamDebugInfo.cpp \
#    src/kits/debugger/debug_info/DwarfFunctionDebugInfo.cpp \
#    src/kits/debugger/debug_info/DwarfImageDebugInfo.cpp \
#    src/kits/debugger/debug_info/DwarfImageDebugInfoLoadingState.cpp \
#    src/kits/debugger/debug_info/DwarfStackFrameDebugInfo.cpp \
#    src/kits/debugger/debug_info/DwarfTeamDebugInfo.cpp \
#    src/kits/debugger/debug_info/DwarfTypeFactory.cpp \
#    src/kits/debugger/debug_info/DwarfTypes.cpp \
#    src/kits/debugger/debug_info/Function.cpp \
#    src/kits/debugger/debug_info/FunctionDebugInfo.cpp \
#    src/kits/debugger/debug_info/FunctionInstance.cpp \
#    src/kits/debugger/debug_info/GlobalTypeLookup.cpp \
#    src/kits/debugger/debug_info/ImageDebugInfo.cpp \
#    src/kits/debugger/debug_info/ImageDebugInfoLoadingState.cpp \
#    src/kits/debugger/debug_info/ImageDebugInfoProvider.cpp \
#    src/kits/debugger/debug_info/ImageDebugLoadingStateHandler.cpp \
#    src/kits/debugger/debug_info/ImageDebugLoadingStateHandlerRoster.cpp \
#    src/kits/debugger/debug_info/NoOpStackFrameDebugInfo.cpp \
#    src/kits/debugger/debug_info/SpecificImageDebugInfo.cpp \
#    src/kits/debugger/debug_info/SpecificImageDebugInfoLoadingState.cpp \
#    src/kits/debugger/debug_info/SpecificTeamDebugInfo.cpp \
#    src/kits/debugger/debug_info/StackFrameDebugInfo.cpp \
#    src/kits/debugger/debug_info/TeamDebugInfo.cpp \
#    src/kits/debugger/debug_managers/BreakpointManager.cpp \
#    src/kits/debugger/debug_managers/TeamMemoryBlockManager.cpp \
#    src/kits/debugger/debug_managers/ValueNodeManager.cpp \
#    src/kits/debugger/debug_managers/WatchpointManager.cpp \
#    src/kits/debugger/debugger_interface/core/CoreFileDebuggerInterface.cpp \
#    src/kits/debugger/debugger_interface/local/LocalDebuggerInterface.cpp \
#    src/kits/debugger/debugger_interface/remote/RemoteDebugRequest.cpp \
#    src/kits/debugger/debugger_interface/DebugEvent.cpp \
#    src/kits/debugger/debugger_interface/DebuggerInterface.cpp \
#    src/kits/debugger/demangler/Demangler.cpp \
#    src/kits/debugger/dwarf/AbbreviationTable.cpp \
#    src/kits/debugger/dwarf/AttributeClasses.cpp \
#    src/kits/debugger/dwarf/AttributeValue.cpp \
#    src/kits/debugger/dwarf/BaseUnit.cpp \
#    src/kits/debugger/dwarf/CfaContext.cpp \
#    src/kits/debugger/dwarf/CfaRuleSet.cpp \
#    src/kits/debugger/dwarf/CompilationUnit.cpp \
#    src/kits/debugger/dwarf/DebugInfoEntries.cpp \
#    src/kits/debugger/dwarf/DebugInfoEntry.cpp \
#    src/kits/debugger/dwarf/DwarfExpressionEvaluator.cpp \
#    src/kits/debugger/dwarf/DwarfFile.cpp \
#    src/kits/debugger/dwarf/DwarfFileLoadingState.cpp \
#    src/kits/debugger/dwarf/DwarfManager.cpp \
#    src/kits/debugger/dwarf/DwarfTargetInterface.cpp \
#    src/kits/debugger/dwarf/DwarfUtils.cpp \
#    src/kits/debugger/dwarf/LineNumberProgram.cpp \
#    src/kits/debugger/dwarf/SourceLanguageInfo.cpp \
#    src/kits/debugger/dwarf/TagNames.cpp \
#    src/kits/debugger/dwarf/TypeUnit.cpp \
#    src/kits/debugger/elf/CoreFile.cpp \
#    src/kits/debugger/elf/ElfFile.cpp \
#    src/kits/debugger/elf/ElfSymbolLookup.cpp \
#    src/kits/debugger/files/FileManager.cpp \
#    src/kits/debugger/files/LocatableDirectory.cpp \
#    src/kits/debugger/files/LocatableEntry.cpp \
#    src/kits/debugger/files/LocatableFile.cpp \
#    src/kits/debugger/files/SourceFile.cpp \
#    src/kits/debugger/ids/FunctionID.cpp \
#    src/kits/debugger/ids/FunctionParameterID.cpp \
#    src/kits/debugger/ids/LocalVariableID.cpp \
#    src/kits/debugger/ids/ObjectID.cpp \
#    src/kits/debugger/ids/ReturnValueID.cpp \
#    src/kits/debugger/jobs/ExpressionEvaluationJob.cpp \
#    src/kits/debugger/jobs/GetCPUStateJob.cpp \
#    src/kits/debugger/jobs/GetStackTraceJob.cpp \
#    src/kits/debugger/jobs/GetThreadStateJob.cpp \
#    src/kits/debugger/jobs/LoadImageDebugInfoJob.cpp \
#    src/kits/debugger/jobs/LoadSourceCodeJob.cpp \
#    src/kits/debugger/jobs/ResolveValueNodeJob.cpp \
#    src/kits/debugger/jobs/RetrieveMemoryBlockJob.cpp \
#    src/kits/debugger/jobs/WriteCoreFileJob.cpp \
#    src/kits/debugger/jobs/WriteMemoryJob.cpp \
#    src/kits/debugger/jobs/WriteValueNodeJob.cpp \
#    src/kits/debugger/model/AreaInfo.cpp \
#    src/kits/debugger/model/Breakpoint.cpp \
#    src/kits/debugger/model/DisassembledCode.cpp \
#    src/kits/debugger/model/ExpressionInfo.cpp \
#    src/kits/debugger/model/ExpressionValues.cpp \
#    src/kits/debugger/model/FileSourceCode.cpp \
#    src/kits/debugger/model/Image.cpp \
#    src/kits/debugger/model/ImageInfo.cpp \
#    src/kits/debugger/model/LineDataSource.cpp \
#    src/kits/debugger/model/ReturnValueInfo.cpp \
#    src/kits/debugger/model/SemaphoreInfo.cpp \
#    src/kits/debugger/model/SignalInfo.cpp \
#    src/kits/debugger/model/SourceCode.cpp \
#    src/kits/debugger/model/StackFrame.cpp \
#    src/kits/debugger/model/StackFrameValueInfos.cpp \
#    src/kits/debugger/model/StackFrameValues.cpp \
#    src/kits/debugger/model/StackTrace.cpp \
#    src/kits/debugger/model/Statement.cpp \
#    src/kits/debugger/model/SymbolInfo.cpp \
#    src/kits/debugger/model/SyntheticPrimitiveType.cpp \
#    src/kits/debugger/model/SyscallInfo.cpp \
#    src/kits/debugger/model/SystemInfo.cpp \
#    src/kits/debugger/model/TargetHost.cpp \
#    src/kits/debugger/model/Team.cpp \
#    src/kits/debugger/model/TeamFunctionSourceInformation.cpp \
#    src/kits/debugger/model/TeamInfo.cpp \
#    src/kits/debugger/model/TeamMemory.cpp \
#    src/kits/debugger/model/TeamMemoryBlock.cpp \
#    src/kits/debugger/model/TeamTypeInformation.cpp \
#    src/kits/debugger/model/Thread.cpp \
#    src/kits/debugger/model/ThreadInfo.cpp \
#    src/kits/debugger/model/Type.cpp \
#    src/kits/debugger/model/TypeComponentPath.cpp \
#    src/kits/debugger/model/TypeLookupConstraints.cpp \
#    src/kits/debugger/model/UserBreakpoint.cpp \
#    src/kits/debugger/model/Variable.cpp \
#    src/kits/debugger/model/Watchpoint.cpp \
#    src/kits/debugger/settings/generic/Setting.cpp \
#    src/kits/debugger/settings/generic/Settings.cpp \
#    src/kits/debugger/settings/generic/SettingsDescription.cpp \
#    src/kits/debugger/settings/BreakpointSetting.cpp \
#    src/kits/debugger/settings/NoOpSettingsManager.cpp \
#    src/kits/debugger/settings/SettingsManager.cpp \
#    src/kits/debugger/settings/TeamFileManagerSettings.cpp \
#    src/kits/debugger/settings/TeamSettings.cpp \
#    src/kits/debugger/settings/TeamSignalSettings.cpp \
#    src/kits/debugger/settings/TeamUiSettings.cpp \
#    src/kits/debugger/settings/TeamUiSettingsFactory.cpp \
#    src/kits/debugger/source_language/c_family/CLanguage.cpp \
#    src/kits/debugger/source_language/c_family/CLanguageExpressionEvaluator.cpp \
#    src/kits/debugger/source_language/c_family/CLanguageFamily.cpp \
#    src/kits/debugger/source_language/c_family/CLanguageFamilySyntaxHighlighter.cpp \
#    src/kits/debugger/source_language/c_family/CLanguageFamilySyntaxHighlightInfo.cpp \
#    src/kits/debugger/source_language/c_family/CLanguageTokenizer.cpp \
#    src/kits/debugger/source_language/c_family/CppLanguage.cpp \
#    src/kits/debugger/source_language/x86/X86AssemblyLanguage.cpp \
#    src/kits/debugger/source_language/SourceLanguage.cpp \
#    src/kits/debugger/source_language/SyntaxHighlighter.cpp \
#    src/kits/debugger/source_language/UnsupportedLanguage.cpp \
#    src/kits/debugger/target_host_interface/local/LocalTargetHostInterface.cpp \
#    src/kits/debugger/target_host_interface/local/LocalTargetHostInterfaceInfo.cpp \
#    src/kits/debugger/target_host_interface/TargetHostInterface.cpp \
#    src/kits/debugger/target_host_interface/TargetHostInterfaceInfo.cpp \
#    src/kits/debugger/target_host_interface/TargetHostInterfaceRoster.cpp \
#    src/kits/debugger/types/ArrayIndexPath.cpp \
#    src/kits/debugger/types/TargetAddressRangeList.cpp \
#    src/kits/debugger/types/ValueLocation.cpp \
#    src/kits/debugger/user_interface/util/UiUtils.cpp \
#    src/kits/debugger/user_interface/UserInterface.cpp \
#    src/kits/debugger/util/ArchivingUtils.cpp \
#    src/kits/debugger/util/BitBuffer.cpp \
#    src/kits/debugger/util/IntegerFormatter.cpp \
#    src/kits/debugger/util/RangeList.cpp \
#    src/kits/debugger/util/StringUtils.cpp \
#    src/kits/debugger/util/Worker.cpp \
#    src/kits/debugger/value/type_handlers/BListTypeHandler.cpp \
#    src/kits/debugger/value/type_handlers/BMessageTypeHandler.cpp \
#    src/kits/debugger/value/type_handlers/CStringTypeHandler.cpp \
#    src/kits/debugger/value/value_formatters/BoolValueFormatter.cpp \
#    src/kits/debugger/value/value_formatters/EnumerationValueFormatter.cpp \
#    src/kits/debugger/value/value_formatters/FloatValueFormatter.cpp \
#    src/kits/debugger/value/value_formatters/IntegerValueFormatter.cpp \
#    src/kits/debugger/value/value_formatters/StringValueFormatter.cpp \
#    src/kits/debugger/value/value_nodes/AddressValueNode.cpp \
#    src/kits/debugger/value/value_nodes/ArrayValueNode.cpp \
#    src/kits/debugger/value/value_nodes/BListValueNode.cpp \
#    src/kits/debugger/value/value_nodes/BMessageValueNode.cpp \
#    src/kits/debugger/value/value_nodes/CompoundValueNode.cpp \
#    src/kits/debugger/value/value_nodes/CStringValueNode.cpp \
#    src/kits/debugger/value/value_nodes/EnumerationValueNode.cpp \
#    src/kits/debugger/value/value_nodes/ExpressionValueNode.cpp \
#    src/kits/debugger/value/value_nodes/PointerToMemberValueNode.cpp \
#    src/kits/debugger/value/value_nodes/PrimitiveValueNode.cpp \
#    src/kits/debugger/value/value_nodes/VariableValueNodeChild.cpp \
#    src/kits/debugger/value/values/AddressValue.cpp \
#    src/kits/debugger/value/values/BoolValue.cpp \
#    src/kits/debugger/value/values/EnumerationValue.cpp \
#    src/kits/debugger/value/values/FloatValue.cpp \
#    src/kits/debugger/value/values/IntegerValue.cpp \
#    src/kits/debugger/value/values/StringValue.cpp \
#    src/kits/debugger/value/TypeHandler.cpp \
#    src/kits/debugger/value/TypeHandlerRoster.cpp \
#    src/kits/debugger/value/Value.cpp \
#    src/kits/debugger/value/ValueFormatter.cpp \
#    src/kits/debugger/value/ValueLoader.cpp \
#    src/kits/debugger/value/ValueNode.cpp \
#    src/kits/debugger/value/ValueNodeContainer.cpp \
#    src/kits/debugger/value/ValueWriter.cpp \
#    src/kits/debugger/DebuggerGlobals.cpp \
#    src/kits/device/A2D.cpp \
#    src/kits/device/D2A.cpp \
#    src/kits/device/DigitalPort.cpp \
#    src/kits/device/Joystick.cpp \
#    src/kits/device/JoystickTweaker.cpp \
#    src/kits/device/SerialPort.cpp \
#    src/kits/device/USBConfiguration.cpp \
#    src/kits/device/USBDevice.cpp \
#    src/kits/device/USBEndpoint.cpp \
#    src/kits/device/USBInterface.cpp \
#    src/kits/device/USBRoster.cpp \
#    src/kits/game/DirectWindow.cpp \
#    src/kits/game/FileGameSound.cpp \
#    src/kits/game/GameProducer.cpp \
#    src/kits/game/GameSound.cpp \
#    src/kits/game/GameSoundBuffer.cpp \
#    src/kits/game/GameSoundDevice.cpp \
#    src/kits/game/GSUtility.cpp \
#    src/kits/game/PushGameSound.cpp \
#    src/kits/game/SimpleGameSound.cpp \
#    src/kits/game/StreamingGameSound.cpp \
#    src/kits/game/WindowScreen.cpp \
    src/kits/interface/layouter/CollapsingLayouter.cpp \
    src/kits/interface/layouter/ComplexLayouter.cpp \
    src/kits/interface/layouter/Layouter.cpp \
    src/kits/interface/layouter/LayoutOptimizer.cpp \
    src/kits/interface/layouter/OneElementLayouter.cpp \
    src/kits/interface/layouter/SimpleLayouter.cpp \
    src/kits/interface/textview_support/InlineInput.cpp \
    src/kits/interface/textview_support/LineBuffer.cpp \
    src/kits/interface/textview_support/StyleBuffer.cpp \
    src/kits/interface/textview_support/TextGapBuffer.cpp \
    src/kits/interface/textview_support/UndoBuffer.cpp \
    src/kits/interface/textview_support/WidthBuffer.cpp \
    src/kits/interface/AboutWindow.cpp \
    src/kits/interface/AbstractLayout.cpp \
    src/kits/interface/AbstractLayoutItem.cpp \
    src/kits/interface/AbstractSpinner.cpp \
    src/kits/interface/AffineTransform.cpp \
    src/kits/interface/Alignment.cpp \
    src/kits/interface/BMCPrivate.cpp \
    src/kits/interface/CardLayout.cpp \
    src/kits/interface/CardView.cpp \
    src/kits/interface/ChannelControl.cpp \
    src/kits/interface/ChannelSlider.cpp \
    src/kits/interface/ColorConversion.cpp \
    src/kits/interface/ColorMenuItem.cpp \
    src/kits/interface/ColorTools.cpp \
    src/kits/interface/ColumnListView.cpp \
    src/kits/interface/ColumnTypes.cpp \
    src/kits/interface/ControlLook.cpp \
    src/kits/interface/DecimalSpinner.cpp \
    src/kits/interface/DecorInfo.cpp \
    src/kits/interface/Deskbar.cpp \
    src/kits/interface/Dragger.cpp \
    src/kits/interface/Gradient.cpp \
    src/kits/interface/GradientConic.cpp \
    src/kits/interface/GradientDiamond.cpp \
    src/kits/interface/GradientLinear.cpp \
    src/kits/interface/GradientRadial.cpp \
    src/kits/interface/GradientRadialFocus.cpp \
    src/kits/interface/GridLayout.cpp \
    src/kits/interface/GridLayoutBuilder.cpp \
    src/kits/interface/GridView.cpp \
    src/kits/interface/GroupLayout.cpp \
    src/kits/interface/GroupLayoutBuilder.cpp \
    src/kits/interface/GroupView.cpp \
    src/kits/interface/Icon.cpp \
    src/kits/interface/Input.cpp \
    src/kits/interface/InterfaceDefs.cpp \
    src/kits/interface/Layout.cpp \
    src/kits/interface/LayoutContext.cpp \
    src/kits/interface/LayoutItem.cpp \
    src/kits/interface/LayoutUtils.cpp \
    src/kits/interface/MenuItemPrivate.cpp \
    src/kits/interface/MenuPrivate.cpp \
    src/kits/interface/MenuWindow.cpp \
    src/kits/interface/OptionControl.cpp \
    src/kits/interface/OptionPopUp.cpp \
    src/kits/interface/Picture.cpp \
    src/kits/interface/PictureButton.cpp \
    src/kits/interface/PictureDataWriter.cpp \
    src/kits/interface/PicturePlayer.cpp \
    src/kits/interface/PrintJob.cpp \
    src/kits/interface/PrivateScreen.cpp \
    src/kits/interface/RegionSupport.cpp \
    src/kits/interface/SeparatorItem.cpp \
    src/kits/interface/SeparatorView.cpp \
    src/kits/interface/Shape.cpp \
    src/kits/interface/Shelf.cpp \
    src/kits/interface/Size.cpp \
    src/kits/interface/Slider.cpp \
    src/kits/interface/SpaceLayoutItem.cpp \
    src/kits/interface/Spinner.cpp \
    src/kits/interface/SplitLayout.cpp \
    src/kits/interface/SplitLayoutBuilder.cpp \
    src/kits/interface/SplitView.cpp \
    src/kits/interface/StringItem.cpp \
    src/kits/interface/SystemPalette.cpp \
    src/kits/interface/TextInput.cpp \
    src/kits/interface/ToolTipManager.cpp \
    src/kits/interface/TwoDimensionalLayout.cpp \
    src/kits/interface/ViewLayoutItem.cpp \
    src/kits/interface/ViewPort.cpp \
    src/kits/interface/WindowStack.cpp \
    src/kits/interface/ZombieReplicantView.cpp \
#    src/kits/locale/cat.cpp \
#    src/kits/locale/Catalog.cpp \
#    src/kits/locale/CatalogData.cpp \
#    src/kits/locale/CatalogStub.cpp \
#    src/kits/locale/Collator.cpp \
#    src/kits/locale/Country.cpp \
#    src/kits/locale/Currency.cpp \
#    src/kits/locale/DateFormat.cpp \
#    src/kits/locale/DateTimeFormat.cpp \
#    src/kits/locale/DefaultCatalog.cpp \
#    src/kits/locale/DurationFormat.cpp \
#    src/kits/locale/EditableCatalog.cpp \
#    src/kits/locale/FloatFormat.cpp \
#    src/kits/locale/FloatFormatImpl.cpp \
#    src/kits/locale/FloatFormatParameters.cpp \
#    src/kits/locale/Format.cpp \
#    src/kits/locale/FormatImpl.cpp \
#    src/kits/locale/FormatParameters.cpp \
#    src/kits/locale/FormattingConventions.cpp \
#    src/kits/locale/GenericNumberFormat.cpp \
#    src/kits/locale/HashMapCatalog.cpp \
#    src/kits/locale/InitLocaleKit.cpp \
#    src/kits/locale/IntegerFormat.cpp \
#    src/kits/locale/IntegerFormatImpl.cpp \
#    src/kits/locale/IntegerFormatParameters.cpp \
#    src/kits/locale/Language.cpp \
#    src/kits/locale/LocaleClass.cpp \
#    src/kits/locale/LocaleRoster.cpp \
#    src/kits/locale/LocaleRosterData.cpp \
#    src/kits/locale/MessageFormat.cpp \
#    src/kits/locale/MutableLocaleRoster.cpp \
#    src/kits/locale/NumberFormat.cpp \
#    src/kits/locale/NumberFormatParameters.cpp \
#    src/kits/locale/TextEncoding.cpp \
#    src/kits/locale/TimeFormat.cpp \
#    src/kits/locale/TimeUnitFormat.cpp \
#    src/kits/locale/TimeZone.cpp \
#    src/kits/locale/UnicodeChar.cpp \
#    src/kits/mail/b_mail_message.cpp \
#    src/kits/mail/c_mail_api.cpp \
#    src/kits/mail/crypt.cpp \
#    src/kits/mail/FileConfigView.cpp \
#    src/kits/mail/HaikuMailFormatFilter.cpp \
#    src/kits/mail/mail_encoding.cpp \
#    src/kits/mail/mail_util.cpp \
#    src/kits/mail/MailAttachment.cpp \
#    src/kits/mail/MailComponent.cpp \
#    src/kits/mail/MailContainer.cpp \
#    src/kits/mail/MailDaemon.cpp \
#    src/kits/mail/MailFilter.cpp \
#    src/kits/mail/MailMessage.cpp \
#    src/kits/mail/MailProtocol.cpp \
#    src/kits/mail/MailSettings.cpp \
#    src/kits/mail/MailSettingsView.cpp \
#    src/kits/mail/NodeMessage.cpp \
#    src/kits/mail/numailkit.cpp \
#    src/kits/mail/ProtocolConfigView.cpp \
#    src/kits/media/legacy/!missing_symbols.cpp \
#    src/kits/media/legacy/Deprecated.cpp \
#    src/kits/media/legacy/OldAudioModule.cpp \
#    src/kits/media/legacy/OldAudioStream.cpp \
#    src/kits/media/legacy/OldBufferStream.cpp \
#    src/kits/media/legacy/OldBufferStreamManager.cpp \
#    src/kits/media/legacy/OldMediaModule.cpp \
#    src/kits/media/legacy/OldSubscriber.cpp \
#    src/kits/media/!missing_symbols.cpp \
#    src/kits/media/AdapterIO.cpp \
#    src/kits/media/AddOnManager.cpp \
#    src/kits/media/Buffer.cpp \
#    src/kits/media/BufferCache.cpp \
#    src/kits/media/BufferConsumer.cpp \
#    src/kits/media/BufferGroup.cpp \
#    src/kits/media/BufferProducer.cpp \
#    src/kits/media/ChunkCache.cpp \
#    src/kits/media/Controllable.cpp \
#    src/kits/media/DataExchange.cpp \
#    src/kits/media/DecoderPlugin.cpp \
#    src/kits/media/DefaultMediaTheme.cpp \
#    src/kits/media/DormantNodeManager.cpp \
#    src/kits/media/EncoderPlugin.cpp \
#    src/kits/media/FileInterface.cpp \
#    src/kits/media/FormatManager.cpp \
#    src/kits/media/MediaAddOn.cpp \
#    src/kits/media/MediaClient.cpp \
#    src/kits/media/MediaClientDefs.cpp \
#    src/kits/media/MediaClientNode.cpp \
#    src/kits/media/MediaConnection.cpp \
#    src/kits/media/MediaDecoder.cpp \
#    src/kits/media/MediaDefs.cpp \
#    src/kits/media/MediaEncoder.cpp \
#    src/kits/media/MediaEventLooper.cpp \
#    src/kits/media/MediaExtractor.cpp \
#    src/kits/media/MediaFile.cpp \
#    src/kits/media/MediaFiles.cpp \
#    src/kits/media/MediaFormats.cpp \
#    src/kits/media/MediaIO.cpp \
#    src/kits/media/MediaNode.cpp \
#    src/kits/media/MediaPlugin.cpp \
#    src/kits/media/MediaRecorder.cpp \
#    src/kits/media/MediaRecorderNode.cpp \
#    src/kits/media/MediaRoster.cpp \
#    src/kits/media/MediaTheme.cpp \
#    src/kits/media/MediaTrack.cpp \
#    src/kits/media/MediaWriter.cpp \
#    src/kits/media/Notifications.cpp \
#    src/kits/media/ParameterWeb.cpp \
#    src/kits/media/PlaySound.cpp \
#    src/kits/media/PluginManager.cpp \
#    src/kits/media/ReaderPlugin.cpp \
#    src/kits/media/RealtimeAlloc.cpp \
#    src/kits/media/request_data.cpp \
#    src/kits/media/SharedBufferList.cpp \
#    src/kits/media/Sound.cpp \
#    src/kits/media/SoundFile.cpp \
#    src/kits/media/SoundPlayer.cpp \
#    src/kits/media/SoundPlayNode.cpp \
#    src/kits/media/StreamerPlugin.cpp \
#    src/kits/media/TimeCode.cpp \
#    src/kits/media/TimedEventQueue.cpp \
#    src/kits/media/TimedEventQueuePrivate.cpp \
#    src/kits/media/TimeSource.cpp \
#    src/kits/media/TimeSourceObject.cpp \
#    src/kits/media/TimeSourceObjectManager.cpp \
#    src/kits/media/TrackReader.cpp \
#    src/kits/media/WriterPlugin.cpp \
#    src/kits/midi/Midi.cpp \
#    src/kits/midi/MidiGlue.cpp \
#    src/kits/midi/MidiPort.cpp \
#    src/kits/midi/MidiSettings.cpp \
#    src/kits/midi/MidiStore.cpp \
#    src/kits/midi/MidiSynth.cpp \
#    src/kits/midi/MidiSynthFile.cpp \
#    src/kits/midi/MidiText.cpp \
#    src/kits/midi/Samples.cpp \
#    src/kits/midi/SoftSynth.cpp \
#    src/kits/midi/Synth.cpp \
#    src/kits/midi2/MidiConsumer.cpp \
#    src/kits/midi2/MidiEndpoint.cpp \
#    src/kits/midi2/MidiLocalConsumer.cpp \
#    src/kits/midi2/MidiLocalProducer.cpp \
#    src/kits/midi2/MidiProducer.cpp \
#    src/kits/midi2/MidiRoster.cpp \
#    src/kits/midi2/MidiRosterLooper.cpp \
    src/kits/network/libnetapi/AbstractSocket.cpp \
    src/kits/network/libnetapi/Certificate.cpp \
    src/kits/network/libnetapi/DatagramSocket.cpp \
    src/kits/network/libnetapi/DataRequest.cpp \
    src/kits/network/libnetapi/DynamicBuffer.cpp \
    src/kits/network/libnetapi/FileRequest.cpp \
    src/kits/network/libnetapi/GopherRequest.cpp \
    src/kits/network/libnetapi/HttpAuthentication.cpp \
    src/kits/network/libnetapi/HttpForm.cpp \
    src/kits/network/libnetapi/HttpHeaders.cpp \
    src/kits/network/libnetapi/HttpRequest.cpp \
    src/kits/network/libnetapi/HttpResult.cpp \
    src/kits/network/libnetapi/HttpTime.cpp \
    src/kits/network/libnetapi/init.cpp \
    src/kits/network/libnetapi/NetAddress.cpp \
    src/kits/network/libnetapi/NetBuffer.cpp \
    src/kits/network/libnetapi/NetDebug.cpp \
    src/kits/network/libnetapi/NetEndpoint.cpp \
    src/kits/network/libnetapi/NetworkAddress.cpp \
    src/kits/network/libnetapi/NetworkAddressResolver.cpp \
    src/kits/network/libnetapi/NetworkCookie.cpp \
    src/kits/network/libnetapi/NetworkCookieJar.cpp \
    src/kits/network/libnetapi/NetworkDevice.cpp \
    src/kits/network/libnetapi/NetworkInterface.cpp \
    src/kits/network/libnetapi/NetworkRequest.cpp \
    src/kits/network/libnetapi/NetworkRoster.cpp \
    src/kits/network/libnetapi/NetworkRoute.cpp \
    src/kits/network/libnetapi/NetworkSettings.cpp \
    src/kits/network/libnetapi/notifications.cpp \
    src/kits/network/libnetapi/ProxySecureSocket.cpp \
    src/kits/network/libnetapi/SecureSocket.cpp \
    src/kits/network/libnetapi/Socket.cpp \
    src/kits/network/libnetapi/SocketMessenger.cpp \
    src/kits/network/libnetapi/SSL.cpp \
    src/kits/network/libnetapi/Url.cpp \
    src/kits/network/libnetapi/UrlContext.cpp \
    src/kits/network/libnetapi/UrlProtocolAsynchronousListener.cpp \
    src/kits/network/libnetapi/UrlProtocolDispatchingListener.cpp \
    src/kits/network/libnetapi/UrlProtocolListener.cpp \
    src/kits/network/libnetapi/UrlProtocolRoster.cpp \
    src/kits/network/libnetapi/UrlRequest.cpp \
    src/kits/network/libnetapi/UrlResult.cpp \
    src/kits/network/libnetapi/UrlSynchronousRequest.cpp \
    src/kits/network/netresolv/irs/lcl_sv.cpp \
    src/kits/network/getifaddrs.cpp \
    src/kits/network/init.cpp \
    src/kits/network/interfaces.cpp \
    src/kits/network/r5_compatibility.cpp \
    src/kits/network/socket.cpp \
#    src/kits/package/hpkg/v1/HPKGDefsV1.cpp \
#    src/kits/package/hpkg/v1/PackageContentHandlerV1.cpp \
#    src/kits/package/hpkg/v1/PackageDataReaderV1.cpp \
#    src/kits/package/hpkg/v1/PackageDataV1.cpp \
#    src/kits/package/hpkg/v1/PackageEntryAttributeV1.cpp \
#    src/kits/package/hpkg/v1/PackageEntryV1.cpp \
#    src/kits/package/hpkg/v1/PackageInfoContentHandlerV1.cpp \
#    src/kits/package/hpkg/v1/PackageReaderImplV1.cpp \
#    src/kits/package/hpkg/v1/PackageReaderV1.cpp \
#    src/kits/package/hpkg/v1/ReaderImplBaseV1.cpp \
#    src/kits/package/hpkg/AttributeDataReader.cpp \
#    src/kits/package/hpkg/BlockBufferPool.cpp \
#    src/kits/package/hpkg/BlockBufferPoolImpl.cpp \
#    src/kits/package/hpkg/BlockBufferPoolNoLock.cpp \
#    src/kits/package/hpkg/BufferPool.cpp \
#    src/kits/package/hpkg/DataReader.cpp \
#    src/kits/package/hpkg/ErrorOutput.cpp \
#    src/kits/package/hpkg/FDDataReader.cpp \
#    src/kits/package/hpkg/HPKGDefs.cpp \
#    src/kits/package/hpkg/NoErrorOutput.cpp \
#    src/kits/package/hpkg/PackageContentHandler.cpp \
#    src/kits/package/hpkg/PackageData.cpp \
#    src/kits/package/hpkg/PackageDataReader.cpp \
#    src/kits/package/hpkg/PackageEntry.cpp \
#    src/kits/package/hpkg/PackageEntryAttribute.cpp \
#    src/kits/package/hpkg/PackageFileHeapAccessorBase.cpp \
#    src/kits/package/hpkg/PackageFileHeapReader.cpp \
#    src/kits/package/hpkg/PackageFileHeapWriter.cpp \
#    src/kits/package/hpkg/PackageReader.cpp \
#    src/kits/package/hpkg/PackageReaderImpl.cpp \
#    src/kits/package/hpkg/PackageWriter.cpp \
#    src/kits/package/hpkg/PackageWriterImpl.cpp \
#    src/kits/package/hpkg/PoolBuffer.cpp \
#    src/kits/package/hpkg/ReaderImplBase.cpp \
#    src/kits/package/hpkg/RepositoryContentHandler.cpp \
#    src/kits/package/hpkg/RepositoryReader.cpp \
#    src/kits/package/hpkg/RepositoryReaderImpl.cpp \
#    src/kits/package/hpkg/RepositoryWriter.cpp \
#    src/kits/package/hpkg/RepositoryWriterImpl.cpp \
#    src/kits/package/hpkg/StandardErrorOutput.cpp \
#    src/kits/package/hpkg/Strings.cpp \
#    src/kits/package/hpkg/WriterImplBase.cpp \
#    src/kits/package/manager/Exceptions.cpp \
#    src/kits/package/manager/PackageManager.cpp \
#    src/kits/package/manager/RepositoryBuilder.cpp \
#    src/kits/package/solver/LibsolvSolver.cpp \
#    src/kits/package/solver/Solver.cpp \
#    src/kits/package/solver/SolverPackage.cpp \
#    src/kits/package/solver/SolverPackageSpecifier.cpp \
#    src/kits/package/solver/SolverPackageSpecifierList.cpp \
#    src/kits/package/solver/SolverProblem.cpp \
#    src/kits/package/solver/SolverProblemSolution.cpp \
#    src/kits/package/solver/SolverRepository.cpp \
#    src/kits/package/solver/SolverResult.cpp \
#    src/kits/package/ActivateRepositoryCacheJob.cpp \
#    src/kits/package/ActivateRepositoryConfigJob.cpp \
#    src/kits/package/ActivationTransaction.cpp \
#    src/kits/package/AddRepositoryRequest.cpp \
#    src/kits/package/Attributes.cpp \
#    src/kits/package/ChecksumAccessors.cpp \
#    src/kits/package/CommitTransactionResult.cpp \
#    src/kits/package/Context.cpp \
#    src/kits/package/DaemonClient.cpp \
#    src/kits/package/DownloadFileRequest.cpp \
#    src/kits/package/DropRepositoryRequest.cpp \
#    src/kits/package/FetchFileJob.cpp \
#    src/kits/package/GlobalWritableFileInfo.cpp \
#    src/kits/package/InitTerminateLibPackage.cpp \
#    src/kits/package/InstallationLocationInfo.cpp \
#    src/kits/package/Job.cpp \
#    src/kits/package/PackageInfo.cpp \
#    src/kits/package/PackageInfoContentHandler.cpp \
#    src/kits/package/PackageInfoParser.cpp \
#    src/kits/package/PackageInfoSet.cpp \
#    src/kits/package/PackageResolvable.cpp \
#    src/kits/package/PackageResolvableExpression.cpp \
#    src/kits/package/PackageRoster.cpp \
#    src/kits/package/PackageVersion.cpp \
#    src/kits/package/PathFinder.cpp \
#    src/kits/package/RefreshRepositoryRequest.cpp \
#    src/kits/package/RemoveRepositoryJob.cpp \
#    src/kits/package/RepositoryCache.cpp \
#    src/kits/package/RepositoryConfig.cpp \
#    src/kits/package/RepositoryInfo.cpp \
#    src/kits/package/Request.cpp \
#    src/kits/package/TempfileManager.cpp \
#    src/kits/package/User.cpp \
#    src/kits/package/UserSettingsFileInfo.cpp \
#    src/kits/package/ValidateChecksumJob.cpp \
#    src/kits/print/BeUtils.cpp \
#    src/kits/print/BeUtilsTranslation.cpp \
#    src/kits/print/FolderWatcher.cpp \
#    src/kits/print/Jobs.cpp \
#    src/kits/print/JobSetupPanel.cpp \
#    src/kits/print/PictureIterator.cpp \
#    src/kits/print/PicturePrinter.cpp \
#    src/kits/print/PrintAddOnServer.cpp \
#    src/kits/print/PrintAddOnServerProtocol.cpp \
#    src/kits/print/Printer.cpp \
#    src/kits/print/PrinterDriverAddOn.cpp \
#    src/kits/print/PrinterRoster.cpp \
#    src/kits/print/PrintPanel.cpp \
#    src/kits/print/PrintTransport.cpp \
#    src/kits/print/PrintTransportAddOn.cpp \
#    src/kits/print/Template.cpp \
    src/kits/screensaver/BuildScreenSaverDefaultSettingsView.cpp \
    src/kits/screensaver/DefaultSettingsView.cpp \
    src/kits/screensaver/ScreenSaver.cpp \
    src/kits/screensaver/ScreenSaverRunner.cpp \
    src/kits/screensaver/ScreenSaverSettings.cpp \
    src/kits/shared/AboutMenuItem.cpp \
    src/kits/shared/ArgumentVector.cpp \
    src/kits/shared/AttributeUtilities.cpp \
    src/kits/shared/CalendarView.cpp \
    src/kits/shared/ColorQuantizer.cpp \
    src/kits/shared/CommandPipe.cpp \
    src/kits/shared/DragTrackingFilter.cpp \
    src/kits/shared/DriverSettingsMessageAdapter.cpp \
    src/kits/shared/ExpressionParser.cpp \
    src/kits/shared/Geolocation.cpp \
    src/kits/shared/HashString.cpp \
    src/kits/shared/IconButton.cpp \
    src/kits/shared/IconView.cpp \
    src/kits/shared/Json.cpp \
    src/kits/shared/Keymap.cpp \
    src/kits/shared/LongAndDragTrackingFilter.cpp \
    src/kits/shared/MessageBuilder.cpp \
    src/kits/shared/NaturalCompare.cpp \
    src/kits/shared/PromptWindow.cpp \
    src/kits/shared/QueryFile.cpp \
    src/kits/shared/RegExp.cpp \
    src/kits/shared/RWLocker.cpp \
    src/kits/shared/RWLockManager.cpp \
    src/kits/shared/SHA256.cpp \
    src/kits/shared/ShakeTrackingFilter.cpp \
    src/kits/shared/StringForRate.cpp \
    src/kits/shared/StringForSize.cpp \
    src/kits/shared/TextTable.cpp \
    src/kits/shared/Thread.cpp \
    src/kits/shared/ToolBar.cpp \
    src/kits/shared/Variant.cpp \
    src/kits/storage/disk_device/jobs/CreateChildJob.cpp \
    src/kits/storage/disk_device/jobs/DefragmentJob.cpp \
    src/kits/storage/disk_device/jobs/DeleteChildJob.cpp \
    src/kits/storage/disk_device/jobs/InitializeJob.cpp \
    src/kits/storage/disk_device/jobs/MoveJob.cpp \
    src/kits/storage/disk_device/jobs/RepairJob.cpp \
    src/kits/storage/disk_device/jobs/ResizeJob.cpp \
    src/kits/storage/disk_device/jobs/SetStringJob.cpp \
    src/kits/storage/disk_device/jobs/UninitializeJob.cpp \
    src/kits/storage/disk_device/DiskDevice.cpp \
    src/kits/storage/disk_device/DiskDeviceJob.cpp \
    src/kits/storage/disk_device/DiskDeviceJobGenerator.cpp \
    src/kits/storage/disk_device/DiskDeviceJobQueue.cpp \
    src/kits/storage/disk_device/DiskDeviceList.cpp \
    src/kits/storage/disk_device/DiskDevicePrivate.cpp \
    src/kits/storage/disk_device/DiskDeviceRoster.cpp \
    src/kits/storage/disk_device/DiskDeviceTypes.cpp \
    src/kits/storage/disk_device/DiskDeviceVisitor.cpp \
    src/kits/storage/disk_device/DiskScannerAddOn.cpp \
    src/kits/storage/disk_device/DiskSystem.cpp \
    src/kits/storage/disk_device/DiskSystemAddOn.cpp \
    src/kits/storage/disk_device/DiskSystemAddOnManager.cpp \
    src/kits/storage/disk_device/MutablePartition.cpp \
    src/kits/storage/disk_device/Partition.cpp \
    src/kits/storage/disk_device/PartitionDelegate.cpp \
    src/kits/storage/disk_device/PartitioningDialog.cpp \
    src/kits/storage/disk_device/PartitioningInfo.cpp \
    src/kits/storage/disk_device/PartitionParameterEditor.cpp \
    src/kits/storage/disk_device/PartitionReference.cpp \
    src/kits/storage/mime/AppMetaMimeCreator.cpp \
    src/kits/storage/mime/AssociatedTypes.cpp \
    src/kits/storage/mime/Database.cpp \
    src/kits/storage/mime/database_support.cpp \
    src/kits/storage/mime/DatabaseDirectory.cpp \
    src/kits/storage/mime/DatabaseLocation.cpp \
    src/kits/storage/mime/InstalledTypes.cpp \
    src/kits/storage/mime/MimeEntryProcessor.cpp \
    src/kits/storage/mime/MimeInfoUpdater.cpp \
    src/kits/storage/mime/MimeSniffer.cpp \
    src/kits/storage/mime/MimeSnifferAddon.cpp \
    src/kits/storage/mime/MimeSnifferAddonManager.cpp \
    src/kits/storage/mime/SnifferRules.cpp \
    src/kits/storage/mime/Supertype.cpp \
    src/kits/storage/mime/SupportingApps.cpp \
    src/kits/storage/mime/TextSnifferAddon.cpp \
    src/kits/storage/sniffer/CharStream.cpp \
    src/kits/storage/sniffer/DisjList.cpp \
    src/kits/storage/sniffer/Err.cpp \
    src/kits/storage/sniffer/Parser.cpp \
    src/kits/storage/sniffer/Pattern.cpp \
    src/kits/storage/sniffer/PatternList.cpp \
    src/kits/storage/sniffer/Range.cpp \
    src/kits/storage/sniffer/RPattern.cpp \
    src/kits/storage/sniffer/RPatternList.cpp \
    src/kits/storage/sniffer/Rule.cpp \
    src/kits/storage/AddOnImage.cpp \
    src/kits/storage/AddOnMonitor.cpp \
    src/kits/storage/AddOnMonitorHandler.cpp \
    src/kits/storage/AppFileInfo.cpp \
    src/kits/storage/CopyEngine.cpp \
    src/kits/storage/DriverSettings.cpp \
    src/kits/storage/EntryList.cpp \
    src/kits/storage/EntryOperationEngineBase.cpp \
    src/kits/storage/FdIO.cpp \
    src/kits/storage/FileDescriptorIO.cpp \
    src/kits/storage/FileIO.cpp \
    src/kits/storage/MergedDirectory.cpp \
    src/kits/storage/Mime.cpp \
    src/kits/storage/MimeType.cpp \
    src/kits/storage/NodeInfo.cpp \
    src/kits/storage/NodeMonitor.cpp \
    src/kits/storage/NodeMonitorHandler.cpp \
    src/kits/storage/OffsetFile.cpp \
    src/kits/storage/PathFinder.cpp \
    src/kits/storage/PathMonitor.cpp \
    src/kits/storage/Query.cpp \
    src/kits/storage/QueryPredicate.cpp \
    src/kits/storage/RemoveEngine.cpp \
    src/kits/storage/ResourceFile.cpp \
    src/kits/storage/ResourceItem.cpp \
    src/kits/storage/Resources.cpp \
    src/kits/storage/ResourcesContainer.cpp \
    src/kits/storage/ResourceStrings.cpp \
    src/kits/storage/Statable.cpp \
    src/kits/storage/storage_support.cpp \
    src/kits/storage/SymLink.cpp \
    src/kits/support/ArchivingManagers.cpp \
    src/kits/support/PointerList.cpp \
    src/kits/textencoding/character_sets.cpp \
    src/kits/textencoding/CharacterSet.cpp \
    src/kits/textencoding/CharacterSetRoster.cpp \
    src/kits/textencoding/utf8_conversions.cpp \
#    src/kits/tracker/AttributeStream.cpp \
#    src/kits/tracker/AutoMounterSettings.cpp \
#    src/kits/tracker/BackgroundImage.cpp \
#    src/kits/tracker/Bitmaps.cpp \
#    src/kits/tracker/ContainerWindow.cpp \
#    src/kits/tracker/CountView.cpp \
#    src/kits/tracker/DesktopPoseView.cpp \
#    src/kits/tracker/DeskWindow.cpp \
#    src/kits/tracker/DialogPane.cpp \
#    src/kits/tracker/DirMenu.cpp \
#    src/kits/tracker/EntryIterator.cpp \
#    src/kits/tracker/FavoritesMenu.cpp \
#    src/kits/tracker/FBCPadding.cpp \
#    src/kits/tracker/FilePanel.cpp \
#    src/kits/tracker/FilePanelPriv.cpp \
#    src/kits/tracker/FilePermissionsView.cpp \
#    src/kits/tracker/FindPanel.cpp \
#    src/kits/tracker/FSClipboard.cpp \
#    src/kits/tracker/FSUndoRedo.cpp \
#    src/kits/tracker/FSUtils.cpp \
#    src/kits/tracker/GroupedMenu.cpp \
#    src/kits/tracker/IconCache.cpp \
#    src/kits/tracker/IconMenuItem.cpp \
#    src/kits/tracker/InfoWindow.cpp \
#    src/kits/tracker/MimeTypeList.cpp \
#    src/kits/tracker/MiniMenuField.cpp \
#    src/kits/tracker/Model.cpp \
#    src/kits/tracker/MountMenu.cpp \
#    src/kits/tracker/Navigator.cpp \
#    src/kits/tracker/NavMenu.cpp \
#    src/kits/tracker/NodePreloader.cpp \
#    src/kits/tracker/NodeWalker.cpp \
#    src/kits/tracker/OpenWithWindow.cpp \
#    src/kits/tracker/OverrideAlert.cpp \
#    src/kits/tracker/PendingNodeMonitorCache.cpp \
#    src/kits/tracker/Pose.cpp \
#    src/kits/tracker/PoseList.cpp \
#    src/kits/tracker/PoseView.cpp \
#    src/kits/tracker/PoseViewScripting.cpp \
#    src/kits/tracker/QueryContainerWindow.cpp \
#    src/kits/tracker/QueryPoseView.cpp \
#    src/kits/tracker/RecentItems.cpp \
#    src/kits/tracker/RegExp.cpp \
#    src/kits/tracker/SelectionWindow.cpp \
#    src/kits/tracker/Settings.cpp \
#    src/kits/tracker/SettingsHandler.cpp \
#    src/kits/tracker/SettingsViews.cpp \
#    src/kits/tracker/SlowContextPopup.cpp \
#    src/kits/tracker/SlowMenu.cpp \
#    src/kits/tracker/StatusWindow.cpp \
#    src/kits/tracker/TaskLoop.cpp \
#    src/kits/tracker/TemplatesMenu.cpp \
#    src/kits/tracker/Tests.cpp \
#    src/kits/tracker/TextWidget.cpp \
#    src/kits/tracker/TitleView.cpp \
#    src/kits/tracker/Tracker.cpp \
#    src/kits/tracker/TrackerInitialState.cpp \
#    src/kits/tracker/TrackerScripting.cpp \
#    src/kits/tracker/TrackerSettings.cpp \
#    src/kits/tracker/TrackerSettingsWindow.cpp \
#    src/kits/tracker/TrackerString.cpp \
#    src/kits/tracker/TrashWatcher.cpp \
#    src/kits/tracker/Utilities.cpp \
#    src/kits/tracker/ViewState.cpp \
#    src/kits/tracker/VirtualDirectoryEntryList.cpp \
#    src/kits/tracker/VirtualDirectoryManager.cpp \
#    src/kits/tracker/VirtualDirectoryPoseView.cpp \
#    src/kits/tracker/VirtualDirectoryWindow.cpp \
#    src/kits/tracker/WidgetAttributeText.cpp \
#    src/kits/translation/BitmapStream.cpp \
#    src/kits/translation/FuncTranslator.cpp \
#    src/kits/translation/TranslationUtils.cpp \
#    src/kits/translation/Translator.cpp \
#    src/kits/translation/TranslatorRoster.cpp \
#    src/kits/mail/des.c \
    src/kits/network/libnetapi/md5.c \
    src/kits/network/netresolv/dst/dst_api.c \
    src/kits/network/netresolv/dst/hmac_link.c \
    src/kits/network/netresolv/dst/md5_dgst.c \
    src/kits/network/netresolv/dst/support.c \
    src/kits/network/netresolv/inet/inet_addr.c \
    src/kits/network/netresolv/inet/inet_cidr_ntop.c \
    src/kits/network/netresolv/inet/inet_cidr_pton.c \
    src/kits/network/netresolv/inet/inet_lnaof.c \
    src/kits/network/netresolv/inet/inet_makeaddr.c \
    src/kits/network/netresolv/inet/inet_net_ntop.c \
    src/kits/network/netresolv/inet/inet_net_pton.c \
    src/kits/network/netresolv/inet/inet_neta.c \
    src/kits/network/netresolv/inet/inet_netof.c \
    src/kits/network/netresolv/inet/inet_network.c \
    src/kits/network/netresolv/inet/inet_ntoa.c \
    src/kits/network/netresolv/inet/inet_ntop.c \
    src/kits/network/netresolv/inet/inet_pton.c \
    src/kits/network/netresolv/inet/nsap_addr.c \
    src/kits/network/netresolv/irs/base64.c \
    src/kits/network/netresolv/irs/getaddrinfo.c \
    src/kits/network/netresolv/irs/gethnamaddr.c \
    src/kits/network/netresolv/irs/getnameinfo.c \
    src/kits/network/netresolv/irs/getnetent.c \
    src/kits/network/netresolv/irs/getnetnamadr.c \
    src/kits/network/netresolv/irs/getprotobyname.c \
    src/kits/network/netresolv/irs/getprotobyname_r.c \
    src/kits/network/netresolv/irs/getprotobynumber.c \
    src/kits/network/netresolv/irs/getprotobynumber_r.c \
    src/kits/network/netresolv/irs/getprotoent.c \
    src/kits/network/netresolv/irs/getprotoent_r.c \
    src/kits/network/netresolv/irs/getservbyname.c \
    src/kits/network/netresolv/irs/getservbyname_r.c \
    src/kits/network/netresolv/irs/getservbyport.c \
    src/kits/network/netresolv/irs/getservbyport_r.c \
    src/kits/network/netresolv/irs/getservent.c \
    src/kits/network/netresolv/irs/getservent_r.c \
    src/kits/network/netresolv/irs/hesiod.c \
    src/kits/network/netresolv/irs/nsdispatch.c \
    src/kits/network/netresolv/irs/sethostent.c \
    src/kits/network/netresolv/irs/vars6.c \
    src/kits/network/netresolv/isc/assertions.c \
    src/kits/network/netresolv/isc/bitncmp.c \
    src/kits/network/netresolv/isc/ctl_clnt.c \
    src/kits/network/netresolv/isc/ctl_p.c \
    src/kits/network/netresolv/isc/ctl_srvr.c \
    src/kits/network/netresolv/isc/ev_connects.c \
    src/kits/network/netresolv/isc/ev_files.c \
    src/kits/network/netresolv/isc/ev_streams.c \
    src/kits/network/netresolv/isc/ev_timers.c \
    src/kits/network/netresolv/isc/ev_waits.c \
    src/kits/network/netresolv/isc/eventlib.c \
    src/kits/network/netresolv/isc/heap.c \
    src/kits/network/netresolv/isc/hex.c \
    src/kits/network/netresolv/isc/logging.c \
    src/kits/network/netresolv/isc/memcluster.c \
    src/kits/network/netresolv/isc/movefile.c \
    src/kits/network/netresolv/isc/tree.c \
    src/kits/network/netresolv/nameser/ns_name.c \
    src/kits/network/netresolv/nameser/ns_netint.c \
    src/kits/network/netresolv/nameser/ns_parse.c \
    src/kits/network/netresolv/nameser/ns_print.c \
    src/kits/network/netresolv/nameser/ns_samedomain.c \
    src/kits/network/netresolv/nameser/ns_ttl.c \
    src/kits/network/netresolv/resolv/h_errno.c \
    src/kits/network/netresolv/resolv/herror.c \
    src/kits/network/netresolv/resolv/mtctxres.c \
    src/kits/network/netresolv/resolv/res_comp.c \
    src/kits/network/netresolv/resolv/res_data.c \
    src/kits/network/netresolv/resolv/res_debug.c \
    src/kits/network/netresolv/resolv/res_init.c \
    src/kits/network/netresolv/resolv/res_mkquery.c \
    src/kits/network/netresolv/resolv/res_query.c \
    src/kits/network/netresolv/resolv/res_send.c \
    src/kits/network/netresolv/resolv/res_state.c \
    src/haiku/arch/generic/generic_atomic.cpp \
    src/haiku/arch/generic/generic_system_time_nsecs.cpp \
    src/haiku/locks/init_once.cpp \
    src/haiku/locks/mutex.cpp \
    src/haiku/locks/recursive_lock.cpp \
    src/haiku/locks/rw_lock.cpp \
    src/haiku/Architecture.cpp \
    src/haiku/byteorder.cpp \
    src/haiku/disk_scanner.cpp \
    src/haiku/driver_settings.cpp \
    src/haiku/errors.cpp \
    src/haiku/extended_system_info.cpp \
    src/haiku/find_directory.cpp \
    src/haiku/find_paths.cpp \
    src/haiku/fs.cpp \
    src/haiku/fs_attr.cpp \
    src/haiku/fs_attr_generic.cpp \
    src/haiku/fs_attr_untyped.cpp \
    src/haiku/fs_darwin.cpp \
    src/haiku/fs_descriptors.cpp \
    src/haiku/fs_freebsd.cpp \
    src/haiku/fs_query.cpp \
    src/haiku/function_remapper.cpp \
    src/haiku/image.cpp \
    src/haiku/launch.cpp \
    src/haiku/memory.cpp \
    src/haiku/misc.cpp \
    src/haiku/parsedate.cpp \
    src/haiku/syscalls.cpp \
    src/haiku/system_info.cpp \
    src/haiku/time.cpp \
    src/haiku/wait_for_objects.cpp \
    src/haiku/area.c \
    src/haiku/atomic.c \
    src/haiku/debug.c \
    src/haiku/fs_index.c \
    src/haiku/fs_info.c \
    src/haiku/fs_volume.c \
    src/haiku/port.c \
    src/haiku/scheduler.c \
    src/haiku/sem.c \
    src/haiku/system_revision.c \
    src/haiku/team.c \
    src/haiku/thread.c \
    src/wrappers/BHAPI_wrapper_port.cpp

HEADERS +=  \
    bhapi.h \
    bhapi_global.h \
    src/kits/BE.h \
    src/kits/AppKit.h \
    src/kits/InterfaceKit.h \
    src/kits/KernelKit.h \
    src/kits/NetKit.h \
    src/kits/RenderKit.h \
    src/kits/StorageKit.h \
    src/kits/SupportKit.h \
    src/kits/XmlKit.h \
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
    src/kits/net/NetAddress.h \
    src/kits/net/NetBuffer.h \
    src/kits/net/NetDebug.h \
    src/kits/net/NetEndpoint.h \
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
    src/kits/support/Flattenable.h \
    src/kits/support/Locker.h \
    src/kits/support/SimpleLocker.h \
    src/kits/support/StreamIO.h \
    src/kits/support/SupportDefs.h \
    src/kits/support/Beep.h \
    src/kits/support/BlockCache.h \
    src/kits/support/BufferedDataIO.h \
    src/kits/support/BufferIO.h \
    src/kits/support/DateTime.h \
    src/kits/support/Job.h \
    src/kits/support/ObjectList.h \
    src/kits/support/Referenceable.h \
    src/kits/support/StackOrHeapArray.h \
    src/kits/support/StandardIO.h \
    src/kits/support/StopWatch.h \
    src/kits/support/TLS.h \
    src/kits/support/UTF8.h \
    src/kits/support/Architecture.h \
    src/kits/support/StringArray.h \
    src/kits/support/StringList.h \
    src/private/kernel/Memory.h \
    src/private/app/MessageBody.h \
    src/private/support/Object.h \
    src/private/app/PrivateApplication.h \
    src/private/app/PrivateHandler.h \
    src/private/support/Token.h \
    src/private/support/StringPrivate.h \
    src/kits/support/StringClass.h \
    src/kits/add-ons/graphics/Accelerant.h \
    src/kits/add-ons/graphics/GraphicsCard.h \
    src/kits/add-ons/input_server/InputServerDevice.h \
    src/kits/add-ons/input_server/InputServerFilter.h \
    src/kits/add-ons/input_server/InputServerMethod.h \
    src/kits/add-ons/mail_daemon/MailFilter.h \
    src/kits/add-ons/mail_daemon/MailProtocol.h \
    src/kits/add-ons/mail_daemon/MailSettingsView.h \
    src/kits/add-ons/network_settings/NetworkProfile.h \
    src/kits/add-ons/network_settings/NetworkSettings.h \
    src/kits/add-ons/network_settings/NetworkSettingsAddOn.h \
    src/kits/add-ons/registrar/MimeSnifferAddon.h \
    src/kits/add-ons/screen_saver/ScreenSaver.h \
    src/kits/add-ons/tracker/TrackerAddOn.h \
    src/kits/app/Key.h \
    src/kits/app/KeyStore.h \
    src/kits/app/link_message.h \
    src/kits/app/Notification.h \
    src/kits/app/PropertyInfo.h \
    src/kits/app/Roster.h \
#    src/kits/bluetooth/HCI/btHCI.h \
#    src/kits/bluetooth/HCI/btHCI_acl.h \
#    src/kits/bluetooth/HCI/btHCI_command.h \
#    src/kits/bluetooth/HCI/btHCI_event.h \
#    src/kits/bluetooth/HCI/btHCI_sco.h \
#    src/kits/bluetooth/HCI/btHCI_transport.h \
#    src/kits/bluetooth/L2CAP/btL2CAP.h \
#    src/kits/bluetooth/bdaddrUtils.h \
#    src/kits/bluetooth/bluetooth.h \
#    src/kits/bluetooth/bluetooth_error.h \
#    src/kits/bluetooth/BluetoothDevice.h \
#    src/kits/bluetooth/DeviceClass.h \
#    src/kits/bluetooth/DiscoveryAgent.h \
#    src/kits/bluetooth/DiscoveryListener.h \
#    src/kits/bluetooth/KitSupport.h \
#    src/kits/bluetooth/LinkKeyUtils.h \
#    src/kits/bluetooth/LocalDevice.h \
#    src/kits/bluetooth/LocalDeviceAddOn.h \
#    src/kits/bluetooth/RemoteDevice.h \
    src/kits/debug/arch/arch_debug_support.h \
    src/kits/debug/image.h \
    src/kits/debug/SymbolLookup.h \
#    src/kits/debugger/arch/InstructionInfo.h \
#    src/kits/debugger/arch/RegisterMap.h \
#    src/kits/debugger/controllers/DebugReportGenerator.h \
#    src/kits/debugger/debug_info/loading_state_handlers/DwarfLoadingStateHandler.h \
#    src/kits/debugger/debug_info/BasicFunctionDebugInfo.h \
#    src/kits/debugger/debug_info/DebuggerImageDebugInfo.h \
#    src/kits/debugger/debug_info/DebuggerTeamDebugInfo.h \
#    src/kits/debugger/debug_info/DwarfFunctionDebugInfo.h \
#    src/kits/debugger/debug_info/DwarfImageDebugInfo.h \
#    src/kits/debugger/debug_info/DwarfImageDebugInfoLoadingState.h \
#    src/kits/debugger/debug_info/DwarfStackFrameDebugInfo.h \
#    src/kits/debugger/debug_info/DwarfTeamDebugInfo.h \
#    src/kits/debugger/debug_info/DwarfTypeFactory.h \
#    src/kits/debugger/debug_info/DwarfTypes.h \
#    src/kits/debugger/debug_info/ImageDebugInfoLoadingState.h \
#    src/kits/debugger/debug_info/ImageDebugLoadingStateHandler.h \
#    src/kits/debugger/debug_info/ImageDebugLoadingStateHandlerRoster.h \
#    src/kits/debugger/debug_info/NoOpStackFrameDebugInfo.h \
#    src/kits/debugger/debug_info/SpecificImageDebugInfoLoadingState.h \
#    src/kits/debugger/debug_managers/BreakpointManager.h \
#    src/kits/debugger/debug_managers/TeamMemoryBlockManager.h \
#    src/kits/debugger/debug_managers/WatchpointManager.h \
#    src/kits/debugger/debugger_interface/core/CoreFileDebuggerInterface.h \
#    src/kits/debugger/debugger_interface/local/LocalDebuggerInterface.h \
#    src/kits/debugger/debugger_interface/remote/RemoteDebugRequest.h \
#    src/kits/debugger/debugger_interface/DebugEvent.h \
#    src/kits/debugger/debugger_interface/DebuggerInterface.h \
#    src/kits/debugger/demangler/Demangler.h \
#    src/kits/debugger/dwarf/AbbreviationTable.h \
#    src/kits/debugger/dwarf/AttributeClasses.h \
#    src/kits/debugger/dwarf/AttributeValue.h \
#    src/kits/debugger/dwarf/BaseUnit.h \
#    src/kits/debugger/dwarf/CfaContext.h \
#    src/kits/debugger/dwarf/CfaRule.h \
#    src/kits/debugger/dwarf/CfaRuleSet.h \
#    src/kits/debugger/dwarf/CompilationUnit.h \
#    src/kits/debugger/dwarf/DataReader.h \
#    src/kits/debugger/dwarf/DebugInfoEntries.h \
#    src/kits/debugger/dwarf/DebugInfoEntry.h \
#    src/kits/debugger/dwarf/Dwarf.h \
#    src/kits/debugger/dwarf/DwarfExpressionEvaluator.h \
#    src/kits/debugger/dwarf/DwarfFile.h \
#    src/kits/debugger/dwarf/DwarfFileLoadingState.h \
#    src/kits/debugger/dwarf/DwarfManager.h \
#    src/kits/debugger/dwarf/DwarfTargetInterface.h \
#    src/kits/debugger/dwarf/DwarfUtils.h \
#    src/kits/debugger/dwarf/LineNumberProgram.h \
#    src/kits/debugger/dwarf/SourceLanguageInfo.h \
#    src/kits/debugger/dwarf/TagNames.h \
#    src/kits/debugger/dwarf/TypeUnit.h \
#    src/kits/debugger/elf/CoreFile.h \
#    src/kits/debugger/elf/ElfSymbolLookup.h \
#    src/kits/debugger/files/FileManager.h \
#    src/kits/debugger/jobs/Jobs.h \
#    src/kits/debugger/settings/BreakpointSetting.h \
#    src/kits/debugger/settings/NoOpSettingsManager.h \
#    src/kits/debugger/settings/TeamFileManagerSettings.h \
#    src/kits/debugger/settings/TeamSignalSettings.h \
#    src/kits/debugger/source_language/c_family/CLanguage.h \
#    src/kits/debugger/source_language/c_family/CLanguageExpressionEvaluator.h \
#    src/kits/debugger/source_language/c_family/CLanguageFamilySyntaxHighlighter.h \
#    src/kits/debugger/source_language/c_family/CLanguageFamilySyntaxHighlightInfo.h \
#    src/kits/debugger/source_language/c_family/CLanguageTokenizer.h \
#    src/kits/debugger/source_language/x86/X86AssemblyLanguage.h \
#    src/kits/debugger/source_language/UnsupportedLanguage.h \
#    src/kits/debugger/target_host_interface/local/LocalTargetHostInterface.h \
#    src/kits/debugger/target_host_interface/local/LocalTargetHostInterfaceInfo.h \
#    src/kits/debugger/util/ArchivingUtils.h \
#    src/kits/debugger/util/BitBuffer.h \
#    src/kits/debugger/value/type_handlers/BListTypeHandler.h \
#    src/kits/debugger/value/type_handlers/BMessageTypeHandler.h \
#    src/kits/debugger/value/type_handlers/CStringTypeHandler.h \
#    src/kits/debugger/value/ValueLoader.h \
#    src/kits/debugger/value/ValueWriter.h \
#    src/kits/device/A2D.h \
#    src/kits/device/CAM.h \
#    src/kits/device/D2A.h \
#    src/kits/device/DigitalPort.h \
#    src/kits/device/graphic_driver.h \
#    src/kits/device/Joystick.h \
#    src/kits/device/scsi.h \
#    src/kits/device/scsi_bus_raw_driver.h \
#    src/kits/device/SerialPort.h \
#    src/kits/device/USBKit.h \
#    src/kits/drivers/bus/ATA.h \
#    src/kits/drivers/bus/FDT.h \
#    src/kits/drivers/bus/IDE.h \
#    src/kits/drivers/bus/ISA.h \
#    src/kits/drivers/bus/PCI.h \
#    src/kits/drivers/bus/SCSI.h \
#    src/kits/drivers/pcmcia/bulkmem.h \
#    src/kits/drivers/pcmcia/bus_ops.h \
#    src/kits/drivers/pcmcia/cb_enabler.h \
#    src/kits/drivers/pcmcia/cisreg.h \
#    src/kits/drivers/pcmcia/cistpl.h \
#    src/kits/drivers/pcmcia/config.h \
#    src/kits/drivers/pcmcia/cs.h \
#    src/kits/drivers/pcmcia/cs_timer.h \
#    src/kits/drivers/pcmcia/cs_types.h \
#    src/kits/drivers/pcmcia/driver_ops.h \
#    src/kits/drivers/pcmcia/ds.h \
#    src/kits/drivers/pcmcia/ftl.h \
#    src/kits/drivers/pcmcia/k_compat.h \
#    src/kits/drivers/pcmcia/mem_op.h \
#    src/kits/drivers/pcmcia/memory.h \
#    src/kits/drivers/pcmcia/ss.h \
#    src/kits/drivers/pcmcia/version.h \
#    src/kits/drivers/tty/tty_module.h \
#    src/kits/drivers/tty/ttylayer.h \
#    src/kits/drivers/usb/USB_audio.h \
#    src/kits/drivers/usb/USB_cdc.h \
#    src/kits/drivers/usb/USB_hid.h \
#    src/kits/drivers/usb/USB_hid_page_alphanumeric_display.h \
#    src/kits/drivers/usb/USB_hid_page_arcade.h \
#    src/kits/drivers/usb/USB_hid_page_bar_code_scanner.h \
#    src/kits/drivers/usb/USB_hid_page_battery_system.h \
#    src/kits/drivers/usb/USB_hid_page_consumer.h \
#    src/kits/drivers/usb/USB_hid_page_digitizers.h \
#    src/kits/drivers/usb/USB_hid_page_game_controls.h \
#    src/kits/drivers/usb/USB_hid_page_generic_desktop.h \
#    src/kits/drivers/usb/USB_hid_page_generic_device_controls.h \
#    src/kits/drivers/usb/USB_hid_page_keyboard.h \
#    src/kits/drivers/usb/USB_hid_page_led.h \
#    src/kits/drivers/usb/USB_hid_page_magnetic_stripe_reader.h \
#    src/kits/drivers/usb/USB_hid_page_medical_instrument.h \
#    src/kits/drivers/usb/USB_hid_page_pid.h \
#    src/kits/drivers/usb/USB_hid_page_power_device.h \
#    src/kits/drivers/usb/USB_hid_page_simulation.h \
#    src/kits/drivers/usb/USB_hid_page_sport_controls.h \
#    src/kits/drivers/usb/USB_hid_page_telephony.h \
#    src/kits/drivers/usb/USB_hid_page_usb_monitor.h \
#    src/kits/drivers/usb/USB_hid_page_vesa_virtual_controls.h \
#    src/kits/drivers/usb/USB_hid_page_vr_controls.h \
#    src/kits/drivers/usb/USB_hid_page_weighing_device.h \
#    src/kits/drivers/usb/USB_langIDs.h \
#    src/kits/drivers/usb/USB_massbulk.h \
#    src/kits/drivers/usb/USB_midi.h \
#    src/kits/drivers/usb/USB_video.h \
#    src/kits/drivers/ACPI.h \
#    src/kits/drivers/atomizer.h \
#    src/kits/drivers/bios.h \
#    src/kits/drivers/bus_manager.h \
#    src/kits/drivers/config_manager.h \
#    src/kits/drivers/cpufreq.h \
#    src/kits/drivers/cpuidle.h \
#    src/kits/drivers/device_manager.h \
#    src/kits/drivers/disk_device_manager.h \
#    src/kits/drivers/dpc.h \
#    src/kits/drivers/driver_settings.h \
#    src/kits/drivers/Drivers.h \
#    src/kits/drivers/fs_cache.h \
#    src/kits/drivers/fs_interface.h \
#    src/kits/drivers/interrupt_controller.h \
#    src/kits/drivers/io_requests.h \
#    src/kits/drivers/ISA.h \
#    src/kits/drivers/isapnp.h \
#    src/kits/drivers/KernelExport.h \
#    src/kits/drivers/locked_pool.h \
#    src/kits/drivers/midi_driver.h \
#    src/kits/drivers/mime_table.h \
#    src/kits/drivers/module.h \
#    src/kits/drivers/PCI.h \
#    src/kits/drivers/PCI_x86.h \
#    src/kits/drivers/Select.h \
#    src/kits/drivers/USB.h \
#    src/kits/drivers/USB2.h \
#    src/kits/drivers/USB3.h \
#    src/kits/drivers/USB_printer.h \
#    src/kits/drivers/USB_rle.h \
#    src/kits/drivers/USB_spec.h \
#    src/kits/game/DirectWindow.h \
#    src/kits/game/FileGameSound.h \
#    src/kits/game/GameProducer.h \
#    src/kits/game/GameSound.h \
#    src/kits/game/GameSoundBuffer.h \
#    src/kits/game/GameSoundDefs.h \
#    src/kits/game/GameSoundDevice.h \
#    src/kits/game/GSUtility.h \
#    src/kits/game/PushGameSound.h \
#    src/kits/game/SimpleGameSound.h \
#    src/kits/game/StreamingGameSound.h \
#    src/kits/game/WindowScreen.h \
    src/kits/interface/carbon/etk-carbon.h \
    src/kits/interface/layouter/CollapsingLayouter.h \
    src/kits/interface/layouter/ComplexLayouter.h \
    src/kits/interface/layouter/Layouter.h \
    src/kits/interface/layouter/LayoutOptimizer.h \
    src/kits/interface/layouter/OneElementLayouter.h \
    src/kits/interface/layouter/SimpleLayouter.h \
    src/kits/interface/textview_support/InlineInput.h \
    src/kits/interface/textview_support/LineBuffer.h \
    src/kits/interface/textview_support/StyleBuffer.h \
    src/kits/interface/textview_support/TextGapBuffer.h \
    src/kits/interface/textview_support/UndoBuffer.h \
    src/kits/interface/AbstractLayout.h \
    src/kits/interface/AbstractLayoutItem.h \
    src/kits/interface/AffineTransform.h \
    src/kits/interface/Alignment.h \
    src/kits/interface/CardLayout.h \
    src/kits/interface/CardView.h \
    src/kits/interface/ChannelControl.h \
    src/kits/interface/ChannelSlider.h \
    src/kits/interface/ControlLook.h \
    src/kits/interface/Deskbar.h \
    src/kits/interface/Dragger.h \
    src/kits/interface/Gradient.h \
    src/kits/interface/GradientConic.h \
    src/kits/interface/GradientDiamond.h \
    src/kits/interface/GradientLinear.h \
    src/kits/interface/GradientRadial.h \
    src/kits/interface/GradientRadialFocus.h \
    src/kits/interface/GridLayout.h \
    src/kits/interface/GridLayoutBuilder.h \
    src/kits/interface/GridView.h \
    src/kits/interface/GroupLayout.h \
    src/kits/interface/GroupLayoutBuilder.h \
    src/kits/interface/GroupView.h \
    src/kits/interface/IconUtils.h \
    src/kits/interface/Layout.h \
    src/kits/interface/LayoutBuilder.h \
    src/kits/interface/LayoutContext.h \
    src/kits/interface/LayoutItem.h \
    src/kits/interface/LayoutUtils.h \
    src/kits/interface/MultiChannelControl.h \
    src/kits/interface/OptionControl.h \
    src/kits/interface/OptionPopUp.h \
    src/kits/interface/Picture.h \
    src/kits/interface/PictureButton.h \
    src/kits/interface/PrintJob.h \
    src/kits/interface/SeparatorItem.h \
    src/kits/interface/SeparatorView.h \
    src/kits/interface/Shape.h \
    src/kits/interface/Shelf.h \
    src/kits/interface/Size.h \
    src/kits/interface/Slider.h \
    src/kits/interface/SpaceLayoutItem.h \
    src/kits/interface/SplitLayout.h \
    src/kits/interface/SplitLayoutBuilder.h \
    src/kits/interface/SplitView.h \
    src/kits/interface/StringItem.h \
    src/kits/interface/TextInput.h \
    src/kits/interface/TwoDimensionalLayout.h \
    src/kits/interface/UnicodeBlockObjects.h \
    src/kits/interface/ViewLayoutItem.h \
    src/kits/kernel/debugger.h \
    src/kits/kernel/elf.h \
    src/kits/kernel/fs_attr.h \
    src/kits/kernel/fs_index.h \
    src/kits/kernel/fs_info.h \
    src/kits/kernel/fs_query.h \
    src/kits/kernel/fs_volume.h \
    src/kits/kernel/Memory.h \
#    src/kits/locale/tools/CollectingCatalog.h \
#    src/kits/locale/Catalog.h \
#    src/kits/locale/CatalogData.h \
#    src/kits/locale/Collator.h \
#    src/kits/locale/Country.h \
#    src/kits/locale/Currency.h \
#    src/kits/locale/DateFormat.h \
#    src/kits/locale/DateTimeFormat.h \
#    src/kits/locale/DurationFormat.h \
#    src/kits/locale/FloatFormat.h \
#    src/kits/locale/FloatFormatImpl.h \
#    src/kits/locale/FloatFormatParameters.h \
#    src/kits/locale/Format.h \
#    src/kits/locale/FormatImpl.h \
#    src/kits/locale/FormatParameters.h \
#    src/kits/locale/FormattingConventions.h \
#    src/kits/locale/GenericNumberFormat.h \
#    src/kits/locale/IntegerFormat.h \
#    src/kits/locale/IntegerFormatImpl.h \
#    src/kits/locale/IntegerFormatParameters.h \
#    src/kits/locale/Language.h \
#    src/kits/locale/LocaleClass.h \
#    src/kits/locale/LocaleRoster.h \
#    src/kits/locale/LocaleStrings.h \
#    src/kits/locale/MessageFormat.h \
#    src/kits/locale/NumberFormat.h \
#    src/kits/locale/NumberFormatParameters.h \
#    src/kits/locale/TextEncoding.h \
#    src/kits/locale/TimeFormat.h \
#    src/kits/locale/TimeUnitFormat.h \
#    src/kits/locale/TimeZone.h \
#    src/kits/locale/UnicodeChar.h \
#    src/kits/mail/E-mail.h \
#    src/kits/mail/HaikuMailFormatFilter.h \
#    src/kits/mail/mail_encoding.h \
#    src/kits/mail/MailAttachment.h \
#    src/kits/mail/MailComponent.h \
#    src/kits/mail/MailContainer.h \
#    src/kits/mail/MailDaemon.h \
#    src/kits/mail/MailMessage.h \
#    src/kits/mail/MailSettings.h \
#    src/kits/media/legacy/OldAudioModule.h \
#    src/kits/media/legacy/OldAudioStream.h \
#    src/kits/media/legacy/OldBufferMsgs.h \
#    src/kits/media/legacy/OldBufferStream.h \
#    src/kits/media/legacy/OldBufferStreamManager.h \
#    src/kits/media/legacy/OldMediaDefs.h \
#    src/kits/media/legacy/OldMediaKit.h \
#    src/kits/media/legacy/OldMediaModule.h \
#    src/kits/media/legacy/OldSubscriber.h \
#    src/kits/media/AddOnManager.h \
#    src/kits/media/Buffer.h \
#    src/kits/media/BufferCache.h \
#    src/kits/media/BufferConsumer.h \
#    src/kits/media/BufferGroup.h \
#    src/kits/media/BufferProducer.h \
#    src/kits/media/ChunkCache.h \
#    src/kits/media/Controllable.h \
#    src/kits/media/FileInterface.h \
#    src/kits/media/FormatManager.h \
#    src/kits/media/MediaAddOn.h \
#    src/kits/media/MediaDecoder.h \
#    src/kits/media/MediaDefs.h \
#    src/kits/media/MediaEncoder.h \
#    src/kits/media/MediaEventLooper.h \
#    src/kits/media/MediaFile.h \
#    src/kits/media/MediaFiles.h \
#    src/kits/media/MediaFormats.h \
#    src/kits/media/MediaNode.h \
#    src/kits/media/MediaRoster.h \
#    src/kits/media/MediaTheme.h \
#    src/kits/media/MediaTrack.h \
#    src/kits/media/ParameterWeb.h \
#    src/kits/media/PlaySound.h \
#    src/kits/media/RealtimeAlloc.h \
#    src/kits/media/Sound.h \
#    src/kits/media/SoundFile.h \
#    src/kits/media/SoundPlayer.h \
#    src/kits/media/SoundPlayNode.h \
#    src/kits/media/TimeCode.h \
#    src/kits/media/TimedEventQueue.h \
#    src/kits/media/TimedEventQueuePrivate.h \
#    src/kits/media/TimeSource.h \
#    src/kits/media/TimeSourceObject.h \
#    src/kits/media/TimeSourceObjectManager.h \
#    src/kits/media/TrackReader.h \
#    src/kits/midi/Midi.h \
#    src/kits/midi/MidiDefs.h \
#    src/kits/midi/MidiGlue.h \
#    src/kits/midi/MidiPort.h \
#    src/kits/midi/MidiStore.h \
#    src/kits/midi/MidiSynth.h \
#    src/kits/midi/MidiSynthFile.h \
#    src/kits/midi/MidiText.h \
#    src/kits/midi/Samples.h \
#    src/kits/midi/SoftSynth.h \
#    src/kits/midi/Synth.h \
#    src/kits/midi2/Midi2Defs.h \
#    src/kits/midi2/MidiConsumer.h \
#    src/kits/midi2/MidiEndpoint.h \
#    src/kits/midi2/MidiProducer.h \
#    src/kits/midi2/MidiRoster.h \
#    src/kits/midi2/MidiRosterLooper.h \
    src/kits/net/AbstractSocket.h \
    src/kits/net/Certificate.h \
    src/kits/net/DatagramSocket.h \
    src/kits/net/DataRequest.h \
    src/kits/net/FileRequest.h \
    src/kits/net/GopherRequest.h \
    src/kits/net/HttpAuthentication.h \
    src/kits/net/HttpForm.h \
    src/kits/net/HttpHeaders.h \
    src/kits/net/HttpRequest.h \
    src/kits/net/HttpResult.h \
    src/kits/net/net_settings.h \
    src/kits/net/NetworkAddress.h \
    src/kits/net/NetworkAddressResolver.h \
    src/kits/net/NetworkCookie.h \
    src/kits/net/NetworkCookieJar.h \
    src/kits/net/NetworkDevice.h \
    src/kits/net/NetworkInterface.h \
    src/kits/net/NetworkNotifications.h \
    src/kits/net/NetworkRequest.h \
    src/kits/net/NetworkRoster.h \
    src/kits/net/NetworkRoute.h \
    src/kits/net/ProxySecureSocket.h \
    src/kits/net/SecureSocket.h \
    src/kits/net/Socket.h \
    src/kits/net/Url.h \
    src/kits/net/UrlContext.h \
    src/kits/net/UrlProtocolAsynchronousListener.h \
    src/kits/net/UrlProtocolDispatchingListener.h \
    src/kits/net/UrlProtocolListener.h \
    src/kits/net/UrlProtocolRoster.h \
    src/kits/net/UrlRequest.h \
    src/kits/net/UrlResult.h \
    src/kits/net/UrlSynchronousRequest.h \
    src/kits/network/libnetapi/CertificatePrivate.h \
    src/kits/network/libnetapi/md5.h \
    src/kits/network/libnetapi/NetworkCookieJarPrivate.h \
    src/kits/network/netresolv/dst/dst_internal.h \
    src/kits/network/netresolv/dst/md5.h \
    src/kits/network/netresolv/dst/md5_locl.h \
    src/kits/network/netresolv/include/isc/assertions.h \
    src/kits/network/netresolv/include/isc/ctl.h \
    src/kits/network/netresolv/include/isc/dst.h \
    src/kits/network/netresolv/include/isc/eventlib.h \
    src/kits/network/netresolv/include/isc/heap.h \
    src/kits/network/netresolv/include/isc/irpmarshall.h \
    src/kits/network/netresolv/include/isc/list.h \
    src/kits/network/netresolv/include/isc/logging.h \
    src/kits/network/netresolv/include/isc/memcluster.h \
    src/kits/network/netresolv/include/isc/misc.h \
    src/kits/network/netresolv/include/isc/platform.h \
    src/kits/network/netresolv/include/isc/tree.h \
    src/kits/network/netresolv/include/sys/bitypes.h \
    src/kits/network/netresolv/include/fd_setsize.h \
    src/kits/network/netresolv/include/hesiod.h \
    src/kits/network/netresolv/include/irp.h \
    src/kits/network/netresolv/include/irs.h \
    src/kits/network/netresolv/include/netgroup.h \
    src/kits/network/netresolv/include/nsswitch.h \
    src/kits/network/netresolv/include/res_update.h \
    src/kits/network/netresolv/include/resolv_mt.h \
    src/kits/network/netresolv/include/utmp.h \
    src/kits/network/netresolv/irs/protoent.h \
    src/kits/network/netresolv/irs/servent.h \
    src/kits/network/netresolv/isc/ctl_p.h \
    src/kits/network/netresolv/isc/eventlib_p.h \
    src/kits/network/netresolv/isc/logging_p.h \
    src/kits/network/netresolv/resolv/res_debug.h \
    src/kits/network/netresolv/resolv/res_private.h \
    src/kits/network/netresolv/config.h \
    src/kits/network/netresolv/port_after.h \
    src/kits/network/netresolv/port_before.h \
#    src/kits/package/hpkg/v1/HPKGDefs.h \
#    src/kits/package/hpkg/v1/PackageAttributeValue.h \
#    src/kits/package/hpkg/v1/PackageContentHandler.h \
#    src/kits/package/hpkg/v1/PackageData.h \
#    src/kits/package/hpkg/v1/PackageDataReader.h \
#    src/kits/package/hpkg/v1/PackageEntry.h \
#    src/kits/package/hpkg/v1/PackageEntryAttribute.h \
#    src/kits/package/hpkg/v1/PackageReader.h \
#    src/kits/package/hpkg/BlockBufferPool.h \
#    src/kits/package/hpkg/BlockBufferPoolNoLock.h \
#    src/kits/package/hpkg/BufferPool.h \
#    src/kits/package/hpkg/DataReader.h \
#    src/kits/package/hpkg/ErrorOutput.h \
#    src/kits/package/hpkg/HPKGDefs.h \
#    src/kits/package/hpkg/NoErrorOutput.h \
#    src/kits/package/hpkg/PackageAttributes.h \
#    src/kits/package/hpkg/PackageAttributeValue.h \
#    src/kits/package/hpkg/PackageContentHandler.h \
#    src/kits/package/hpkg/PackageData.h \
#    src/kits/package/hpkg/PackageDataReader.h \
#    src/kits/package/hpkg/PackageEntry.h \
#    src/kits/package/hpkg/PackageEntryAttribute.h \
#    src/kits/package/hpkg/PackageInfoAttributeValue.h \
#    src/kits/package/hpkg/PackageReader.h \
#    src/kits/package/hpkg/PackageWriter.h \
#    src/kits/package/hpkg/RepositoryContentHandler.h \
#    src/kits/package/hpkg/RepositoryReader.h \
#    src/kits/package/hpkg/RepositoryWriter.h \
#    src/kits/package/hpkg/StandardErrorOutput.h \
#    src/kits/package/manager/PackageManagerUtils.h \
#    src/kits/package/solver/LibsolvSolver.h \
#    src/kits/package/solver/Solver.h \
#    src/kits/package/solver/SolverPackage.h \
#    src/kits/package/solver/SolverPackageSpecifier.h \
#    src/kits/package/solver/SolverPackageSpecifierList.h \
#    src/kits/package/solver/SolverProblem.h \
#    src/kits/package/solver/SolverProblemSolution.h \
#    src/kits/package/solver/SolverRepository.h \
#    src/kits/package/solver/SolverResult.h \
#    src/kits/package/AddRepositoryRequest.h \
#    src/kits/package/Attributes.h \
#    src/kits/package/CommitTransactionResult.h \
#    src/kits/package/Context.h \
#    src/kits/package/DownloadFileRequest.h \
#    src/kits/package/DropRepositoryRequest.h \
#    src/kits/package/GlobalWritableFileInfo.h \
#    src/kits/package/InstallationLocationInfo.h \
#    src/kits/package/Job.h \
#    src/kits/package/PackageArchitecture.h \
#    src/kits/package/PackageDefs.h \
#    src/kits/package/PackageFlags.h \
#    src/kits/package/PackageInfo.h \
#    src/kits/package/PackageInfoAttributes.h \
#    src/kits/package/PackageInfoContentHandler.h \
#    src/kits/package/PackageInfoParser.h \
#    src/kits/package/PackageInfoSet.h \
#    src/kits/package/PackageInfoStringBuilder.h \
#    src/kits/package/PackageResolvable.h \
#    src/kits/package/PackageResolvableExpression.h \
#    src/kits/package/PackageResolvableOperator.h \
#    src/kits/package/PackageRoster.h \
#    src/kits/package/PackageVersion.h \
#    src/kits/package/RefreshRepositoryRequest.h \
#    src/kits/package/RepositoryCache.h \
#    src/kits/package/RepositoryConfig.h \
#    src/kits/package/RepositoryInfo.h \
#    src/kits/package/Request.h \
#    src/kits/package/User.h \
#    src/kits/package/UserSettingsFileInfo.h \
#    src/kits/package/WritableFileUpdateType.h \
    src/kits/storage/disk_device/jobs/CreateChildJob.h \
    src/kits/storage/disk_device/jobs/DefragmentJob.h \
    src/kits/storage/disk_device/jobs/DeleteChildJob.h \
    src/kits/storage/disk_device/jobs/InitializeJob.h \
    src/kits/storage/disk_device/jobs/MoveJob.h \
    src/kits/storage/disk_device/jobs/RepairJob.h \
    src/kits/storage/disk_device/jobs/ResizeJob.h \
    src/kits/storage/disk_device/jobs/SetStringJob.h \
    src/kits/storage/disk_device/jobs/UninitializeJob.h \
    src/kits/storage/disk_device/DiskDeviceJob.h \
    src/kits/storage/disk_device/DiskDeviceJobGenerator.h \
    src/kits/storage/disk_device/DiskDeviceJobQueue.h \
    src/kits/storage/disk_device/DiskDeviceUtils.h \
    src/kits/storage/disk_device/PartitionDelegate.h \
    src/kits/storage/disk_device/PartitioningDialog.h \
    src/kits/storage/disk_device/PartitionReference.h \
    src/kits/storage/mime/names.h \
    src/kits/storage/AddOnImage.h \
    src/kits/storage/AppFileInfo.h \
    src/kits/storage/DiskDeviceDefs.h \
    src/kits/storage/EntryList.h \
    src/kits/storage/Mime.h \
    src/kits/storage/MimeType.h \
    src/kits/storage/NodeInfo.h \
    src/kits/storage/NodeMonitor.h \
    src/kits/storage/PathFinder.h \
    src/kits/storage/Query.h \
    src/kits/storage/Resources.h \
    src/kits/storage/ResourceStrings.h \
    src/kits/storage/Statable.h \
    src/kits/storage/SymLink.h \
    src/kits/support/ArchivingManagers.h \
    src/kits/support/Debug.h \
    src/kits/support/parsedate.h \
    src/kits/textencoding/character_sets.h \
#    src/kits/tracker/Attributes.h \
#    src/kits/tracker/AttributeStream.h \
#    src/kits/tracker/Background.h \
#    src/kits/tracker/BackgroundImage.h \
#    src/kits/tracker/Bitmaps.h \
#    src/kits/tracker/Commands.h \
#    src/kits/tracker/ContainerWindow.h \
#    src/kits/tracker/CountView.h \
#    src/kits/tracker/Cursors.h \
#    src/kits/tracker/DesktopPoseView.h \
#    src/kits/tracker/DeskWindow.h \
#    src/kits/tracker/DialogPane.h \
#    src/kits/tracker/EntryIterator.h \
#    src/kits/tracker/FavoritesMenu.h \
#    src/kits/tracker/FilePanelPriv.h \
#    src/kits/tracker/FilePermissionsView.h \
#    src/kits/tracker/FindPanel.h \
#    src/kits/tracker/FSClipboard.h \
#    src/kits/tracker/FSUndoRedo.h \
#    src/kits/tracker/FSUtils.h \
#    src/kits/tracker/GroupedMenu.h \
#    src/kits/tracker/IconCache.h \
#    src/kits/tracker/InfoWindow.h \
#    src/kits/tracker/LocalizedFolders.h \
#    src/kits/tracker/LockingList.h \
#    src/kits/tracker/MimeTypeList.h \
#    src/kits/tracker/MimeTypes.h \
#    src/kits/tracker/MiniMenuField.h \
#    src/kits/tracker/Model.h \
#    src/kits/tracker/MountMenu.h \
#    src/kits/tracker/Navigator.h \
#    src/kits/tracker/NodePreloader.h \
#    src/kits/tracker/NodeWalker.h \
#    src/kits/tracker/OpenWithWindow.h \
#    src/kits/tracker/OverrideAlert.h \
#    src/kits/tracker/PendingNodeMonitorCache.h \
#    src/kits/tracker/Pose.h \
#    src/kits/tracker/PoseList.h \
#    src/kits/tracker/PoseView.h \
#    src/kits/tracker/QueryContainerWindow.h \
#    src/kits/tracker/QueryPoseView.h \
#    src/kits/tracker/RecentItems.h \
#    src/kits/tracker/RegExp.h \
#    src/kits/tracker/SelectionWindow.h \
#    src/kits/tracker/Settings.h \
#    src/kits/tracker/SettingsHandler.h \
#    src/kits/tracker/SettingsViews.h \
#    src/kits/tracker/SlowContextPopup.h \
#    src/kits/tracker/SlowMenu.h \
#    src/kits/tracker/StatusWindow.h \
#    src/kits/tracker/TaskLoop.h \
#    src/kits/tracker/TemplatesMenu.h \
#    src/kits/tracker/Tests.h \
#    src/kits/tracker/TextWidget.h \
#    src/kits/tracker/TitleView.h \
#    src/kits/tracker/Tracker.h \
#    src/kits/tracker/TrackerIcons.h \
#    src/kits/tracker/TrackerSettings.h \
#    src/kits/tracker/TrackerSettingsWindow.h \
#    src/kits/tracker/TrackerString.h \
#    src/kits/tracker/TrashWatcher.h \
#    src/kits/tracker/Utilities.h \
#    src/kits/tracker/ViewState.h \
#    src/kits/tracker/VirtualDirectoryEntryList.h \
#    src/kits/tracker/VirtualDirectoryManager.h \
#    src/kits/tracker/VirtualDirectoryPoseView.h \
#    src/kits/tracker/VirtualDirectoryWindow.h \
#    src/kits/tracker/WidgetAttributeText.h \
#    src/kits/translation/BitmapStream.h \
#    src/kits/translation/FuncTranslator.h \
#    src/kits/translation/TranslationDefs.h \
#    src/kits/translation/TranslationErrors.h \
#    src/kits/translation/TranslationUtils.h \
#    src/kits/translation/Translator.h \
#    src/kits/translation/TranslatorAddOn.h \
#    src/kits/translation/TranslatorFormats.h \
#    src/kits/translation/TranslatorRoster.h \
#    src/kits/translation/TranslatorRosterPrivate.h \
    src/kits/xml/SimpleXmlParser.h \
    src/kits/DeviceKit.h \
    src/kits/GameKit.h \
    src/kits/LocaleKit.h \
    src/kits/MailKit.h \
    src/kits/MediaKit.h \
    src/kits/MidiKit.h \
    src/kits/NetworkKit.h \
    src/kits/TranslationKit.h \
    src/HaikuConstants.h \
    src/HaikuTypes.h \
    src/Haiku.h \
    src/HaikuDefs.h \
    src/HaikuConfig.h \
    src/haiku/fs_attr_bsdxattr.h \
    src/haiku/fs_attr_extattr.h \
    src/haiku/fs_attr_xattr.h \
    src/haiku/fs_descriptors.h \
    src/haiku/fs_freebsd.h \
    src/haiku/fs_impl.h \
    src/haiku/remapped_functions.h \
    src/haiku/fs_node_ref.h \
    src/haiku/fs_path_buffer.h \
    src/kits/kernel/OS.h \
    src/kits/kernel/Scheduler.h \
    src/kits/kernel/Image.h

LIBS -= -lfreetype
LIBS += -L"../freetype/lib/debug" -libfreetype


contains(QMAKE_COMPILER_DEFINES, __INTEL__) {
SOURCES += \
    src/kits/debug/arch/x86/arch_debug_support.cpp \
#    src/kits/debugger/arch/x86/disasm/DisassemblerX86.cpp \
#    src/kits/debugger/arch/x86/ArchitectureX86.cpp \
#    src/kits/debugger/arch/x86/CpuStateX86.cpp \
    src/haiku/arch/x86/time.cpp \
    src/haiku/arch/x86_common/stack_trace.cpp \
    src/haiku/arch/x86/compatibility.c \
    src/haiku/arch/x86/system_info.c \
    src/haiku/arch/x86/system_time.c \
    src/haiku/arch/x86/thread.c \
    src/haiku/arch/x86/tls.c
HEADERS += \
    src/kits/arch/x86/arch_debugger.h \
#    src/kits/debugger/arch/x86/disasm/DisassemblerX86.h \
#    src/kits/debugger/arch/x86/ArchitectureX86.h \
#    src/kits/debugger/arch/x86/CpuStateX86.h
INCLUDEPATH += src/kits/arch/x86
INCLUDEPATH += src/kits/debugger/arch/x86
INCLUDEPATH += src/libs/musl/include
INCLUDEPATH += src/libs/musl/arch/x86
}
contains(QMAKE_COMPILER_DEFINES, __x86_64__) {
SOURCES += \
    src/kits/debug/arch/x86_64/arch_debug_support.cpp \
#    src/kits/debugger/arch/x86_64/disasm/DisassemblerX8664.cpp \
#    src/kits/debugger/arch/x86_64/ArchitectureX8664.cpp \
#    src/kits/debugger/arch/x86_64/CpuStateX8664.cpp \
    src/haiku/arch/x86_64/system_info.cpp \
    src/haiku/arch/x86_64/system_time.cpp \
    src/haiku/arch/x86_64/thread.cpp \
    src/haiku/arch/x86_64/time.cpp \
    src/haiku/arch/x86_64/tls.cpp
HEADERS += \
    src/kits/arch/x86_64/arch_debugger.h \
#    src/kits/debugger/arch/x86_64/disasm/DisassemblerX8664.h \
#    src/kits/debugger/arch/x86_64/ArchitectureX8664.h \
#    src/kits/debugger/arch/x86_64/CpuStateX8664.h
}
contains(QMAKE_COMPILER_DEFINES, __MIPSEL__) {
SOURCES += \
    src/kits/debug/arch/mipsel/arch_debug_support.cpp
HEADERS += \
    src/kits/arch/mipsel/arch_debugger.h
}
contains(QMAKE_COMPILER_DEFINES, __POWERPC__) {
SOURCES += \
    src/kits/debug/arch/ppc/arch_debug_support.cpp \
    src/haiku/arch/ppc/compatibility.c \
    src/haiku/arch/ppc/stack_frame.c \
    src/haiku/arch/ppc/system_time.c \
    src/haiku/arch/ppc/thread.c \
    src/haiku/arch/ppc/time.c \
    src/haiku/arch/ppc/tls.c
HEADERS += \
    src/kits/arch/ppc/arch_debugger.h
}

contains(QMAKE_COMPILER_DEFINES, __M68K__) {
SOURCES += \
    src/kits/debug/arch/m68k/arch_debug_support.cpp \
    src/haiku/arch/m68k/compatibility.c \
    src/haiku/arch/m68k/stack_frame.c \
    src/haiku/arch/m68k/system_time.c \
    src/haiku/arch/m68k/thread.c \
    src/haiku/arch/m68k/time.c \
    src/haiku/arch/m68k/tls.c
HEADERS += \
    src/kits/arch/m68k/arch_debugger.h
}

contains(QMAKE_COMPILER_DEFINES, __ARM__) {
SOURCES += \
    src/kits/debug/arch/arm/arch_debug_support.cpp \
    src/haiku/arch/arm/stack_frame.c \
    src/haiku/arch/arm/stdatomic.c \
    src/haiku/arch/arm/system_time.c \
    src/haiku/arch/arm/thread.c \
    src/haiku/arch/arm/time.c \
    src/haiku/arch/arm/tls.c
HEADERS += \
    src/kits/arch/arm/arch_debugger.h
}

contains(QMAKE_COMPILER_DEFINES, BEOS) {
SOURCES +=  src/wrappers/beos/BHAPI_wrapper_application.cpp \
    src/wrappers/beos/BHAPI_wrapper_area.cpp \
    src/wrappers/beos/BHAPI_wrapper_image.cpp \
    src/wrappers/beos/BHAPI_wrapper_os.cpp \
    src/wrappers/beos/BHAPI_wrapper_timefuncs.cpp \
    src/wrappers/beos/BHAPI_wrapper_locker.cpp \
    src/wrappers/beos/BHAPI_wrapper_semaphore.cpp \
    src/wrappers/beos/BHAPI_wrapper_thread.cpp \
    src/wrappers/beos/ui/BHAPI_wrapper_beos-font.cpp \
    src/wrappers/beos/ui/BHAPI_wrapper_drawing.cpp \
    src/wrappers/beos/ui/BHAPI_wrapper_pixmap.cpp \
    src/wrappers/beos/ui/BHAPI_wrapper_window.cpp

HEADERS += src/wrappers/beos/ui/BHAPI_wrapper_beos-graphics.h
}

contains(QMAKE_COMPILER_DEFINES, WIN32) {
SOURCES += src/wrappers/win32/GDI/BHAPI_wrapper_application.cpp \
    src/wrappers/win32/GDI/BHAPI_wrapper_drawing.cpp \
    src/wrappers/win32/GDI/BHAPI_wrapper_pixmap.cpp \
    src/wrappers/win32/GDI/BHAPI_wrapper_win32-font.cpp \
    src/wrappers/win32/GDI/BHAPI_wrapper_window.cpp \
    src/wrappers/win32/BHAPI_wrapper_locker.cpp \
    src/wrappers/win32/BHAPI_wrapper_semaphore.cpp \
    src/wrappers/win32/BHAPI_wrapper_thread.cpp \
    src/wrappers/win32/BHAPI_wrapper_area.cpp \
    src/wrappers/win32/BHAPI_wrapper_image.cpp \
    src/wrappers/win32/BHAPI_wrapper_os.cpp \
    src/wrappers/win32/BHAPI_wrapper_timefuncs.cpp

HEADERS += src/wrappers/win32/GDI/BHAPI_wrapper_win32gdi.h

LIBS += -lkernel32 -lgdi32 -lshell32 -luser32 -lcomctl32 -luserenv
LIBS += -lws2_32 -lwsock32 -lwinmm -limm32 -lole32 -loleaut32
}

contains(QMAKE_COMPILER_DEFINES, LINUX) {
SOURCES += src/wrappers/posix/X11/BHAPI_wrapper_application.cpp \
    src/wrappers/posix/X11/BHAPI_wrapper_drawing.cpp \
    src/wrappers/posix/X11/BHAPI_wrapper_pixmap.cpp \
    src/wrappers/posix/X11/BHAPI_wrapper_window.cpp \
    src/wrappers/posix/X11/BHAPI_wrapper_x11-font.cpp \
    src/wrappers/posix/BHAPI_wrapper_locker.cpp \
    src/wrappers/posix/BHAPI_wrapper_semaphore-mach.cpp \
    src/wrappers/posix/BHAPI_wrapper_semaphore-umtx.cpp \
    src/wrappers/posix/BHAPI_wrapper_semaphore.cpp \
    src/wrappers/posix/BHAPI_wrapper_thread.cpp \
    src/wrappers/posix/BHAPI_wrapper_area.cpp \
    src/wrappers/posix/BHAPI_wrapper_image.cpp \
    src/wrappers/posix/BHAPI_wrapper_os.cpp \
    src/wrappers/posix/BHAPI_wrapper_timefuncs.cpp

HEADERS +=     src/wrappers/posix/X11/BHAPI_wrapper_x11.h
}

contains(QMAKE_COMPILER_DEFINES, DIRECTFB) {
SOURCES +=    src/wrappers/posix/DIRECTFB/BHAPI_wrapper_application.cpp \
    src/wrappers/posix/DIRECTFB/BHAPI_wrapper_dfb-font.cpp \
    src/wrappers/posix/DIRECTFB/BHAPI_wrapper_drawing.cpp \
    src/wrappers/posix/DIRECTFB/BHAPI_wrapper_pixmap.cpp \
    src/wrappers/posix/DIRECTFB/BHAPI_wrapper_window.cpp \
    src/wrappers/posix/DIRECTFB/BHAPI_wrapper_wm.cpp

HEADERS += src/wrappers/posix/DIRECTFB/BHAPI_wrapper_dfb.h
}

contains(QMAKE_COMPILER_DEFINES, MACOS) {
HEADERS += src/kits/interface/carbon/BHAPI_wrapper_carbon.h
}

contains(QMAKE_COMPILER_DEFINES, __GNUC__) {
QMAKE_CXXFLAGS -= -pipe

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

QMAKE_CXXFLAGS += -save-temps -Winvalid-pch
QMAKE_CXXFLAGS += -fstrict-aliasing
QMAKE_CXXFLAGS += -std=gnu++11 -pthread
QMAKE_CXXFLAGS += -malign-double
QMAKE_CXXFLAGS += -momit-leaf-frame-pointer
QMAKE_CXXFLAGS += -fwrapv
QMAKE_CXXFLAGS += -m32 -mfpmath=sse -flto
QMAKE_CXXFLAGS += -mpreferred-stack-boundary=8
QMAKE_CXXFLAGS += -mmmx -msse -msse2

    contains(CONFIG, debug) {
    QMAKE_CXXFLAGS += -fverbose-asm
    QMAKE_CXXFLAGS += -g -dD
    }
    contains(CONFIG, realese) {
    QMAKE_CXXFLAGS += -O3
    QMAKE_CXXFLAGS += -funroll-loops
    }
}

contains(QMAKE_COMPILER_DEFINES, __clang__) {
QMAKE_CXXFLAGS -= -fno-keep-inline-dllexport
QMAKE_CXXFLAGS -= -finline-small-functions
QMAKE_CXXFLAGS -= -pipe

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

QMAKE_CXXFLAGS += -fstrict-aliasing
QMAKE_CXXFLAGS += -std=gnu++0x -pthread
QMAKE_CXXFLAGS += -malign-double
QMAKE_CXXFLAGS += -momit-leaf-frame-pointer
QMAKE_CXXFLAGS += -fwrapv
QMAKE_CXXFLAGS += -m32 --32 -mfpmath=sse -flto
QMAKE_CXXFLAGS += -mpreferred-stack-boundary=8
QMAKE_CXXFLAGS += -mmmx -msse -msse2
QMAKE_CXXFLAGS += -Qunused-arguments
QMAKE_CXXFLAGS += -Wno-error=unused-command-line-argument-hard-error-in-future

QMAKE_LFLAGS -= -mthreads

QMAKE_LFLAGS += -Qunused-arguments
QMAKE_LFLAGS += -Wno-error=unused-command-line-argument-hard-error-in-future

    contains(CONFIG, debug) {
    QMAKE_CXXFLAGS += -save-temps
    QMAKE_CXXFLAGS += -fverbose-asm
    QMAKE_CXXFLAGS += -g -dD
    }
    contains(CONFIG, realese) {
    QMAKE_CXXFLAGS += -O3
    QMAKE_CXXFLAGS += -funroll-loops
    }

}


unix {
    target.path = /usr/lib
    INSTALLS += target
}
