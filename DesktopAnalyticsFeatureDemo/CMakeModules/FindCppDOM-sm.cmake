# Find CppDOM
# Find the native CppDOM includes and library
# This module defines
#  CPPDOM_INCLUDE_DIRS
#  CPPDOM_LIBRARIES, libraries to link against to use CppDOM.
#  CPPDOM_VERSION, package version in CMake decimal version# format (e.g., 3.1.6)
#  CPPDOM_FOUND (Note capitalization), If false, do not try to use CppDOM.
#
# Search hint (in CMake or as an environment variable):
#  CppDOM_ROOT, the CppDOM install directory root.


# Find the main CppDOM header.
unset( CPPDOM_INCLUDE_DIRS )
find_path( CPPDOM_INCLUDE_DIRS cppdom/cppdom.h
    PATHS ${CppDOM_ROOT} ENV CppDOM_ROOT
    PATH_SUFFIXES include
)


# Set CPPDOM_VERSION.
# Also set _libVersionString, which will be appended to the lib name.
set( CPPDOM_VERSION )
if( CPPDOM_INCLUDE_DIRS )
    set( _versionFile "${CPPDOM_INCLUDE_DIRS}/cppdom/version.h" )
    if( NOT EXISTS ${_versionFile} )
        message( SEND_ERROR "Can't find ${_versionFile}" )
    else()
        file( READ "${_versionFile}" _versionContents )
        string( REGEX REPLACE ".*#define CPPDOM_VERSION_MAJOR[ \t]+([0-9]+).*CPPDOM_VERSION_MINOR[ \t]+([0-9]+).*CPPDOM_VERSION_PATCH[ \t]+([0-9]+).*"
            "-\\1_\\2_\\3" _libVersionString ${_versionContents} )

        # Create CPPDOM_VERSION from _libVersionString
        # _libVersionString is in form "-<M>_<m>_<p>"
        #    but we need CPPDOM_VERSION in form "<M>.<m>.<p>".
        string( REGEX REPLACE "-([0-9]+)_([0-9]+)_([0-9]+)"
            "\\1.\\2.\\3" CPPDOM_VERSION ${_libVersionString} )

#        message( STATUS "versions: ${_libVersionString} ${CPPDOM_VERSION}" )
    endif()
endif()


# Find the library.
unset( CPPDOM_LIBRARIES )
unset( "CPPDOM_LIBRARY" CACHE )
set( _libName cppdom${_libVersionString} )
find_library( CPPDOM_LIBRARY
    NAMES ${_libName}
    PATHS ${CppDOM_ROOT} ENV CppDOM_ROOT
    PATH_SUFFIXES lib64
)
if( NOT CPPDOM_LIBRARY )
    message( WARNING "Could not find library ${_libName}" )
else()
    list( APPEND CPPDOM_LIBRARIES "optimized" ${CPPDOM_LIBRARY} )
    mark_as_advanced( CPPDOM_LIBRARY )
endif()

unset( "CPPDOM_LIBRARY_DEBUG" CACHE )
set( _libName CPPDOM_d${_libVersionString} )
find_library( CPPDOM_LIBRARY_DEBUG
    NAMES ${_libName}
    PATHS ${CppDOM_ROOT} ENV CppDOM_ROOT
    PATH_SUFFIXES lib64
)
if( NOT CPPDOM_LIBRARY_DEBUG )
    message( WARNING "Could not find library ${_libName}" )
else()
    list( APPEND CPPDOM_LIBRARIES "debug" ${CPPDOM_LIBRARY_DEBUG} )
    mark_as_advanced( CPPDOM_LIBRARY_DEBUG )
endif()


# handle the QUIETLY and REQUIRED arguments and set CPPDOM_FOUND to TRUE if
# all listed variables are TRUE
include( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( CppDOM
    REQUIRED_VARS CPPDOM_INCLUDE_DIRS CPPDOM_LIBRARIES
    VERSION_VAR CPPDOM_VERSION
)


mark_as_advanced(
    CPPDOM_INCLUDE_DIRS
    CPPDOM_LIBRARIES
    CPPDOM_VERSION
)
