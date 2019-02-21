ETK++ The EASY Toolkit for C++ programing
Copyright (C) 2004-2007 Anthony Lee, All rights reserved.
Copyright (C) 2017-2019 Stanisław Stasiak, with the name BHAPI - BeOS/Haiku API. 
It's attempt to make Haiku OS API to be available on other systems as toolkit. 
To make BeOS specific software available on more popular systems such as Windows/MacOS/Linux. 
Furthermore make Haiku OS more attractive and interoperative as development platform.

 * Most ideas of it just from the great BeOS API.
 * ETK++ library is a freeware; it may be used and distributed according to
   the terms of The MIT License.

How to compile ETK++ from source
--------------------------------
 * First run "./autogen.sh --config" in current directory
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

