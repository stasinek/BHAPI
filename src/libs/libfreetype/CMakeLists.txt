project( freetype )

if( WIN32 )
    if( MINGW )
      add_definitions( -pedantic -ansi -Wall -g -O2 -DPIC )
    else( MINGW )
      add_definitions( -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN -D_REENTRANT )
    endif( MINGW )
endif( WIN32 )

include( CheckIncludeFile )
include( CheckTypeSize )

CHECK_INCLUDE_FILE( "unistd.h" HAVE_UNISTD_H )
CHECK_INCLUDE_FILE( "fcntl.h" HAVE_FCNTL_H )
CHECK_TYPE_SIZE( int SIZEOF_INT )
CHECK_TYPE_SIZE( long SIZEOF_LONG )

configure_file( ftconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/ftconfig.h )
configure_file( ftmodule.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/ftmodule.h )

set( freetype_SRC
    src/base/ftsystem.c
    src/base/ftdebug.c
    src/base/ftinit.c
    src/base/ftbase.c
    src/base/ftbbox.c
    src/base/ftbdf.c
    src/base/ftbitmap.c
    src/base/ftglyph.c
    src/base/ftgxval.c
    src/base/ftmm.c
    src/base/ftotval.c
    src/base/ftpfr.c
    src/base/ftstroke.c
    src/base/ftsynth.c
    src/base/fttype1.c
    src/base/ftwinfnt.c
    src/base/ftxf86.c
    src/base/ftlcdfil.c
    src/base/ftgasp.c
    src/base/ftpatent.c
    src/truetype/truetype.c
    src/type1/type1.c
    src/cff/cff.c
    src/cid/type1cid.c
    src/pfr/pfr.c
    src/type42/type42.c
    src/winfonts/winfnt.c
    src/pcf/pcf.c
    src/bdf/bdf.c
    src/sfnt/sfnt.c
    src/autofit/autofit.c
    src/pshinter/pshinter.c
    src/raster/raster.c
    src/smooth/smooth.c
    src/cache/ftcache.c
    src/gzip/ftgzip.c
    src/lzw/ftlzw.c
    src/psaux/psaux.c
    src/psnames/psmodule.c
)

include_directories( 
    builds/unix
    src/base 
    src/truetype 
    src/type1 
    src/cff 
    src/cid 
    src/pfr 
    src/type42 
    src/winfonts 
    src/pcf 
    src/bdf 
    src/sfnt 
    src/autofit 
    src/pshinter 
    src/raster 
    src/smooth 
    src/cache 
    src/gzip 
    src/lzw 
    src/psaux 
    src/psnames
    include
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_definitions( -DFT_CONFIG_CONFIG_H="<ftconfig.h>" -DFT2_BUILD_LIBRARY -DFT_CONFIG_MODULES_H="<ftmodule.h>" )

add_library( freetype SHARED ${freetype_SRC} )

install( TARGETS freetype RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib )

file( GLOB include_files "include/freetype/*.h")
file( GLOB include_config_files "include/freetype/config/*.h")
file( GLOB include_internal_files "include/freetype/internal/*.h")

install( FILES ${include_files} DESTINATION include/freetype2/freetype )
install( FILES ${include_config_files} DESTINATION include/freetype2/freetype/config )
install( FILES ${include_internal_files} DESTINATION include/freetype2/freetype/internal )
install( FILES include/ft2build.h DESTINATION include/freetype2 )
