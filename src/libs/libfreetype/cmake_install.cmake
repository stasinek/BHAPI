# Install script for directory: C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/freetype")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/Debug/freetype.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/Release/freetype.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/MinSizeRel/freetype.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/RelWithDebInfo/freetype.lib")
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/Debug/freetype.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/Release/freetype.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/MinSizeRel/freetype.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/RelWithDebInfo/freetype.dll")
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/freetype2/freetype" TYPE FILE FILES
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/freetype.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftbbox.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftbdf.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftbitmap.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftcache.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftchapters.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftcid.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/fterrdef.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/fterrors.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftgasp.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftglyph.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftgxval.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftgzip.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftimage.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftincrem.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftlcdfil.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftlist.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftlzw.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftmac.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftmm.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftmodapi.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftmoderr.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftotval.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftoutln.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftpfr.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftrender.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftsizes.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftsnames.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftstroke.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftsynth.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftsystem.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/fttrigon.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/fttypes.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftwinfnt.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ftxf86.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/t1tables.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ttnameid.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/tttables.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/tttags.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/ttunpat.h"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/freetype2/freetype/config" TYPE FILE FILES
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/config/ftconfig.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/config/ftheader.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/config/ftmodule.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/config/ftoption.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/config/ftstdlib.h"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/freetype2/freetype/internal" TYPE FILE FILES
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/internal/autohint.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/internal/ftcalc.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/internal/ftdebug.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/internal/ftdriver.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/internal/ftgloadr.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/internal/ftmemory.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/internal/ftobjs.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/internal/ftrfork.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/internal/ftserv.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/internal/ftstream.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/internal/fttrace.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/internal/ftvalid.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/internal/internal.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/internal/pcftypes.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/internal/psaux.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/internal/pshints.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/internal/sfnt.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/internal/t1types.h"
    "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/freetype/internal/tttypes.h"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/freetype2" TYPE FILE FILES "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/include/ft2build.h")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/Documents and Settings/stasiek.TC12/My Documents/prc++/x86_libraries/freetype/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
