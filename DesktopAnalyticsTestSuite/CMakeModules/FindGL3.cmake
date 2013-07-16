# Finds the gl3.h header file.
# Looks for GL/gl3.h, GL3/gl3.h, or OpenGL/gl3.h (for OSX compatibility).
# 
# This script defines the following:
#  GL3_FOUND // Set to TRUE if gl3.h is found
#  GL3_INCLUDE_DIR // Parent directory of GL3 directory containing gl3.h.
#  GL3_GLCOREARB_HEADER // advanced
#  GL3_GL3_HEADER // advanced
#
# GL3_ROOT can be set as an environment variable or a CMake entry
# to the parent directory of the GL (or GL3 or OpenGL) directory containing gl3.h.
#
# Note: glcorearb.h replaced gl3.h in mid-2012.
#  - If this script finds glcorearb.h, but does not find gl3.h,
#    it will display a warning but set GL3_FOUND to TRUE and
#    set GL3_INCLUDE_DIR to the directory where GL3/glcorearb.h
#    was found.
#  - If it finds gl3.h but not glcorearb.h, no warning will be
#    displayed (for backwards compatiblity).
# To use this script with the latest glcorearb.h, add a gl3.h
# stub to the same directory that indirectly includes glcorearb.h.


FIND_PATH( GL3_GL3_HEADER 
    NAMES GL/gl3.h GL3/gl3.h OpenGL/gl3.h
    HINTS ${GL3_ROOT}
    PATHS ENV GL3_ROOT
)
FIND_PATH( GL3_GLCOREARB_HEADER
    NAMES GL/glcorearb.h GL3/glcorearb.h OpenGL/glcorearb.h
    HINTS ${GL3_ROOT}
    PATHS ENV GL3_ROOT
)

if( GL3_GLCOREARB_HEADER AND NOT GL3_GL3_HEADER )
    message( WARNING "Found glcorearb.h but not gl3.h." )
    set( GL3_INCLUDE_DIR ${GL3_GLCOREARB_HEADER} )
elseif( GL3_GL3_HEADER )
    set( GL3_INCLUDE_DIR ${GL3_GL3_HEADER} )
else()
    set( GL3_INCLUDE_DIR )
endif()


# handle the QUIETLY and REQUIRED arguments and set
# GL3_FOUND to TRUE as appropriate
INCLUDE( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( GL3 DEFAULT_MSG GL3_INCLUDE_DIR )

MARK_AS_ADVANCED(
    GL3_INCLUDE_DIR
    GL3_GLCOREARB_HEADER
    GL3_GL3_HEADER
)
