# Find VR Juggler
# Find the native VR Juggler includes and libraries
# This module defines
#  Vrj_INCLUDE_DIRS
#  Vrj_LIBRARIES, libraries to link against to use VR Juggler.
#  Vrj_VERSION, package version in CMake decimal version# format (e.g., 3.1.6)
#  VRJ_FOUND (Note capitalization), If false, do not try to use VR Juggler.
#
# Suppert environment variable search hint:
#  Vrj_ROOT, VR Juggler install directory root.


# Find the main Vrj header.
unset( Vrj_INCLUDE_DIRS )
find_path( Vrj_INCLUDE_DIRS vrj/vrjDefines.h
    PATHS ${Vrj_ROOT} ENV Vrj_ROOT
    PATH_SUFFIXES include
)


# Error if we didn't find VRJ's GL3 branch.
if (Vrj_INCLUDE_DIRS )
    set( _gl3File "${Vrj_INCLUDE_DIRS}/vrj/Draw/OpenGL/GL3/gl3.h" )
    if( NOT EXISTS ${_gl3File} )
        message( STATUS "Can't find ${_gl3File}" )
        message( SEND_ERROR "JAG3D requires the GL3 branch of VR Juggler." )
    endif()
endif()


# Set Vrj_VERSION.
set( Vrj_VERSION )
if( Vrj_INCLUDE_DIRS )
    set( _versionFile "${Vrj_INCLUDE_DIRS}/vrj/vrjParam.h" )
    if( NOT EXISTS ${_versionFile} )
        message( SEND_ERROR "Can't find ${_versionFile}" )
    else()
        file( READ "${_versionFile}" _versionContents )
        string( REGEX REPLACE ".*#define __VJ_MAJOR__[ \t]+([0-9]+).*__VJ_MINOR__[ \t]+([0-9]+).*__VJ_PATCH__[ \t]+([0-9]+).*"
            "\\1.\\2.\\3" Vrj_VERSION ${_versionContents} )
    endif()
endif()

function( _getLibVersion _inDir _inSubstring _outVersion )
#    message( STATUS "_getLibVersion ${_inDir} ${_inSubstring} ${${_outVersion}}" )
    set( _versionFile "${Vrj_INCLUDE_DIRS}/${_inDir}/${_inDir}Param.h" )
    if( NOT EXISTS ${_versionFile} )
        message( SEND_ERROR "_getLibVersion: Can't find ${_versionFile}" )
    else()
        file( READ "${_versionFile}" _versionContents )
        string( REGEX REPLACE ".*#define __${_inSubstring}_MAJOR__[ \t]+([0-9]+).*__${_inSubstring}_MINOR__[ \t]+([0-9]+).*__${_inSubstring}_PATCH__[ \t]+([0-9]+).*"
            "-\\1_\\2_\\3" _result ${_versionContents} )
    endif()
    set( ${_outVersion} ${_result} PARENT_SCOPE )
endfunction()


# Get a list of requested Vrj components. 'vrj' is assumed as always requested.
set( _requestedComponents )
# vrj is always present.
list( APPEND _requestedComponents "vrj" )
#
foreach( _component ${Vrj_FIND_COMPONENTS})
    list( APPEND _requestedComponents ${_component} )
endforeach()
list( REMOVE_DUPLICATES _requestedComponents )

#message( STATUS "${_requestedComponents}" )

# Find each library.
set( Vrj_LIBRARIES )
foreach( lib ${_requestedComponents} )
    # Get the library version string.
    #   lib     dir    substring
    #   ------  ------ ---------
    #   vrj     vrj    VJ
    #   vrj_ogl vrj    VJ
    #   gadget  gadget GADGET
    #   jccl    jccl   JCCL
    #   sonix   snx    SNX
    #   vpr     vpr    VPR
    if( "${lib}" STREQUAL "sonix" )
        set( _dir "snx" )
    elseif( "${lib}" STREQUAL "vrj_ogl" )
        set( _dir "vrj" )
    else()
        set( _dir "${lib}" )
    endif()
    if( "${lib}" STREQUAL "vrj" OR "${lib}" STREQUAL "vrj_ogl" )
        set( _substring "VJ" )
    else()
        string( TOUPPER ${_dir} _substring )
    endif()

    _getLibVersion( ${_dir} ${_substring} _versionString )
#    message( STATUS "${lib} ${_dir} ${_versionString}" )

    unset( "Vrj_${lib}_LIBRARY" CACHE )
    find_library( Vrj_${lib}_LIBRARY
        NAMES ${lib}${_versionString}
        PATHS ${Vrj_ROOT} ENV Vrj_ROOT
    )
    if( NOT Vrj_${lib}_LIBRARY )
        message( WARNING "Could not find Vrj component library ${lib}" )
    else()
        list( APPEND Vrj_LIBRARIES "optimized" ${Vrj_${lib}_LIBRARY} )
        mark_as_advanced( Vrj_${lib}_LIBRARY )
    endif()

    unset( "Vrj_${lib}_LIBRARY_DEBUG" CACHE )
    find_library( Vrj_${lib}_LIBRARY_DEBUG
        NAMES ${lib}_d${_versionString}
        PATHS ${Vrj_ROOT} ENV Vrj_ROOT
        PATH_SUFFIXES debug
    )
    if( NOT Vrj_${lib}_LIBRARY_DEBUG )
        message( WARNING "Could not find Vrj component Debug library ${lib}" )
    else()
        list( APPEND Vrj_LIBRARIES "debug" ${Vrj_${lib}_LIBRARY_DEBUG} )
        mark_as_advanced( Vrj_${lib}_LIBRARY_DEBUG )
    endif()
endforeach()


# handle the QUIETLY and REQUIRED arguments and set VRJ_FOUND to TRUE if 
# all listed variables are TRUE
include( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( Vrj
    REQUIRED_VARS Vrj_INCLUDE_DIRS Vrj_LIBRARIES
    VERSION_VAR Vrj_VERSION
)


mark_as_advanced(
    Vrj_INCLUDE_DIRS
    Vrj_LIBRARIES
    Vrj_VERSION
)
