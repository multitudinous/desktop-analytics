# - Find a Jag3D installation or build tree.
# The following variables are set if Jag3D is found.  If Jag3D is not
# found, jag3d_FOUND is set to false.
#  jag3d_FOUND         - Set to true when Jag3D is found.
#  jag3d_USE_FILE      - Include this CMake file to use Jag3D.
#  jag3d_MAJOR_VERSION - The Jag3D major version number.
#  jag3d_MINOR_VERSION - The Jag3D minor version number 
#                       (odd non-release).
#  jag3d_BUILD_VERSION - The Jag3D patch level 
#                       (meaningless for odd minor).
#  jag3d_INCLUDE_DIRS  - Include directories for Jag3D.
#  jag3d_LIBRARY_DIRS  - Link directories for Jag3D libraries

# The following cache entries must be set by the user to locate Jag3D:
#  jag3d_DIR  - The directory containing jag3dConfig.cmake.  
#             This is either the root of the build tree,
#             or the lib directory.  This is the 
#             only cache entry.


# Assume not found.
set( jag3d_FOUND 0 )

# Construct consitent error messages for use below.
set( jag3d_DIR_DESCRIPTION "directory containing jag3dConfig.cmake.  This is either the root of the build tree, or PREFIX/lib for an installation." )
set( jag3d_DIR_MESSAGE "Jag3D not found.  Set the jag3d_DIR cmake cache entry to the ${jag3d_DIR_DESCRIPTION}" )

# Use the Config mode of the find_package() command to find jag3dConfig.
# If this succeeds (possibly because jag3d_DIR is already set), the
# command will have already loaded jag3dConfig.cmake and set jag3d_FOUND.
if( NOT jag3d_FOUND )
    find_package( jag3d QUIET NO_MODULE )
endif()

#-----------------------------------------------------------------------------
if( NOT jag3d_FOUND )
    # Jag3D not found, explain to the user how to specify its location.
    if( jag3d_FIND_REQUIRED )
        message( FATAL_ERROR ${jag3d_DIR_MESSAGE} )
    else()
        if( NOT jag3d_FIND_QUIETLY )
            message( STATUS ${jag3d_DIR_MESSAGE} )
        endif( NOT jag3d_FIND_QUIETLY )
    endif()
endif()
