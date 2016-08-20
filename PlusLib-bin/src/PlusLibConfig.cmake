#-----------------------------------------------------------------------------
# Configuration file for the Public Library for Ultrasound (PLUS) toolkit
#
# © Copyright 2016 The Laboratory for Percutaneous Surgery, Queen's University, Canada
#
# This file can be passed to a CMake FIND_PACKAGE call with the following syntax:
#
# FIND_PACKAGE(PlusLib 2.5.0 <REQUIRED|QUIET> <NO_MODULE>)
#   If NO_MODULE is included, set the variable PlusLib_DIR:PATH=C:/D/PBE/PlusLib-bin
# 
# Once successful, you can either use the USE_FILE approach by the following CMake command:
# INCLUDE(${PLUS_USE_FILE})
#
# Or you can use the following variables to configure your CMake project:
#  PlusLib_INCLUDE_DIRS - include directories for Plus headers
#  PlusLib_LIBRARIES - list of CMake targets produced by this build of Plus
#  PlusLib_DATA_DIR - directory containing data collector configuration files, sample images, and 3d models
#-----------------------------------------------------------------------------

SET(PlusLib_TARGETS_FILE C:/D/PBE/PlusLib-bin/PlusLibTargets.cmake)
INCLUDE(${PlusLib_TARGETS_FILE})

# Tell the user project where to find our headers and libraries
SET(PlusLib_INCLUDE_DIRS "C:/D/PBE/PlusLib/src;C:/D/PBE/PlusLib-bin/src;C:/D/PBE/PlusLib/src/Utilities/Ransac;C:/D/PBE/PlusLib/src/Utilities/xio;C:/D/PBE/PlusLib/src/PlusCommon;C:/D/PBE/PlusLib-bin/src/PlusCommon;C:/D/PBE/PlusLib/src/PlusCommon/IO;C:/D/PBE/PlusLib/src/PlusOpenIGTLink;C:/D/PBE/PlusLib-bin/src/PlusOpenIGTLink;C:/D/PBE/PlusLib/src/PlusImageProcessing;C:/D/PBE/PlusLib-bin/src/PlusImageProcessing;C:/D/PBE/PlusLib/src/PlusUsSimulator;C:/D/PBE/PlusLib-bin/src/PlusUsSimulator;C:/D/PBE/PlusLib/src/PlusVolumeReconstruction;C:/D/PBE/PlusLib-bin/src/PlusVolumeReconstruction;C:/D/PBE/PlusLib/src/PlusDataCollection/Haptics;C:/D/PBE/PlusLib-bin/src/PlusDataCollection/Haptics;C:/D/PBE/PlusLib/src/PlusDataCollection;C:/D/PBE/PlusLib-bin/src/PlusDataCollection;C:/D/PBE/PlusLib/src/PlusDataCollection/FakeTracking;C:/D/PBE/PlusLib/src/PlusDataCollection/ImageProcessor;C:/D/PBE/PlusLib/src/PlusDataCollection/SavedDataSource;C:/D/PBE/PlusLib/src/PlusDataCollection/UsSimulatorVideo;C:/D/PBE/PlusLib/src/PlusDataCollection/VirtualDevices;C:/D/PBE/PlusLib/src/PlusDataCollection/ChRobotics;C:/D/PBE/PlusLib/src/PlusDataCollection/MicrochipTracking;C:/D/PBE/PlusLib/src/PlusDataCollection/OpenIGTLink;C:/D/PBE/Deps/OpenIGTLink-bin;C:/D/PBE/Deps/OpenIGTLink/Source;C:/D/PBE/Deps/OpenIGTLink/Source/igtlutil;C:/D/PBE/PlusLib-bin/src/PlusCalibration;C:/D/PBE/PlusLib/src/PlusCalibration/PatternLocAlgo;C:/D/PBE/PlusLib/src/PlusCalibration/vtkBrachyStepperPhantomRegistrationAlgo;C:/D/PBE/PlusLib/src/PlusCalibration/vtkCenterOfRotationCalibAlgo;C:/D/PBE/PlusLib/src/PlusCalibration/vtkLineSegmentationAlgo;C:/D/PBE/PlusLib/src/PlusCalibration/vtkPhantomLandmarkRegistrationAlgo;C:/D/PBE/PlusLib/src/PlusCalibration/vtkPhantomLinearObjectRegistrationAlgo;C:/D/PBE/PlusLib/src/PlusCalibration/vtkPivotCalibrationAlgo;C:/D/PBE/PlusLib/src/PlusCalibration/vtkProbeCalibrationAlgo;C:/D/PBE/PlusLib/src/PlusCalibration/vtkSpacingCalibAlgo;C:/D/PBE/PlusLib/src/PlusCalibration/vtkTemporalCalibrationAlgo;C:/D/PBE/PlusLib/src/PlusServer;C:/D/PBE/PlusLib-bin/src/PlusServer;C:/D/PBE/PlusLib/src/PlusServer/Commands")
SET(PlusLib_DATA_DIR "C:/D/PBE/PlusLibData")
SET(PlusLib_LIBRARIES "vtkxio;vtkPlusCommon;vtkPlusOpenIGTLink;vtkPlusImageProcessing;vtkPlusUsSimulator;vtkPlusVolumeReconstruction;vtkPlusHaptics;vtkPlusDataCollection;vtkPlusCalibration;vtkPlusServer")

# Tell the user project where to find Plus use file
SET(PlusLib_USE_FILE "C:/D/PBE/PlusLib-bin/UsePlusLib.cmake" )

# Get the location for IntersonCxx
SET(PLUS_USE_INTERSONSDKCXX_VIDEO "OFF")
IF(PLUS_USE_INTERSONSDKCXX_VIDEO)
  IF(NOT DEFINED IntersonSDKCxx_DIR)
    SET(IntersonSDKCxx_DIR )
  ENDIF()
  FIND_PACKAGE(IntersonSDKCxx REQUIRED)
ENDIF()

IF( OFF AND NOT TARGET Epiphan )
  ADD_LIBRARY(Epiphan SHARED IMPORTED)
  SET_PROPERTY(TARGET Epiphan PROPERTY IMPORTED_IMPLIB C:/D/PBE/PlusLib/tools/Epiphan/x64/frmgrab.lib)
  SET_PROPERTY(TARGET Epiphan PROPERTY IMPORTED_LOCATION C:/D/PBE/PlusLib/tools/Epiphan/x64/frmgrab.dll)
  LIST(APPEND PlusLib_LIBRARIES Epiphan)
ENDIF()

IF( OFF AND NOT TARGET Ascension3DG )
  ADD_LIBRARY(Ascension3DG SHARED IMPORTED)
  SET_PROPERTY(TARGET Ascension3DG PROPERTY IMPORTED_IMPLIB )
  SET_PROPERTY(TARGET Ascension3DG PROPERTY IMPORTED_LOCATION )
  LIST(APPEND PlusLib_LIBRARIES Ascension3DG)
ENDIF()

IF( OFF AND NOT TARGET Ascension3DGm )
  ADD_LIBRARY(Ascension3DGm SHARED IMPORTED)
  SET_PROPERTY(TARGET Ascension3DGm PROPERTY IMPORTED_IMPLIB C:/D/PBE/PlusLib/tools/Ascension/medSAFE_940033_Rev_F/lib/ATC3DGm.lib)
  SET_PROPERTY(TARGET Ascension3DGm PROPERTY IMPORTED_LOCATION )
  LIST(APPEND PlusLib_LIBRARIES Ascension3DGm)
ENDIF()

IF( OFF AND NOT TARGET phidget )
  ADD_LIBRARY(phidget SHARED IMPORTED)
  SET_PROPERTY(TARGET phidget PROPERTY IMPORTED_IMPLIB C:/D/PBE/PlusLib/tools/Phidget/PhidgetSpatial-2.1.8/x64/phidget21.lib)
  SET_PROPERTY(TARGET phidget PROPERTY IMPORTED_LOCATION C:/D/PBE/PlusLib/tools/Phidget/PhidgetSpatial-2.1.8/x64/phidget21.dll)
  LIST(APPEND PlusLib_LIBRARIES phidget)
ENDIF()

IF( OFF AND NOT TARGET USDigitalEncoders )
  ADD_LIBRARY(USDigitalEncoders SHARED IMPORTED)
  SET_PROPERTY(TARGET USDigitalEncoders PROPERTY IMPORTED_IMPLIB C:/D/PBE/PlusLib/tools/UsDigital/SEI_5.22/lib/SEIDrv32.lib)
  SET_PROPERTY(TARGET USDigitalEncoders PROPERTY IMPORTED_LOCATION C:/D/PBE/PlusLib/tools/UsDigital/SEI_5.22/bin/SEIDrv32.dll)
  LIST(APPEND PlusLib_LIBRARIES USDigitalEncoders)
ENDIF()

IF( WIN32 AND OFF AND NOT TARGET NVidiaDVP )
  ADD_LIBRARY(NVidiaDVP SHARED IMPORTED)
  SET_PROPERTY(TARGET NVidiaDVP PROPERTY IMPORTED_IMPLIB C:/D/PBE/PlusLib/tools/NVidia/dvp170/lib/x64/dvp.lib)
  SET_PROPERTY(TARGET NVidiaDVP PROPERTY IMPORTED_LOCATION C:/D/PBE/PlusLib/tools/NVidia/dvp170/bin/x64/dvp.dll)
  LIST(APPEND PlusLib_LIBRARIES NVidiaDVP)

  ADD_LIBRARY(QuadroSDI INTERFACE IMPORTED)
  SET_PROPERTY(TARGET QuadroSDI PROPERTY INTERFACE_INCLUDE_DIRECTORIES 
    ""
    )
  IF( (MSVC AND ${CMAKE_GENERATOR} MATCHES "Win64") OR NOT MSVC)
    SET_PROPERTY(TARGET QuadroSDI PROPERTY INTERFACE_LINK_LIBRARIES 
      "/nvapi64.lib"
      "/ANCapi.lib"
      "/NvCpl.lib"
      "/cutil64.lib"
      )
  ELSE()
    SET_PROPERTY(TARGET QuadroSDI PROPERTY INTERFACE_LINK_LIBRARIES 
      "/nvapi.lib"
      "/ANCapi.lib"
      "/NvCpl.lib"
      "/cutil32.lib"
      )
  ENDIF()
ENDIF()

IF(ON)
  FIND_PACKAGE(OpenIGTLink PATHS "C:/D/PBE/Deps/OpenIGTLink-bin" NO_MODULE REQUIRED)
ENDIF()
