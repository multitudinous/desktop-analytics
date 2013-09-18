#*************** <auto-copyright.pl BEGIN do not edit this line> **************
#
# jag3d is (C) Copyright 2011-2012 by Kenneth Mark Bryden and Paul Martz
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License version 2.1 as published by the Free Software Foundation.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#
#************** <auto-copyright.pl END do not edit this line> ***************
cmake_minimum_required( VERSION 2.8.5 )
project( FEATUREDEMODA )

macro( zeroPad _input _output )
    if( ${${_input}} LESS 10 )
        set( ${_output} "0${${_input}}" )
    else()
        set( ${_output} "${${_input}}" )
    endif()
endmacro()

set( DATEST_MAJOR_VERSION 0 )
set( DATEST_MINOR_VERSION 0 )
set( DATEST_SUB_VERSION 1 )
zeroPad( DATEST_MINOR_VERSION __minorPad )
zeroPad( DATEST_SUB_VERSION __patchPad )
set( DATEST_VERSION ${DATEST_MAJOR_VERSION}.${__minorPad}.${__patchPad} )


set( CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}" )

set_property( GLOBAL PROPERTY USE_FOLDERS ON )


set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib )


if( WIN32 AND MSVC )
    # Disable warnings
    # C4251: <membervar> : class <classX> needs to have dll-interface to be used by clients of class <classY>
    # C4275: non dll-interface class <classX> used as base for dll-interface class <classY>
    foreach( warning 4251 4275 )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd${warning}" )
    endforeach()

    # Enable MP, unless specified otherwise
    #option( JAG3D_WIN32_USE_MP "Build with multiple processes (/MP option; VS 2005 and above)." ON )
    #mark_as_advanced( JAG3D_WIN32_USE_MP )
    #if( JAG3D_WIN32_USE_MP )
    #    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP" )
    #endif()
endif()



#
# Dependencies

set( _boostComponents )
list( APPEND _boostComponents thread )
list( APPEND _boostComponents program_options )
list( APPEND _boostComponents date_time )
list( APPEND _boostComponents chrono )
list( APPEND _boostComponents system )
list( APPEND _boostComponents filesystem )
list( APPEND _boostComponents regex )
#
#   At least one windowing API is required for demos/tests/examples:
#option( JAG3D_USE_FREEGLUT "If set and freeglut is found, Jag3D builds windowed executables with freeglut." ON )
#if( JAG3D_USE_FREEGLUT )
#    include( FindFreeglutHelper )
#endif()
#option( DATEST_USE_QT "If set and Qt is found, DA test suite builds windowed executables with Qt." ON )
#if( DATEST_USE_QT )
find_package( Qt4 4.8.0 COMPONENTS QtCore QtGui QtOpenGL )
#endif()

#   Required dependencies:
find_package( OpenGL REQUIRED )

if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)
message(ERROR ${OPENGL_INCLUDE_DIRS})

find_package( GL3 REQUIRED )
find_package( Boost 1.44.0 REQUIRED ${_boostComponents} )
include( FindPOCOHelper )
include( FindGGTHelper )
if( NOT GMTL_FOUND )
    find_package( GMTL REQUIRED )
endif()

if(APPLE)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    mark_as_advanced(COREFOUNDATION_LIBRARY)
endif(APPLE)

POCOMenuSetup()

POCOFinder()

#include( FindOSGHelper )



if( QT_FOUND )
    include( ${QT_USE_FILE} )
else()
    if( QT_QMAKE_EXECUTABLE AND NOT QT4_QGLOBAL_H_FILE )
        message( STATUS "Qt was not found because the qglobal.h file was not found." )
    endif()
endif()

# Must use Boost as dynamic libs.
add_definitions( -DBOOST_ALL_DYN_LINK )

include (Findjag3d )

include (Findswitchwire)

include( CMakeMacros )


add_subdirectory( src )
#add_subdirectory( perftests )
#add_subdirectory( tritest )

#install pdb files for release with debug info builds
#if( MSVC )
#    install(
#        DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/\${CMAKE_INSTALL_CONFIG_NAME}/
#        DESTINATION lib
#        USE_SOURCE_PERMISSIONS
#        COMPONENT libjag3d
#        FILES_MATCHING PATTERN "*.pdb"
#    )
#endif()


# Install the CMake find script
#install( FILES "extra/Findjag3d.cmake"
#    DESTINATION "share/${CMAKE_PROJECT_NAME}/extra"
#    COMPONENT libjag3d
#)

