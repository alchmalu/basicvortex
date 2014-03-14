SET( LIBRARY_VERSION "0.1.0" )
SET( LIBRARY_SOVERSION "0.1" )


SET( HEADER_PATH ./ )
FILE(GLOB folder_source *.cpp)
FILE(GLOB folder_header ${HEADER_PATH}/*.h)

#SOURCE_GROUP("Source Files" FILES ${folder_source})
#SOURCE_GROUP("Header Files" FILES ${folder_header})

find_package(OpenGL REQUIRED)

find_package(Qt4 REQUIRED) # find and setup Qt4 for this project
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})


include_directories(
   ${QT_INCLUDE_DIR}
   ${QT_QTGUI_INCLUDE_DIR}
   ${QT_QTOPENGL_INCLUDE_DIR}
   ${CMAKE_SOURCE_DIR}/src/engine
   ${GLM_INC}
   ${ASSIMP_INC}
   ${HEADER_PATH}/
)

set(LIBS ${QT_LIBRARIES} ${QT_QTGUI_LIBRARIES} ${QT_QTOPENGL_LIBRARIES} ${OPENGL_LIBRARIES} ${ASSIMP_LIB})

ADD_LIBRARY( vortexengine SHARED
        ${folder_source}
        ${folder_header}
)

SET_TARGET_PROPERTIES( vortexengine PROPERTIES
        VERSION ${LIBRARY_VERSION}
        SOVERSION ${LIBRARY_SOVERSION}
)

TARGET_LINK_LIBRARIES( vortexengine ${LIBS} )

INSTALL( TARGETS vortexengine DESTINATION ${LIB_INSTALL_DIR} )
