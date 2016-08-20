# Install script for directory: C:/D/PBE/PlusLib/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/PlusLib")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
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

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/D/PBE/PlusLib-bin/src/Utilities/cmake_install.cmake")
  include("C:/D/PBE/PlusLib-bin/src/PlusCommon/cmake_install.cmake")
  include("C:/D/PBE/PlusLib-bin/src/PlusOpenIGTLink/cmake_install.cmake")
  include("C:/D/PBE/PlusLib-bin/src/PlusImageProcessing/cmake_install.cmake")
  include("C:/D/PBE/PlusLib-bin/src/PlusUsSimulator/cmake_install.cmake")
  include("C:/D/PBE/PlusLib-bin/src/PlusVolumeReconstruction/cmake_install.cmake")
  include("C:/D/PBE/PlusLib-bin/src/PlusDataCollection/cmake_install.cmake")
  include("C:/D/PBE/PlusLib-bin/src/PlusCalibration/cmake_install.cmake")
  include("C:/D/PBE/PlusLib-bin/src/PlusServer/cmake_install.cmake")
  include("C:/D/PBE/PlusLib-bin/src/scripts/cmake_install.cmake")
  include("C:/D/PBE/PlusLib-bin/src/Documentation/cmake_install.cmake")

endif()

