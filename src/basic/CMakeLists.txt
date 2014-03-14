find_package(Qt4 REQUIRED) # find and setup Qt4 for this project
set(QT_USE_OPENGL TRUE)
include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})


find_package(OpenGL REQUIRED)

# the variable "qtproject_SRCS" contains all .cpp files of this project
FILE(GLOB folder_source *.cpp)
FILE(GLOB folder_header *.h)
FILE(GLOB folder_shader  shaders/*.glsl shaders/*/*.glsl)
SOURCE_GROUP("Source Files" FILES ${folder_source})
SOURCE_GROUP("Header Files" FILES ${folder_header})
SOURCE_GROUP("Shader Files" FILES ${folder_shader})

# moc files
set(basic_MOC_HDRS
mainwindow.h
openglwidget.h
renderercontrol.h
)

set(basic_UIS
renderercontrol.ui
)


QT4_WRAP_UI(basic_UIS_H ${basic_UIS})
QT4_WRAP_CPP(basic_MOC_SRCS ${basic_MOC_HDRS})

include_directories( ./)

# tell cmake to create .moc files for all files in the variable qtproject_SRCS that require such a file.
# note: this assumes that you use #include "header.moc" in your files
#qt4_automoc(${openglrenderer_SRCS})

# the next line sets up include and link directories and defines some variables that we will use.
# you can modify the behavior by setting some variables, e.g.


include_directories(
   ${CMAKE_CURRENT_BINARY_DIR}
   ${QT_INCLUDE_DIR}
   ${QT_QTGUI_INCLUDE_DIR}
   ${QT_QTOPENGL_INCLUDE_DIR}
   ${CMAKE_SOURCE_DIR}/src/engine
   ${GLM_INC}
   ${ASSIMP_INC}
   ${CMAKE_SOURCE_DIR}/src/basic/
   /opt/local/include/tbb
)



if (APPLE)
link_directories(${LIB_INSTALL_DIR} /opt/local/lib)
add_executable(basic ${folder_source} ${folder_header}  ${basic_MOC_SRCS} ${basic_RCCS} ${basic_UIS_H} ${folder_shader})
target_link_libraries(basic ${QT_LIBRARIES} ${QT_QTGUI_LIBRARIES} ${QT_QTOPENGL_LIBRARIES} ${OPENGL_LIBRARIES} "-framework Foundation" "-framework Cocoa" m ${ASSIMP_LIB} vortexengine coreprofileselector)
else()
add_executable(basic ${folder_source} ${folder_header}  ${basic_MOC_SRCS} ${basic_RCCS} ${basic_UIS_H} ${folder_shader})
target_link_libraries(basic ${QT_LIBRARIES} ${QT_QTGUI_LIBRARIES} ${QT_QTOPENGL_LIBRARIES} ${OPENGL_LIBRARIES}  ${ASSIMP_LIB} vortexengine)
endif()

