project(vortexRenderer)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_BUILD_TYPE Debug)
# Pour une compilation gcc4.7 sous macosX (recommandé)
# -DCMAKE_CXX_COMPILER=/opt/local/bin/g++ -DCMAKE_C_COMPILER=/opt/local/bin/gcc
#

# compil flags, remove unused warnings (which we have a lot ;)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -ansi -fPIC -mfpmath=sse -finline -DNDEBUG -DQT_NO_DEBUG -DQT_NO_CHECK -mmmx -msse -msse2 -msse3 -g -Wno-unused")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -ansi -fPIC -mfpmath=sse -finline -DQT_NO_DEBUG -DQT_NO_CHECK -mmmx -msse -msse2 -msse3 -Wno-unused")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_DEBUG} -g3")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g3 ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g3 ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# output dirs
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib-debug)
  set(LIB_INSTALL_DIR ${CMAKE_SOURCE_DIR}/lib-debug)
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin-dbg)
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
  set(LIB_INSTALL_DIR ${CMAKE_SOURCE_DIR}/lib)
  set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
endif()

#external libs and tools
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

# GLM
set(GLM_INC ${EXTERNAL_DIR}/glm)

# Assimp
set(ASSIMP_LIB_DIR  ${CMAKE_SOURCE_DIR}/lib)
set(ASSIMP_INC ${EXTERNAL_DIR}/assimp/include)
set(ASSIMP_LIB -L${ASSIMP_LIB_DIR} assimp)

# tell cmake to process CMakeLists.txt in that subdirectory
add_subdirectory(src)
