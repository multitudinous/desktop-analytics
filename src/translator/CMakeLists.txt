file( GLOB srcs "*.cxx" )
file( GLOB srcsCpp "*.cpp" )
file( GLOB hdrs "*.h" )
list(APPEND srcs ${srcsCpp})

#set(VES_PRIVATE_HEADERS ${hdrs})
set(VES_PUBLIC_HEADERS ${hdrs} )

set(VES_TARGET_NAME jtToive)
set(VES_TARGET_VERSION ${ves_VERSION})
set(VES_TARGET_EXPORT ${PROJECT_NAME}-targets)
set(VES_TARGET_CATEGORY App)

include_directories(${OSG_INCLUDE_DIRS})

add_executable(${VES_TARGET_NAME} ${srcs} ${VES_PUBLIC_HEADERS})

target_link_libraries(${VES_TARGET_NAME} LINK_PUBLIC ${POCO_LIBRARIES})
target_link_libraries(${VES_TARGET_NAME} LINK_PUBLIC ${OSG_LIBRARIES})
target_link_libraries(${VES_TARGET_NAME} LINK_PUBLIC ${Boost_LIBRARIES})
target_link_libraries(${VES_TARGET_NAME} LINK_PUBLIC ${crunchstore_LIBRARIES})
target_link_libraries(${VES_TARGET_NAME} LINK_PUBLIC ${OSGWORKS_LIBRARIES})

include(VESModuleInstall)

install(FILES ptconfigfile.txt 
        DESTINATION ${VES_INSTALL_BINDIR}
        COMPONENT runtime)

