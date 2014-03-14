SET( LIBRARY_VERSION "0.1.0" )
SET( LIBRARY_SOVERSION "0.1" )

SET( CMAKE_CXX_COMPILER /usr/bin/g++)
SET( CMAKE_C_COMPILER /usr/bin/gcc)

SET( HEADER_PATH ./ )
FILE(GLOB folder_source *.mm)
FILE(GLOB folder_header ${HEADER_PATH}/*.h)

SOURCE_GROUP("Source Files" FILES ${folder_source})
SOURCE_GROUP("Header Files" FILES ${folder_header})


find_package(OpenGL REQUIRED)

find_package(Qt4 REQUIRED) # find and setup Qt4 for this project
set(QT_USE_OPENGL TRUE)
include(${QT_USE_FILE})

include_directories(
   ${QT_INCLUDE_DIR}
   ${QT_QTGUI_INCLUDE_DIR}
   ${QT_QTOPENGL_INCLUDE_DIR}
   ${HEADER_PATH}/
)


#message(${QT_LIBRARIES} ${QT_QTGUI_LIBRARIES})

set(LIBS "-framework Foundation" "-framework Cocoa" ${QT_LIBRARIES} ${QT_QTGUI_LIBRARIES} ${QT_QTOPENGL_LIBRARIES} ${OPENGL_LIBRARIES})

ADD_LIBRARY( coreprofileselector SHARED
        ${folder_source}
        ${folder_header}
)

SET_TARGET_PROPERTIES( coreprofileselector PROPERTIES
        VERSION ${LIBRARY_VERSION}
        SOVERSION ${LIBRARY_SOVERSION}
)

TARGET_LINK_LIBRARIES( coreprofileselector  ${LIBS} )

INSTALL( TARGETS coreprofileselector DESTINATION ${LIB_INSTALL_DIR} )
