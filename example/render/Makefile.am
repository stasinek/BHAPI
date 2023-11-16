INCLUDES = $(ETKXX_CFLAGS)
LIBS = $(ETKXX_LIBS)
EFENCE = 

if NATIVE_WIN32
if WITH_GCC
AM_CPPFLAGS = -mwindows
AM_LDFLAGS = -mwindows
endif # WITH_GCC
endif # NATIVE_WIN32

noinst_PROGRAMS =

if HAVE_SDL
noinst_PROGRAMS += $(EFENCE) Render-SDL
Render_SDL_SOURCES = Render-SDL.cpp
Render_SDL_CPPFLAGS = $(SDL_CFLAGS)
Render_SDL_LDADD = $(SDL_LIBS)
endif

DISTCLEANFILES = Makefile.in

