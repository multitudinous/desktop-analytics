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

# Freeglut header forces implicit linking with freeglut.lib by
# default. If you have built freeglut as static, add the
# FREEGLUT_STATIC entry in CMake and select it, which will
# #define FREEGLUT_STATIC in the compile, so that the freeglut.h
# header will instead implicitly link against freeglut_static.lib.
if( FREEGLUT_STATIC )
    add_definitions( -DFREEGLUT_STATIC )
endif()


# Macro to force the FindFreeglut.cmake script to
# search again for Freeglut.
macro( unFindFreeglut )
#    message( STATUS "In unFindFreeglut" )

    unset( "Freeglut_INCLUDE_DIR" CACHE )
    unset( "Freeglut_LIBRARIES" CACHE )
    unset( "Freeglut_FOUND" CACHE )
    unset( "Freeglut_glut_LIBRARY" CACHE )
endmacro( unFindFreeglut )


macro( FreeglutMenuSetup )
    # What type of Freeglut should we look for? This is a combo box
    # supporting three types of Freeglut installations:
    #  * Default installation (in which the stock FindFreeglut.cmake script does all the work)
    #  * Alternate Install Location (user must set the FreeglutInstallLocation variable)
    #  * Source And Build Tree (user must supply both the FreeglutSourceRoot and FreeglutBuildRoot variables)
    set( FreeglutInstallType "Default Installation" CACHE STRING "Type of Freeglut install: 'Default Installation', 'Alternate Install Location', or 'Source And Build Tree'." )
    set_property( CACHE FreeglutInstallType PROPERTY STRINGS "Default Installation" "Alternate Install Location" "Source And Build Tree" )

    # We need to detect when the user changes the Freeglut install type
    # or any of the related directory variables, so that we'll know
    # to call unFindFreeglut() and force the stock Freeglut script to search
    # again. To do this, we save the last set value of these variables
    # in the CMake cache as internal (hidden) variables.
    if( NOT DEFINED _lastFreeglutInstallType )
        set( _lastFreeglutInstallType "empty" CACHE INTERNAL "" )
    endif()
    if( NOT DEFINED _lastFreeglutInstallLocation )
        set( _lastFreeglutInstallLocation "empty" CACHE INTERNAL "" )
    endif()
    if( NOT DEFINED _lastFreeglutSourceRoot )
        set( _lastFreeglutSourceRoot "empty" CACHE INTERNAL "" )
    endif()
    if( NOT DEFINED _lastFreeglutBuildRoot )
        set( _lastFreeglutBuildRoot "empty" CACHE INTERNAL "" )
    endif()

    if( NOT DEFINED FreeglutInstallLocation )
        set( FreeglutInstallLocation "Please specify" )
    endif()
    if( NOT DEFINED FreeglutSourceRoot )
        set( FreeglutSourceRoot "Please specify" )
    endif()
    if( NOT DEFINED FreeglutBuildRoot )
        set( FreeglutBuildRoot "Please specify" )
    endif()
endmacro()



macro( FreeglutFinder )
    # If the user has changed the Freeglut install type combo box
    # (or it's a clean cache), then set or unset our related
    # Freeglut directory search variables.
    if( NOT ( ${FreeglutInstallType} STREQUAL ${_lastFreeglutInstallType} ) )
    #    message( STATUS "NOT ( ${FreeglutInstallType} STREQUAL ${_lastFreeglutInstallType} )" )

        if( FreeglutInstallType STREQUAL "Default Installation" )
            # Remove our helper variables and tell the stock script to search again.
            unset( FreeglutInstallLocation CACHE )
            unset( FreeglutSourceRoot CACHE )
            unset( FreeglutBuildRoot CACHE )
        elseif( FreeglutInstallType STREQUAL "Alternate Install Location" )
            # Enable just the FreeglutInstallLocation helper variable.
            set( FreeglutInstallLocation "Please specify" CACHE PATH "Root directory where Freeglut is installed" )
            unset( FreeglutSourceRoot CACHE )
            unset( FreeglutBuildRoot CACHE )
        elseif( FreeglutInstallType STREQUAL "Source And Build Tree" )
            # Enable the FreeglutSourceRoot and FreeglutBuildRoot helper variables.
            unset( FreeglutInstallLocation CACHE )
            set( FreeglutSourceRoot "Please specify" CACHE PATH "Root directory of Freeglut source tree" )
            set( FreeglutBuildRoot "Please specify" CACHE PATH "Root directory of Freeglut build tree" )
        endif()
    endif()

    # Look for conditions that require us to find Freeglut again.
    set( _needToFindFreeglut FALSE )
    if( FreeglutInstallType STREQUAL "Default Installation" )
        if( NOT ( ${FreeglutInstallType} STREQUAL ${_lastFreeglutInstallType} ) )
    #        message( STATUS "Need to find: case A" )
            set( _needToFindFreeglut TRUE )
        endif()
    elseif( FreeglutInstallType STREQUAL "Alternate Install Location" )
        if( NOT ( "${FreeglutInstallLocation}" STREQUAL "${_lastFreeglutInstallLocation}" ) )
    #        message( STATUS "Need to find: case B" )
            set( _needToFindFreeglut TRUE )
        endif()
    elseif( FreeglutInstallType STREQUAL "Source And Build Tree" )
        if( ( NOT ( "${FreeglutSourceRoot}" STREQUAL "${_lastFreeglutSourceRoot}" ) ) OR
            ( NOT ( "${FreeglutBuildRoot}" STREQUAL "${_lastFreeglutBuildRoot}" ) ) )
    #        message( STATUS "Need to find: cade C" )
            set( _needToFindFreeglut TRUE )
        endif()
    endif()
    if( _needToFindFreeglut )
        unFindFreeglut()
        set( _lastFreeglutInstallType ${FreeglutInstallType} CACHE INTERNAL "" FORCE )
        set( _lastFreeglutInstallLocation ${FreeglutInstallLocation} CACHE INTERNAL "" FORCE )
        set( _lastFreeglutSourceRoot ${FreeglutSourceRoot} CACHE INTERNAL "" FORCE )
        set( _lastFreeglutBuildRoot ${FreeglutBuildRoot} CACHE INTERNAL "" FORCE )
    endif()



    # Save internal variables for later restoration
    set( CMAKE_PREFIX_PATH_SAVE ${CMAKE_PREFIX_PATH} )
    set( CMAKE_LIBRARY_PATH_SAVE ${CMAKE_LIBRARY_PATH} )

    set( CMAKE_PREFIX_PATH
        ${FreeglutInstallLocation}
        ${FreeglutSourceRoot}
        ${FreeglutBuildRoot} )
    set( CMAKE_LIBRARY_PATH
        ${FreeglutInstallLocation}
        ${FreeglutBuildRoot} )


    find_package( Freeglut )


    # Restore internal variables
    set( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH_SAVE} )
    set( CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH_SAVE} )
endmacro()
