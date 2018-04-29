# Microsoft Developer Studio Project File - Name="vfs" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=vfs - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vfs.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vfs.mak" CFG="vfs - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vfs - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "vfs - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vfs - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "vfs - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "vfs - Win32 Release"
# Name "vfs - Win32 Debug"
# Begin Group "windows"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\windows\aloc.c
# End Source File
# Begin Source File

SOURCE=.\windows\async.c
# End Source File
# Begin Source File

SOURCE=.\windows\char.c
# End Source File
# Begin Source File

SOURCE=.\windows\cleanup.c
# End Source File
# Begin Source File

SOURCE=.\windows\close.c
# End Source File
# Begin Source File

SOURCE=.\windows\cmcb.c
# End Source File
# Begin Source File

SOURCE=.\windows\create.c
# End Source File
# Begin Source File

SOURCE=.\windows\debug.c
# End Source File
# Begin Source File

SOURCE=.\windows\devctrl.c
# End Source File
# Begin Source File

SOURCE=.\windows\dirctrl.c
# End Source File
# Begin Source File

SOURCE=.\windows\diskio.c
# End Source File
# Begin Source File

SOURCE=.\windows\fastio.c
# End Source File
# Begin Source File

SOURCE=.\windows\fileinfo.c
# End Source File
# Begin Source File

SOURCE=.\windows\fsctrl.c
# End Source File
# Begin Source File

SOURCE=.\windows\init.c
# End Source File
# Begin Source File

SOURCE=.\windows\misc.c
# End Source File
# Begin Source File

SOURCE=.\windows\read.c
# End Source File
# End Group
# Begin Group "include"

# PROP Default_Filter ""
# Begin Group "linux"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\include\linux\blkdev.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\dcache.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\errno.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\fcntl.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\fs.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\fs_struct.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\kdev_t.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\kernel.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\list.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\locks.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\mm.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\mm2.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\module.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\mount.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\pagemap.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\posix_types.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\quota.h
# End Source File
# Begin Source File

SOURCE=".\include\linux\rwsem-spinlock.h"
# End Source File
# Begin Source File

SOURCE=.\include\linux\rwsem.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\sched.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\semaphore.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\slab.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\spinlock.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\stat.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\string.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\types.h
# End Source File
# Begin Source File

SOURCE=.\include\linux\wait.h
# End Source File
# End Group
# Begin Group "asm"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\include\asm\atomic.h
# End Source File
# Begin Source File

SOURCE=.\include\asm\bitops.h
# End Source File
# Begin Source File

SOURCE=.\include\asm\current.h
# End Source File
# Begin Source File

SOURCE=.\include\asm\errno.h
# End Source File
# Begin Source File

SOURCE=.\include\asm\fcntl.h
# End Source File
# Begin Source File

SOURCE=.\include\asm\highmem.h
# End Source File
# Begin Source File

SOURCE=.\include\asm\page.h
# End Source File
# Begin Source File

SOURCE=.\include\asm\posix_types.h
# End Source File
# Begin Source File

SOURCE=.\include\asm\semaphore.h
# End Source File
# Begin Source File

SOURCE=.\include\asm\statfs.h
# End Source File
# Begin Source File

SOURCE=.\include\asm\types.h
# End Source File
# Begin Source File

SOURCE=.\include\asm\uaccess.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\include\debug.h
# End Source File
# Begin Source File

SOURCE=.\include\pitix.h
# End Source File
# Begin Source File

SOURCE=.\include\pitix_sadyc.h
# End Source File
# Begin Source File

SOURCE=.\include\todo.h
# End Source File
# Begin Source File

SOURCE=.\include\usermode.h
# End Source File
# Begin Source File

SOURCE=.\include\vfs.h
# End Source File
# End Group
# Begin Group "linux No. 1"

# PROP Default_Filter ""
# Begin Group "fs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\linux\fs\attr.c
# End Source File
# Begin Source File

SOURCE=.\linux\fs\bad_inode.c
# End Source File
# Begin Source File

SOURCE=.\linux\fs\block_dev.c
# End Source File
# Begin Source File

SOURCE=.\linux\fs\buffer.c
# End Source File
# Begin Source File

SOURCE=.\linux\fs\dcache.c
# End Source File
# Begin Source File

SOURCE=.\linux\fs\devices.c
# End Source File
# Begin Source File

SOURCE=.\linux\fs\file_table.c
# End Source File
# Begin Source File

SOURCE=.\linux\fs\inode.c
# End Source File
# Begin Source File

SOURCE=.\linux\fs\namei.c
# End Source File
# Begin Source File

SOURCE=.\linux\fs\namespace.c
# End Source File
# Begin Source File

SOURCE=.\linux\fs\open.c
# End Source File
# Begin Source File

SOURCE=.\linux\fs\read_write.c
# End Source File
# Begin Source File

SOURCE=.\linux\fs\readdir.c
# End Source File
# Begin Source File

SOURCE=.\linux\fs\super.c
# End Source File
# End Group
# Begin Group "mm"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\linux\mm\filemap.c
# End Source File
# Begin Source File

SOURCE=.\linux\mm\kmem_cache.c
# End Source File
# Begin Source File

SOURCE=.\linux\mm\page_alloc.c
# End Source File
# End Group
# Begin Group "lib"

# PROP Default_Filter ""
# End Group
# End Group
# Begin Group "pitix"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pitix\pitix_block.c
# End Source File
# Begin Source File

SOURCE=.\pitix\pitix_dir.c
# End Source File
# Begin Source File

SOURCE=.\pitix\pitix_file.c
# End Source File
# Begin Source File

SOURCE=.\pitix\pitix_inode.c
# End Source File
# Begin Source File

SOURCE=.\pitix\pitix_main.c
# End Source File
# End Group
# Begin Group "vfat"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\vfat\buffer.c
# End Source File
# Begin Source File

SOURCE=.\vfat\cache.c
# End Source File
# Begin Source File

SOURCE=.\vfat\cvf.c
# End Source File
# Begin Source File

SOURCE=.\vfat\dir.c
# End Source File
# Begin Source File

SOURCE=.\vfat\fatfs_syms.c
# End Source File
# Begin Source File

SOURCE=.\vfat\file.c
# End Source File
# Begin Source File

SOURCE=.\vfat\inode.c
# End Source File
# Begin Source File

SOURCE=.\vfat\misc.c
# End Source File
# Begin Source File

SOURCE=.\vfat\namei.c
# End Source File
# Begin Source File

SOURCE=.\vfat\nls_base.c
# End Source File
# Begin Source File

SOURCE=.\vfat\vfatfs_syms.c
# End Source File
# End Group
# Begin Group "minix"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\minix\bitmap.c
# End Source File
# Begin Source File

SOURCE=.\minix\dir.c
# End Source File
# Begin Source File

SOURCE=.\minix\file.c
# End Source File
# Begin Source File

SOURCE=.\minix\inode.c
# End Source File
# Begin Source File

SOURCE=.\minix\itree_common.c
# End Source File
# Begin Source File

SOURCE=.\minix\itree_v1.c
# End Source File
# Begin Source File

SOURCE=.\minix\itree_v2.c
# End Source File
# Begin Source File

SOURCE=.\minix\namei.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\windows\Makefile
# End Source File
# Begin Source File

SOURCE=.\windows\Sources
# End Source File
# End Target
# End Project
