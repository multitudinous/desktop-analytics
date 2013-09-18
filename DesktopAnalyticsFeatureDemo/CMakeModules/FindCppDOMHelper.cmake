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

# Macro to force the FindCppDOM.cmake script to
# search again for CppDOM.
macro( unFindCppDOM )
#    message( STATUS "In unFindCppDOM" )

    unset( "CPPDOM_INCLUDE_DIRS" CACHE )
    unset( "CPPDOM_LIBRARIES" CACHE )
    unset( "CPPDOM_FOUND" CACHE )
endmacro( unFindCppDOM )


macro( CppDOMMenuSetup )
    # What type of CppDOM should we look for? This is a combo box
    # supporting three types of CppDOM installations:
    #  * Default installation (in which the stock FindCppDOM.cmake script does all the work)
    #  * Alternate Install Location (user must set the CppDOMInstallLocation variable)
    #  * Source And Build Tree (user must supply both the CppDOMSourceRoot and CppDOMBuildRoot variables)
    set( CppDOMInstallType "Default Installation" CACHE STRING "Type of CppDOM install: 'Default Installation', 'Alternate Install Location', or 'Source And Build Tree'." )
    set_property( CACHE CppDOMInstallType PROPERTY STRINGS "Default Installation" "Alternate Install Location" "Source And Build Tree" )

    # We need to detect when the user changes the CppDOM install type
    # or any of the related directory variables, so that we'll know
    # to call unFindCppDOM() and force the stock CppDOM script to search
    # again. To do this, we save the last set value of these variables
    # in the CMake cache as internal (hidden) variables.
    if( NOT DEFINED _lastCppDOMInstallType )
        set( _lastCppDOMInstallType "empty" CACHE INTERNAL "" )
    endif()
    if( NOT DEFINED _lastCppDOMInstallLocation )
        set( _lastCppDOMInstallLocation "empty" CACHE INTERNAL "" )
    endif()
    if( NOT DEFINED _lastCppDOMSourceRoot )
        set( _lastCppDOMSourceRoot "empty" CACHE INTERNAL "" )
    endif()
    if( NOT DEFINED _lastCppDOMBuildRoot )
        set( _lastCppDOMBuildRoot "empty" CACHE INTERNAL "" )
    endif()

    if( NOT DEFINED CppDOMInstallLocation )
        set( CppDOMInstallLocation "Please specify" )
    endif()
    if( NOT DEFINED CppDOMSourceRoot )
        set( CppDOMSourceRoot "Please specify" )
    endif()
    if( NOT DEFINED CppDOMBuildRoot )
        set( CppDOMBuildRoot "Please specify" )
    endif()
endmacro()



macro( CppDOMFinder )
    # If the user has changed the CppDOM install type combo box
    # (or it's a clean cache), then set or unset our related
    # CppDOM directory search variables.
    if( NOT ( ${CppDOMInstallType} STREQUAL ${_lastCppDOMInstallType} ) )
    #    message( STATUS "NOT ( ${CppDOMInstallType} STREQUAL ${_lastCppDOMInstallType} )" )

        if( CppDOMInstallType STREQUAL "Default Installation" )
            # Remove our helper variables and tell the stock script to search again.
            unset( CppDOMInstallLocation CACHE )
            unset( CppDOMSourceRoot CACHE )
            unset( CppDOMBuildRoot CACHE )
        elseif( CppDOMInstallType STREQUAL "Alternate Install Location" )
            # Enable just the CppDOMInstallLocation helper variable.
            set( CppDOMInstallLocation "Please specify" CACHE PATH "Root directory where CppDOM is installed" )
            unset( CppDOMSourceRoot CACHE )
            unset( CppDOMBuildRoot CACHE )
        elseif( CppDOMInstallType STREQUAL "Source And Build Tree" )
            # Enable the CppDOMSourceRoot and CppDOMBuildRoot helper variables.
            unset( CppDOMInstallLocation CACHE )
            set( CppDOMSourceRoot "Please specify" CACHE PATH "Root directory of CppDOM source tree" )
            set( CppDOMBuildRoot "Please specify" CACHE PATH "Root directory of CppDOM build tree" )
        endif()
    endif()

    # Look for conditions that require us to find CppDOM again.
    set( _needToFindCppDOM FALSE )
    if( CppDOMInstallType STREQUAL "Default Installation" )
        if( NOT ( ${CppDOMInstallType} STREQUAL ${_lastCppDOMInstallType} ) )
    #        message( STATUS "Need to find: case A" )
            set( _needToFindCppDOM TRUE )
        endif()
    elseif( CppDOMInstallType STREQUAL "Alternate Install Location" )
        if( NOT ( "${CppDOMInstallLocation}" STREQUAL "${_lastCppDOMInstallLocation}" ) )
    #        message( STATUS "Need to find: case B" )
            set( _needToFindCppDOM TRUE )
        endif()
    elseif( CppDOMInstallType STREQUAL "Source And Build Tree" )
        if( ( NOT ( "${CppDOMSourceRoot}" STREQUAL "${_lastCppDOMSourceRoot}" ) ) OR
            ( NOT ( "${CppDOMBuildRoot}" STREQUAL "${_lastCppDOMBuildRoot}" ) ) )
    #        message( STATUS "Need to find: cade C" )
            set( _needToFindCppDOM TRUE )
        endif()
    endif()
    if( _needToFindCppDOM )
        unFindCppDOM()
        set( _lastCppDOMInstallType ${CppDOMInstallType} CACHE INTERNAL "" FORCE )
        set( _lastCppDOMInstallLocation ${CppDOMInstallLocation} CACHE INTERNAL "" FORCE )
        set( _lastCppDOMSourceRoot ${CppDOMSourceRoot} CACHE INTERNAL "" FORCE )
        set( _lastCppDOMBuildRoot ${CppDOMBuildRoot} CACHE INTERNAL "" FORCE )
    endif()



    # Save internal variables for later restoration
    set( CMAKE_PREFIX_PATH_SAVE ${CMAKE_PREFIX_PATH} )
    set( CMAKE_LIBRARY_PATH_SAVE ${CMAKE_LIBRARY_PATH} )

    set( CMAKE_PREFIX_PATH
        ${CppDOMInstallLocation}
        ${CppDOMSourceRoot}
        ${CppDOMBuildRoot} )
    set( CMAKE_LIBRARY_PATH
        ${CppDOMInstallLocation}
        ${CppDOMBuildRoot} )


    find_package( CppDOM 1.3.0 REQUIRED )


    # Restore internal variables
    set( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH_SAVE} )
    set( CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH_SAVE} )
endmacro()
