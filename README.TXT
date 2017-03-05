BHAPI, Copyright(C) 2017 Stanislaw Stasiak was previously named ETK++ and Copyright (C) 2004-2007 by Anthony Lee

 * Most idea of it just from the great BeOS API + new Haiku additions to other OS. 
 That's why i changed name to: BeOS + Haiku API = Don't Worry Be Happy ;)
 * Next step is to port BeOS/Haiku apps and BeFS filesystem driver to Windows, Linux, MacOS
 * Library may be used and distributed according to
   the terms of The MIT License.
 * I hope that this library will bring more apps to Haiku and make existing ones much better. 

How to compile from source on Windows? Most easy way is to use Qt Creator .pro files, or Dev++ .dev or Code:Blocks.cbs
You need to have freetype library same path than BHAPI directory and that's it.
Full freetype sources or freetype/include and freetype/lib/debug/libfreetype.a to link statically into BHAPI 
--------------------------------
 * Old option wont work with BHAPI.
 For ETK+ it was:
 First run "./autogen.sh --config" in current directory
	autogen.sh requires:
		autoconf >= 2.54
		automake >= 1.7
		libtool >= 1.5.24
		pkg-config >= 0.14

 * Linux/MinGW/CygWin/FreeBSD/Solaris/MacOSX/BeOS:
	1. run the "./configure" to generate the config files for compiling.
	2. run "make" to compile all.
	3. run "make install" to install all.
	4. compile example:
		$ g++ `etkxx-config --cflags --libs` myapp.cpp -o myapp
	5. special:
	  IPC on Linux --- 2.6.x kernel
	  IPC on FreeBSD 6.x --- Add "options P1003_1B_SEMAPHORES" to kernel config and recompile kernel

 * Windows MSVC++ 6.0:
	1. run "./configure --with-built-in-graphics=win32" within MinGW to generate the files
	2. open "build/win32/etkxx.dsw" in MSVC++ 6.0
	3. compile the project named "etkxx"
	4. the "build/win32/example.dsw" just for compiling examples

 * Windows MSVC .NET/2005:
	1. run "./configure --with-built-in-graphics=win32" within MinGW to generate the files
	2. open "build/win32/etkxx.sln" in MSVC .NET/2005
	3. compile the project named "etkxx"
	4. the "build/win32/example.sln" just for compiling examples

I try to make this library fully compilable using 3 or even more compilers: MinGW, Clang, Visual Studio 2003
Original one based on ETK+ was sucesfully compilable, since 2017.2 i try to integrate with new Haiku sourcecode, so it wont work will be in alpha stage for a while.
