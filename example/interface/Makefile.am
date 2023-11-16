INCLUDES = $(ETKXX_CFLAGS)
LIBS = $(ETKXX_LIBS)

if NATIVE_WIN32
if WITH_GCC
AM_CPPFLAGS = -mwindows
AM_LDFLAGS = -mwindows
endif # WITH_GCC
endif # NATIVE_WIN32

noinst_PROGRAMS = helloworld1 helloworld2 helloworld3

helloworld1_SOURCES = helloworld1.cpp
helloworld2_SOURCES = helloworld2.cpp
helloworld3_SOURCES = helloworld3.cpp

DISTCLEANFILES = Makefile.in

