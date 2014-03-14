# Install script for directory: /media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "libassimp3.0-r126")
  FOREACH(file
      "$ENV{DESTDIR}/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/lib/libassimp.so.3.0.126"
      "$ENV{DESTDIR}/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/lib/libassimp.so.3"
      "$ENV{DESTDIR}/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/lib/libassimp.so"
      )
    IF(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      FILE(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    ENDIF()
  ENDFOREACH()
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/lib/libassimp.so.3.0.126;/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/lib/libassimp.so.3;/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/lib/libassimp.so")
FILE(INSTALL DESTINATION "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/lib" TYPE SHARED_LIBRARY FILES
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/lib/libassimp.so.3.0.126"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/lib/libassimp.so.3"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/lib/libassimp.so"
    )
  FOREACH(file
      "$ENV{DESTDIR}/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/lib/libassimp.so.3.0.126"
      "$ENV{DESTDIR}/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/lib/libassimp.so.3"
      "$ENV{DESTDIR}/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/lib/libassimp.so"
      )
    IF(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      FILE(RPATH_REMOVE
           FILE "${file}")
      IF(CMAKE_INSTALL_DO_STRIP)
        EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "${file}")
      ENDIF(CMAKE_INSTALL_DO_STRIP)
    ENDIF()
  ENDFOREACH()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "libassimp3.0-r126")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "libassimp3.0-r126-dev")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/assimp" TYPE FILE FILES
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/anim.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/ai_assert.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/camera.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/color4.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/color4.inl"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/config.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/defs.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/cfileio.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/light.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/material.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/material.inl"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/matrix3x3.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/matrix3x3.inl"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/matrix4x4.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/matrix4x4.inl"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/mesh.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/postprocess.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/quaternion.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/quaternion.inl"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/scene.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/texture.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/types.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/vector2.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/vector2.inl"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/vector3.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/vector3.inl"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/version.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/cimport.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/importerdesc.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/Importer.hpp"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/DefaultLogger.hpp"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/ProgressHandler.hpp"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/IOStream.hpp"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/IOSystem.hpp"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/Logger.hpp"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/LogStream.hpp"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/NullLogger.hpp"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/cexport.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/Exporter.hpp"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "libassimp3.0-r126-dev")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "libassimp3.0-r126-dev")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/assimp/Compiler" TYPE FILE FILES
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/Compiler/pushpack1.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/../include/assimp/Compiler/poppack1.h"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/pstdint.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "libassimp3.0-r126-dev")

