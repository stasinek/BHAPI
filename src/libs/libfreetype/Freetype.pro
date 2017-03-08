#-------------------------------------------------
#
# Project created by QtCreator 2017-02-05T15:33:45
#
#-------------------------------------------------

TARGET = freetype
TEMPLATE = lib
DEFINES += FT2_BUILD_LIBRARY
CONFIG += windows shared dll lib_bundle staticlib
CONFIG -= app_bundle
CONFIG -= qt
QT -= core gui

LIBS += -lkernel32 -lgdi32 -lshell32 -luser32 -lcomctl32 -luserenv
LIBS += -lws2_32 -lwsock32 -lwinmm -limm32 -lole32 -loleaut32

SOURCES += Freetype.cpp \
    src/base/ftsystem.c \
    src/base/ftdebug.c \
    src/base/ftinit.c \
    src/base/ftbase.c \
    src/base/ftbbox.c \
    src/base/ftbdf.c \
    src/base/ftbitmap.c \
    src/base/ftglyph.c \
    src/base/ftgxval.c \
    src/base/ftmm.c \
    src/base/ftotval.c \
    src/base/ftpfr.c \
    src/base/ftstroke.c \
    src/base/ftsynth.c \
    src/base/fttype1.c \
    src/base/ftwinfnt.c \
    src/base/ftxf86.c \
    src/base/ftlcdfil.c \
    src/base/ftgasp.c \
    src/base/ftpatent.c \
    src/truetype/truetype.c \
    src/type1/type1.c \
    src/cff/cff.c \
    src/cid/type1cid.c \
    src/pfr/pfr.c \
    src/type42/type42.c \
    src/winfonts/winfnt.c \
    src/pcf/pcf.c \
    src/bdf/bdf.c \
    src/sfnt/sfnt.c \
    src/autofit/autofit.c \
    src/pshinter/pshinter.c \
    src/raster/raster.c \
    src/smooth/smooth.c \
    src/cache/ftcache.c \
    src/gzip/ftgzip.c \
    src/lzw/ftlzw.c \
    src/psaux/psaux.c \
    src/psnames/psmodule.c
 

HEADERS += Freetype.h \
    include/ft2build.h \
    src/autofit/afcjk.h \
    src/autofit/afcover.h \
    src/autofit/afdummy.h \
    src/autofit/aferrors.h \
    src/autofit/afglobal.h \
    src/autofit/afhints.h \
    src/autofit/afindic.h \
    src/autofit/aflatin.h \
    src/autofit/aflatin2.h \
    src/autofit/afloader.h \
    src/autofit/afmodule.h \
    src/autofit/afangles.h \
    src/autofit/afscript.h \
    src/autofit/afstyles.h \
    src/autofit/aftypes.h \
    src/autofit/afwarp.h \
    src/autofit/afwrtsys.h \
    src/base/basepic.h \
    src/base/ftbase.h \
    src/base/md5.h \
    src/bdf/bdf.h \
    src/bdf/bdfdrivr.h \
    src/bdf/bdferror.h \
    src/cache/ftccache.h \
    src/cache/ftccback.h \
    src/cache/ftcerror.h \
    src/cache/ftcglyph.h \
    src/cache/ftcimage.h \
    src/cache/ftcmanag.h \
    src/cache/ftcmru.h \
    src/cache/ftcsbits.h \
    src/cff/cf2arrst.h \
    src/cff/cf2blues.h \
    src/cff/cf2error.h \
    src/cff/cf2fixed.h \
    src/cff/cf2font.h \
    src/cff/cf2ft.h \
    src/cff/cf2glue.h \
    src/cff/cf2hints.h \
    src/cff/cf2intrp.h \
    src/cff/cf2read.h \
    src/cff/cf2stack.h \
    src/cff/cf2types.h \
    src/cff/cffcmap.h \
    src/cff/cffdrivr.h \
    src/cff/cfferrs.h \
    src/cff/cffgload.h \
    src/cff/cffload.h \
    src/cff/cffobjs.h \
    src/cff/cffparse.h \
    src/cff/cffpic.h \
    src/cff/cfftoken.h \
    src/cff/cfftypes.h \
    src/cid/ciderrs.h \
    src/cid/cidgload.h \
    src/cid/cidload.h \
    src/cid/cidobjs.h \
    src/cid/cidparse.h \
    src/cid/cidriver.h \
    src/cid/cidtoken.h \
    src/gxvalid/gxvalid.h \
    src/gxvalid/gxvcommn.h \
    src/gxvalid/gxverror.h \
    src/gxvalid/gxvfeat.h \
    src/gxvalid/gxvmod.h \
    src/gxvalid/gxvmort.h \
    src/gxvalid/gxvmorx.h \
    src/gzip/ftzconf.h \
    src/gzip/infblock.h \
    src/gzip/infcodes.h \
    src/gzip/inffixed.h \
    src/gzip/inftrees.h \
    src/gzip/infutil.h \
    src/gzip/zlib.h \
    src/gzip/zutil.h \
    src/lzw/ftzopen.h \
    src/otvalid/otvalid.h \
    src/otvalid/otvcommn.h \
    src/otvalid/otverror.h \
    src/otvalid/otvgpos.h \
    src/otvalid/otvmod.h \
    src/pcf/pcf.h \
    src/pcf/pcfdrivr.h \
    src/pcf/pcferror.h \
    src/pcf/pcfread.h \
    src/pcf/pcfutil.h \
    src/pfr/pfrcmap.h \
    src/pfr/pfrdrivr.h \
    src/pfr/pfrerror.h \
    src/pfr/pfrgload.h \
    src/pfr/pfrload.h \
    src/pfr/pfrobjs.h \
    src/pfr/pfrsbit.h \
    src/pfr/pfrtypes.h \
    src/psaux/afmparse.h \
    src/psaux/psauxerr.h \
    src/psaux/psauxmod.h \
    src/psaux/psconv.h \
    src/psaux/psobjs.h \
    src/psaux/t1cmap.h \
    src/psaux/t1decode.h \
    src/pshinter/pshalgo.h \
    src/pshinter/pshglob.h \
    src/pshinter/pshmod.h \
    src/pshinter/pshnterr.h \
    src/pshinter/pshpic.h \
    src/pshinter/pshrec.h \
    src/psnames/psmodule.h \
    src/psnames/psnamerr.h \
    src/psnames/pspic.h \
    src/psnames/pstables.h \
    src/raster/ftmisc.h \
    src/raster/ftraster.h \
    src/raster/ftrend1.h \
    src/raster/rasterrs.h \
    src/raster/rastpic.h \
    src/sfnt/pngshim.h \
    src/sfnt/sfdriver.h \
    src/sfnt/sferrors.h \
    src/sfnt/sfntpic.h \
    src/sfnt/sfobjs.h \
    src/sfnt/ttbdf.h \
    src/sfnt/ttcmap.h \
    src/sfnt/ttcmapc.h \
    src/sfnt/ttkern.h \
    src/sfnt/ttload.h \
    src/sfnt/ttmtx.h \
    src/sfnt/ttpost.h \
    src/sfnt/ttsbit.h \
    src/smooth/ftgrays.h \
    src/smooth/ftsmerrs.h \
    src/smooth/ftsmooth.h \
    src/smooth/ftspic.h \
    src/truetype/ttdriver.h \
    src/truetype/tterrors.h \
    src/truetype/ttgload.h \
    src/truetype/ttgxvar.h \
    src/truetype/ttinterp.h \
    src/truetype/ttobjs.h \
    src/truetype/ttpic.h \
    src/truetype/ttpload.h \
    src/truetype/ttsubpix.h \
    src/type1/t1afm.h \
    src/type1/t1driver.h \
    src/type1/t1errors.h \
    src/type1/t1gload.h \
    src/type1/t1load.h \
    src/type1/t1objs.h \
    src/type1/t1parse.h \
    src/type1/t1tokens.h \
    src/type42/t42drivr.h \
    src/type42/t42error.h \
    src/type42/t42objs.h \
    src/type42/t42parse.h \
    src/type42/t42types.h \
    src/winfonts/fnterrs.h \
    src/winfonts/winfnt.h \
    include/freetype/config/ftconfig.h \
    include/freetype/config/ftheader.h \
    include/freetype/config/ftmodule.h \
    include/freetype/config/ftoption.h \
    include/freetype/config/ftstdlib.h \
    include/freetype/internal/services/svbdf.h \
    include/freetype/internal/services/svcid.h \
    include/freetype/internal/services/svfntfmt.h \
    include/freetype/internal/services/svgldict.h \
    include/freetype/internal/services/svgxval.h \
    include/freetype/internal/services/svkern.h \
    include/freetype/internal/services/svmetric.h \
    include/freetype/internal/services/svmm.h \
    include/freetype/internal/services/svotval.h \
    include/freetype/internal/services/svpfr.h \
    include/freetype/internal/services/svpostnm.h \
    include/freetype/internal/services/svprop.h \
    include/freetype/internal/services/svpscmap.h \
    include/freetype/internal/services/svpsinfo.h \
    include/freetype/internal/services/svsfnt.h \
    include/freetype/internal/services/svttcmap.h \
    include/freetype/internal/services/svtteng.h \
    include/freetype/internal/services/svttglyf.h \
    include/freetype/internal/services/svwinfnt.h \
    include/freetype/internal/autohint.h \
    include/freetype/internal/ftcalc.h \
    include/freetype/internal/ftdebug.h \
    include/freetype/internal/ftdriver.h \
    include/freetype/internal/ftgloadr.h \
    include/freetype/internal/fthash.h \
    include/freetype/internal/ftmemory.h \
    include/freetype/internal/ftobjs.h \
    include/freetype/internal/ftpic.h \
    include/freetype/internal/ftrfork.h \
    include/freetype/internal/ftserv.h \
    include/freetype/internal/ftstream.h \
    include/freetype/internal/fttrace.h \
    include/freetype/internal/ftvalid.h \
    include/freetype/internal/internal.h \
    include/freetype/internal/psaux.h \
    include/freetype/internal/pshints.h \
    include/freetype/internal/sfnt.h \
    include/freetype/internal/t1types.h \
    include/freetype/internal/tttypes.h \
    include/freetype/freetype.h \
    include/freetype/ftadvanc.h \
    include/freetype/ftautoh.h \
    include/freetype/ftbbox.h \
    include/freetype/ftbdf.h \
    include/freetype/ftbitmap.h \
    include/freetype/ftbzip2.h \
    include/freetype/ftcache.h \
    include/freetype/ftcffdrv.h \
    include/freetype/ftchapters.h \
    include/freetype/ftcid.h \
    include/freetype/fterrdef.h \
    include/freetype/fterrors.h \
    include/freetype/ftfntfmt.h \
    include/freetype/ftgasp.h \
    include/freetype/ftglyph.h \
    include/freetype/ftgxval.h \
    include/freetype/ftgzip.h \
    include/freetype/ftimage.h \
    include/freetype/ftincrem.h \
    include/freetype/ftlcdfil.h \
    include/freetype/ftlist.h \
    include/freetype/ftlzw.h \
    include/freetype/ftmac.h \
    include/freetype/ftmm.h \
    include/freetype/ftmodapi.h \
    include/freetype/ftmoderr.h \
    include/freetype/ftotval.h \
    include/freetype/ftoutln.h \
    include/freetype/ftpfr.h \
    include/freetype/ftrender.h \
    include/freetype/ftsizes.h \
    include/freetype/ftsnames.h \
    include/freetype/ftstroke.h \
    include/freetype/ftsynth.h \
    include/freetype/ftsystem.h \
    include/freetype/fttrigon.h \
    include/freetype/ftttdrv.h \
    include/freetype/fttypes.h \
    include/freetype/ftwinfnt.h \
    include/freetype/ftxf86.h \
    include/freetype/t1tables.h \
    include/freetype/ttnameid.h \
    include/freetype/tttables.h \
    include/freetype/tttags.h \
    include/freetype/ttunpat.h

contains(QMAKE_COMPILER_DEFINES, BEOS) {
SOURCES += 
HEADERS += 
}

contains(QMAKE_COMPILER_DEFINES, WIN32) {
SOURCES += 
HEADERS += 
}

contains(QMAKE_COMPILER_DEFINES, LINUX) {
SOURCES += 
HEADERS += 
}

contains(QMAKE_COMPILER_DEFINES, MACOS) {
SOURCES += 
HEADERS += 
}

contains(QMAKE_COMPILER_DEFINES, __GNBUC__) {
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
QMAKE_CXXFLAGS += -std=gnu++11 -pthread
QMAKE_CXXFLAGS += -fverbose-asm
QMAKE_CXXFLAGS += -fstrict-aliasing
QMAKE_CXXFLAGS += -dD -DPIC
QMAKE_CXXFLAGS += -g
QMAKE_CXXFLAGS += -malign-double
QMAKE_CXXFLAGS += -momit-leaf-frame-pointer
QMAKE_CXXFLAGS += -fwrapv
QMAKE_CXXFLAGS += -funroll-loops
QMAKE_CXXFLAGS += -m32 -mfpmath=sse -flto #-O1
QMAKE_CXXFLAGS += -mpreferred-stack-boundary=8
QMAKE_CXXFLAGS += -mmmx -msse -msse2 #-msse3
QMAKE_CXXFLAGS += #-fno-leading-underscore
QMAKE_CFLAGS   += #-fno-leading-underscore
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
QMAKE_CXXFLAGS += -Qunused-arguments -Wno-error=unused-command-line-argument-hard-error-in-future

QMAKE_CXXFLAGS += -std=gnu++11 -pthread
QMAKE_CXXFLAGS += -save-temps
QMAKE_CXXFLAGS += -fverbose-asm
QMAKE_CXXFLAGS += -fstrict-aliasing
QMAKE_CXXFLAGS += -dD
QMAKE_CXXFLAGS += -g
QMAKE_CXXFLAGS += -malign-double
QMAKE_CXXFLAGS += -momit-leaf-frame-pointer
QMAKE_CXXFLAGS += -fwrapv
QMAKE_CXXFLAGS += -funroll-loops
QMAKE_CXXFLAGS += -m32 --32 -mfpmath=sse -flto #-O1
QMAKE_CXXFLAGS += -mpreferred-stack-boundary=8
QMAKE_CXXFLAGS += -mmmx -msse -msse2 #-msse3
QMAKE_CXXFLAGS -= -fno-keep-inline-dllexport
QMAKE_CXXFLAGS -= -finline-small-functions
QMAKE_CXXFLAGS -= -pipe

QMAKE_LFLAGS += -Qunused-arguments -Wno-error=unused-command-line-argument-hard-error-in-future
QMAKE_LFLAGS   -= -mthreads
}
unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += src \
    builds/unix \
    src/base \
    src/truetype \
    src/type1 \
    src/cff \
    src/cid \
    src/pfr \
    src/type42 \
    src/winfonts \
    src/pcf \
    src/bdf \
    src/sfnt \
    src/autofit \
    src/pshinter \
    src/raster \
    src/smooth \
    src/cache \
    src/gzip \
    src/lzw \
    src/psaux \
    src/psnames \
    include \
