# Install script for directory: /media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp

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

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "libassimp3.0-r126-dev")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/lib/pkgconfig/assimp.pc")
FILE(INSTALL DESTINATION "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/lib/pkgconfig" TYPE FILE FILES "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/assimp.pc")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "libassimp3.0-r126-dev")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "libassimp3.0-r126-dev")
  list(APPEND CPACK_ABSOLUTE_DESTINATION_FILES
   "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/lib/cmake/assimp-3.0/assimp-config.cmake;/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/lib/cmake/assimp-3.0/assimp-config-version.cmake")
FILE(INSTALL DESTINATION "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/lib/cmake/assimp-3.0" TYPE FILE FILES
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/assimp-config.cmake"
    "/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/assimp-config-version.cmake"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "libassimp3.0-r126-dev")

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("/media/moon/Work/Svn/irit/Courses/M2IM/TP/trunk/external/assimp/code/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)

