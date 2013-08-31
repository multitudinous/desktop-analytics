cmake_minimum_required (VERSION 2.8.8)

project( da )

SET( CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules" )

set(da_MAJOR_VERSION 0)
set(da_MINOR_VERSION 1)
set(da_SUB_VERSION 0)
set(da_VERSION ${da_MAJOR_VERSION}.${da_MINOR_VERSION}.${da_SUB_VERSION})
set(VES_PACKAGE_VERSION ${da_VERSION})

#Set the include directory so that switchwire can be used as a sub project
if( NOT(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME}) )
    set(da_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/src" PARENT_SCOPE)
    #set(da_LIBRARIES "da" PARENT_SCOPE)
endif()

set(POCO_ROOT "Please specify" CACHE PATH "Root directory where POCO is installed" )
find_package(POCO 1.5.0 REQUIRED COMPONENTS PocoFoundation PocoUtil PocoXML PocoData PocoDataSQLite)

set(BOOST_ROOT "Please specify" CACHE PATH "Root directory where Boost is installed" )
set(BOOST_INCLUDEDIR "$ENV{BOOST_INSTALL_DIR}/include" CACHE PATH "docstring")
add_definitions( -DBOOST_ALL_NO_LIB )
add_definitions( -DBOOST_ALL_DYN_LINK )

set(Boost_USE_STATIC_LIBS       OFF)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

find_package(Boost 1.53.0 REQUIRED COMPONENTS filesystem system regex)

include(FindOSGHelper)
find_package( OpenGL )

find_package( crunchstore 0.2.0 REQUIRED )
if( crunchstore_FOUND )
    include( ${crunchstore_USE_FILE} )
endif(crunchstore_FOUND)

include_directories( 
    ${Boost_INCLUDE_DIRS}
    ${POCO_INCLUDE_DIR})

option(BUILD_EXAMPLE "Build example?" ON)

include(VESSetupCompiler)

include(VESBinAndLibDirs)

add_subdirectory(src)

#
# Doxygen
#find_package( Doxygen )
#if( DOXYGEN_FOUND )
#    set( HAVE_DOT "NO" )
#    if( DOXYGEN_DOT_PATH )
#        set( HAVE_DOT "YES" )
#    endif()
#    
#    configure_file( ${PROJECT_SOURCE_DIR}/doc/doxyfile.cmake
#        ${PROJECT_BINARY_DIR}/doc/doxyfile
#    )
#    add_custom_target( Documentation ${DOXYGEN_EXECUTABLE}
#        ${PROJECT_BINARY_DIR}/doc/doxyfile
#    )
#    
#    add_subdirectory( doc/images )
#endif()

include(VESInstallPDBFiles)

#set(VES_EXPORT_LIBRARY_TARGETS "")
set(VES_CONFIGURE_VARS CMAKE_INSTALL_LIBDIR)

#
# Setup the fpc auto detection files
#if(WIN32)
#    set(switchwire_FPC_LIBRARIES "/libpath:\"\${libdir}\" switchwire.lib")
#    set(switchwire_FPC_INCLUDES "/I\"\${includedir}\"")
#else(WIN32)
#    set(switchwire_FPC_INCLUDES "-I\${includedir}")
#    set(switchwire_FPC_LIBRARIES "-L\${libdir} -lswitchwire")
#endif(WIN32)

#include(VESInstallAutoDiscoveryTools)
