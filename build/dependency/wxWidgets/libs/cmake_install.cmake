# Install script for directory: /Users/liujiahao/Documents/CSCI 576/PAs/CSCI-576-starter-code-main/ImageDisplay_Cpp_cross_platform/dependency/wxWidgets/build/cmake/lib

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/liujiahao/Documents/CSCI 576/PAs/CSCI-576-starter-code-main/ImageDisplay_Cpp_cross_platform/build/dependency/wxWidgets/libs/base/cmake_install.cmake")
  include("/Users/liujiahao/Documents/CSCI 576/PAs/CSCI-576-starter-code-main/ImageDisplay_Cpp_cross_platform/build/dependency/wxWidgets/libs/net/cmake_install.cmake")
  include("/Users/liujiahao/Documents/CSCI 576/PAs/CSCI-576-starter-code-main/ImageDisplay_Cpp_cross_platform/build/dependency/wxWidgets/libs/core/cmake_install.cmake")
  include("/Users/liujiahao/Documents/CSCI 576/PAs/CSCI-576-starter-code-main/ImageDisplay_Cpp_cross_platform/build/dependency/wxWidgets/libs/adv/cmake_install.cmake")
  include("/Users/liujiahao/Documents/CSCI 576/PAs/CSCI-576-starter-code-main/ImageDisplay_Cpp_cross_platform/build/dependency/wxWidgets/libs/aui/cmake_install.cmake")
  include("/Users/liujiahao/Documents/CSCI 576/PAs/CSCI-576-starter-code-main/ImageDisplay_Cpp_cross_platform/build/dependency/wxWidgets/libs/html/cmake_install.cmake")
  include("/Users/liujiahao/Documents/CSCI 576/PAs/CSCI-576-starter-code-main/ImageDisplay_Cpp_cross_platform/build/dependency/wxWidgets/libs/propgrid/cmake_install.cmake")
  include("/Users/liujiahao/Documents/CSCI 576/PAs/CSCI-576-starter-code-main/ImageDisplay_Cpp_cross_platform/build/dependency/wxWidgets/libs/ribbon/cmake_install.cmake")
  include("/Users/liujiahao/Documents/CSCI 576/PAs/CSCI-576-starter-code-main/ImageDisplay_Cpp_cross_platform/build/dependency/wxWidgets/libs/richtext/cmake_install.cmake")
  include("/Users/liujiahao/Documents/CSCI 576/PAs/CSCI-576-starter-code-main/ImageDisplay_Cpp_cross_platform/build/dependency/wxWidgets/libs/webview/cmake_install.cmake")
  include("/Users/liujiahao/Documents/CSCI 576/PAs/CSCI-576-starter-code-main/ImageDisplay_Cpp_cross_platform/build/dependency/wxWidgets/libs/stc/cmake_install.cmake")
  include("/Users/liujiahao/Documents/CSCI 576/PAs/CSCI-576-starter-code-main/ImageDisplay_Cpp_cross_platform/build/dependency/wxWidgets/libs/xrc/cmake_install.cmake")
  include("/Users/liujiahao/Documents/CSCI 576/PAs/CSCI-576-starter-code-main/ImageDisplay_Cpp_cross_platform/build/dependency/wxWidgets/libs/media/cmake_install.cmake")
  include("/Users/liujiahao/Documents/CSCI 576/PAs/CSCI-576-starter-code-main/ImageDisplay_Cpp_cross_platform/build/dependency/wxWidgets/libs/gl/cmake_install.cmake")
  include("/Users/liujiahao/Documents/CSCI 576/PAs/CSCI-576-starter-code-main/ImageDisplay_Cpp_cross_platform/build/dependency/wxWidgets/libs/qa/cmake_install.cmake")
  include("/Users/liujiahao/Documents/CSCI 576/PAs/CSCI-576-starter-code-main/ImageDisplay_Cpp_cross_platform/build/dependency/wxWidgets/libs/xml/cmake_install.cmake")

endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/Users/liujiahao/Documents/CSCI 576/PAs/CSCI-576-starter-code-main/ImageDisplay_Cpp_cross_platform/build/dependency/wxWidgets/libs/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
