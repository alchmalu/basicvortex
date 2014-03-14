project(externallibs)
# set(CMAKE_BUILD_TYPE Debug)
cmake_minimum_required(VERSION 2.6)

# some general cmake def
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -ansi -fPIC")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_DEBUG}")

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -mfpmath=sse -finline")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g3 -mfpmath=sse -finline -DNDEBUG -DQT_NO_DEBUG -DQT_NO_CHECK -mmmx -msse -msse2 -msse3")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g3 ${CMAKE_CXX_FLAGS_RELEASE}")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/../bin)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../lib-debug)
set(LIB_INSTALL_DIR ${CMAKE_SOURCE_DIR}/lib-debug)
else()
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../lib)
set(LIB_INSTALL_DIR ${CMAKE_SOURCE_DIR}/lib)
endif()

#building dependencies
#SET( Boost_DETAILED_FAILURE_MSG ON )
#FIND_PACKAGE( Boost 1.35 )

#IF ( NOT Boost_FOUND )
#  MESSAGE( FATAL_ERROR
#    "Boost libraries (http://www.boost.org/) not found. "
#    "You can build a non-boost version of Assimp with slightly reduced "
#    "functionality by specifying -DENABLE_BOOST_WORKAROUND=ON."
#    )
# ENDIF ( NOT Boost_FOUND )
#INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIRS} )

FILE(GLOB folder_glm glm/*/*.hpp glm/*/*/*.hpp glm/*/*.inl glm/*/*/*.inl)
SOURCE_GROUP("glm" FILES ${folder_glm})
# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(assimp)
