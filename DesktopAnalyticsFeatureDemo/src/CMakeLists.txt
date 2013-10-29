
QT4_WRAP_CPP(jag_HEADERS_MOC qtGlWidget.h WarrantyToolPlugin_UIDialog.h QueryResults.h)



QT4_WRAP_UI(datest_FORMS_HEADERS datest.ui WarrantyToolPlugin_UIDialog.ui QueryResults.ui)


ADD_EXECUTABLE( desktopanalytics
    jagmodel.cpp
    qtSupport.cpp
    ToggleByNameVisitor.cpp
    ToggleByNamesVisitor.cpp
    HighlightNodeByNamesVisitor.cpp
    WarrantyToolGP.cxx
    WarrantyToolPlugin_UIDialog.cxx
    NaturalSortQTreeWidgetItem.cxx
    QueryResults.cxx
    RemoveNodeNameVisitor.cpp
    AbufferCallBack.cpp
    csvparser.cpp
    ToggleByNameVisitor.h
    ToggleByNamesVisitor.h
    HighlightNodeByNamesVisitor.h
    RemoveNodeNameVisitor.h
    AbufferCallBack.h
    WarrantyToolGP.h
	DemoInterface.h
	qtGlWidget.h
	jagmodel.h
	QueryResults.h
	alphanum.h
	NaturalSortQTreeWidgetItem.h
    WarrantyToolPlugin_UIDialog.h
    csvparser.h
    ${jag_HEADERS_MOC}
    ${datest_FORMS_HEADERS}
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
    ${switchwire_INCLUDE_DIRS}
)

target_link_libraries(desktopanalytics 
    ${POCO_LIBRARIES}
    ${Boost_LIBRARIES}
    ${OPENGL_gl_LIBRARY}
    ${jag3d_LIBRARIES}
    ${QT_QTOPENGL_LIBRARY}
    ${QT_LIBRARIES}	
    ${switchwire_LIBRARIES}
)