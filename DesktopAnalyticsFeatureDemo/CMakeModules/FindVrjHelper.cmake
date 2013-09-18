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

# Macro to force the FindVrj.cmake script to
# search again for Vrj.
macro( unFindVrj )
#    message( STATUS "In unFindVrj" )

    unset( "Vrj_INCLUDE_DIRS" CACHE )
    unset( "Vrj_LIBRARIES" CACHE )
    unset( "Vrj_FOUND" CACHE )
endmacro( unFindVrj )


macro( VrjMenuSetup )
    # What type of Vrj should we look for? This is a combo box
    # supporting three types of Vrj installations:
    #  * Default installation (in which the stock FindVrj.cmake script does all the work)
    #  * Alternate Install Location (user must set the VrjInstallLocation variable)
    #  * Source And Build Tree (user must supply both the VrjSourceRoot and VrjBuildRoot variables)
    set( VrjInstallType "Default Installation" CACHE STRING "Type of Vrj install: 'Default Installation', 'Alternate Install Location', or 'Source And Build Tree'." )
    set_property( CACHE VrjInstallType PROPERTY STRINGS "Default Installation" "Alternate Install Location" "Source And Build Tree" )

    # We need to detect when the user changes the Vrj install type
    # or any of the related directory variables, so that we'll know
    # to call unFindVrj() and force the stock Vrj script to search
    # again. To do this, we save the last set value of these variables
    # in the CMake cache as internal (hidden) variables.
    if( NOT DEFINED _lastVrjInstallType )
        set( _lastVrjInstallType "empty" CACHE INTERNAL "" )
    endif()
    if( NOT DEFINED _lastVrjInstallLocation )
        set( _lastVrjInstallLocation "empty" CACHE INTERNAL "" )
    endif()
    if( NOT DEFINED _lastVrjSourceRoot )
        set( _lastVrjSourceRoot "empty" CACHE INTERNAL "" )
    endif()
    if( NOT DEFINED _lastVrjBuildRoot )
        set( _lastVrjBuildRoot "empty" CACHE INTERNAL "" )
    endif()

    if( NOT DEFINED VrjInstallLocation )
        set( VrjInstallLocation "Please specify" )
    endif()
    if( NOT DEFINED VrjSourceRoot )
        set( VrjSourceRoot "Please specify" )
    endif()
    if( NOT DEFINED VrjBuildRoot )
        set( VrjBuildRoot "Please specify" )
    endif()
endmacro()



macro( VrjFinder )
    # If the user has changed the Vrj install type combo box
    # (or it's a clean cache), then set or unset our related
    # Vrj directory search variables.
    if( NOT ( ${VrjInstallType} STREQUAL ${_lastVrjInstallType} ) )
    #    message( STATUS "NOT ( ${VrjInstallType} STREQUAL ${_lastVrjInstallType} )" )

        if( VrjInstallType STREQUAL "Default Installation" )
            # Remove our helper variables and tell the stock script to search again.
            unset( VrjInstallLocation CACHE )
            unset( VrjSourceRoot CACHE )
            unset( VrjBuildRoot CACHE )
        elseif( VrjInstallType STREQUAL "Alternate Install Location" )
            # Enable just the VrjInstallLocation helper variable.
            set( VrjInstallLocation "Please specify" CACHE PATH "Root directory where Vrj is installed" )
            unset( VrjSourceRoot CACHE )
            unset( VrjBuildRoot CACHE )
        elseif( VrjInstallType STREQUAL "Source And Build Tree" )
            # Enable the VrjSourceRoot and VrjBuildRoot helper variables.
            unset( VrjInstallLocation CACHE )
            set( VrjSourceRoot "Please specify" CACHE PATH "Root directory of Vrj source tree" )
            set( VrjBuildRoot "Please specify" CACHE PATH "Root directory of Vrj build tree" )
        endif()
    endif()

    # Look for conditions that require us to find Vrj again.
    set( _needToFindVrj FALSE )
    if( VrjInstallType STREQUAL "Default Installation" )
        if( NOT ( ${VrjInstallType} STREQUAL ${_lastVrjInstallType} ) )
    #        message( STATUS "Need to find: case A" )
            set( _needToFindVrj TRUE )
        endif()
    elseif( VrjInstallType STREQUAL "Alternate Install Location" )
        if( NOT ( "${VrjInstallLocation}" STREQUAL "${_lastVrjInstallLocation}" ) )
    #        message( STATUS "Need to find: case B" )
            set( _needToFindVrj TRUE )
        endif()
    elseif( VrjInstallType STREQUAL "Source And Build Tree" )
        if( ( NOT ( "${VrjSourceRoot}" STREQUAL "${_lastVrjSourceRoot}" ) ) OR
            ( NOT ( "${VrjBuildRoot}" STREQUAL "${_lastVrjBuildRoot}" ) ) )
    #        message( STATUS "Need to find: cade C" )
            set( _needToFindVrj TRUE )
        endif()
    endif()
    if( _needToFindVrj )
        unFindVrj()
        set( _lastVrjInstallType ${VrjInstallType} CACHE INTERNAL "" FORCE )
        set( _lastVrjInstallLocation ${VrjInstallLocation} CACHE INTERNAL "" FORCE )
        set( _lastVrjSourceRoot ${VrjSourceRoot} CACHE INTERNAL "" FORCE )
        set( _lastVrjBuildRoot ${VrjBuildRoot} CACHE INTERNAL "" FORCE )
    endif()



    # Save internal variables for later restoration
    set( CMAKE_PREFIX_PATH_SAVE ${CMAKE_PREFIX_PATH} )
    set( CMAKE_LIBRARY_PATH_SAVE ${CMAKE_LIBRARY_PATH} )

    set( CMAKE_PREFIX_PATH
        ${VrjInstallLocation}
        ${VrjSourceRoot}
        ${VrjBuildRoot} )
    set( CMAKE_LIBRARY_PATH
        ${VrjInstallLocation}
        ${VrjBuildRoot} )


    find_package( Vrj 3.1.6 REQUIRED gadget jccl sonix vpr vrj_ogl )


    # Restore internal variables
    set( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH_SAVE} )
    set( CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH_SAVE} )
endmacro()
