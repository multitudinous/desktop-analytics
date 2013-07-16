
#QT4_WRAP_CPP(jag_HEADERS_MOC jagmodel.cpp DemoInterface.h qtGlWidget.h)



#QT4_WRAP_UI(datest_FORMS_HEADERS datest.ui)


ADD_EXECUTABLE( mapPerf
    maptest.cpp
)

include_directories(
    ${POCO_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
    ${GMTL_INCLUDE_DIR}
    ${GL3_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIRS}
    ${jag3d_INCLUDE_DIRS}
    ${GL3_GL3_HEADER}
    ${QT_INCLUDE_DIR}
    ${QT_QTOPENGL_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(mapPerf 
    ${POCO_LIBRARIES}
    ${Boost_LIBRARIES}
    ${OPENGL_gl_LIBRARY}
    ${jag3d_LIBRARIES}
    ${QT_QTOPENGL_LIBRARY}
    ${QT_LIBRARIES}	
)