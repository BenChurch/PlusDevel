#
# This module is provided as PLUSLIB_USE_FILE
# It can be INCLUDEd in a project to load the needed compiler and linker
# settings to use Plus library.
#

IF(NOT PLUSLIB_USE_FILE_INCLUDED)
  SET(PLUSLIB_USE_FILE_INCLUDED 1)

  CMAKE_POLICY(PUSH)
  CMAKE_POLICY(SET CMP0012 NEW) # if() recognizes numbers and boolean constants

  ## Set Plus binary dir path
  SET(PLUSLIB_DIR "C:/D/PBE/PlusLib-bin" )

  ## Set Plus source dir path
  SET(PLUSLIB_SOURCE_DIR "C:/D/PBE/PlusLib" )

  ## Set Plus data dir path
  SET(PLUSLIB_DATA_DIR "C:/D/PBE/PlusLibData" )

  ## Set Plus executable output path
  SET(PLUS_EXECUTABLE_OUTPUT_PATH "C:/D/PBE/bin" )

  ## Set Plus scripts folder path
  SET(PLUSLIB_SCRIPTS_DIR "C:/D/PBE/PlusLib/src/scripts" )

  ## Set Sikuli binary dir path
  SET(SIKULI_BIN_DIR "C:/D/PBE/PlusLib/tools/Sikuli" )

  ## Find ITK
  FIND_PACKAGE(ITK PATHS "C:/D/PBE/Deps/itk-bin" NO_DEFAULT_PATH)
  IF (ITK_FOUND)
    INCLUDE("C:/D/PBE/Deps/itk/CMake/UseITK.cmake")
  ELSE (ITK_FOUND)
    MESSAGE(FATAL_ERROR "Please set ITK.")
  ENDIF (ITK_FOUND)

  ## Find VTK
  FIND_PACKAGE(VTK PATHS "C:/D/PBE/Deps/vtk-bin" NO_DEFAULT_PATH)
  IF (VTK_FOUND)
    INCLUDE("C:/D/PBE/Deps/vtk/CMake/UseVTK.cmake")
  ELSE (VTK_FOUND)
    MESSAGE(FATAL_ERROR "Please set VTK.")
  ENDIF (VTK_FOUND)

  SET(PLUS_USE_OpenIGTLink ON)
  IF(PLUS_USE_OpenIGTLink)
    SET (OpenIGTLink_DIR "C:/D/PBE/Deps/OpenIGTLink-bin")
    FIND_PACKAGE(OpenIGTLink PATHS "C:/D/PBE/Deps/OpenIGTLink-bin" NO_DEFAULT_PATH)
    IF (OpenIGTLink_USE_FILE)
      INCLUDE ("C:/D/PBE/Deps/OpenIGTLink-bin/UseOpenIGTLink.cmake")
    ELSE (OpenIGTLink_USE_FILE)
      MESSAGE( FATAL_ERROR "Please set OpenIGTLink.")
    ENDIF (OpenIGTLink_USE_FILE)
  ENDIF(PLUS_USE_OpenIGTLink)

  SET(PLUS_USE_tesseract OFF)
  IF(PLUS_USE_tesseract)
    SET(tesseract_DIR "")
    FIND_PACKAGE(tesseract NO_MODULE REQUIRED PATHS ${tesseract_DIR})
  ENDIF(PLUS_USE_tesseract)

  SET(PLUS_USE_SLICER OFF)
  IF(PLUS_USE_SLICER)
    SET(SLICER_BIN_DIRECTORY "")
  ENDIF(PLUS_USE_SLICER)

  # Add include directories needed to use PlusLib library
  # Use a temporary CMake variable to store the list of directory paths
  # (it is required because this way directory names
  # that do not contain space and those that do contain space
  # work equally well).
  SET(PLUSLIB_INCLUDE_DIRS_TEMP "C:/D/PBE/PlusLib/src;C:/D/PBE/PlusLib-bin/src;C:/D/PBE/PlusLib/src/Utilities/Ransac;C:/D/PBE/PlusLib/src/Utilities/xio;C:/D/PBE/PlusLib/src/PlusCommon;C:/D/PBE/PlusLib-bin/src/PlusCommon;C:/D/PBE/PlusLib/src/PlusCommon/IO;C:/D/PBE/PlusLib/src/PlusOpenIGTLink;C:/D/PBE/PlusLib-bin/src/PlusOpenIGTLink;C:/D/PBE/PlusLib/src/PlusImageProcessing;C:/D/PBE/PlusLib-bin/src/PlusImageProcessing;C:/D/PBE/PlusLib/src/PlusUsSimulator;C:/D/PBE/PlusLib-bin/src/PlusUsSimulator;C:/D/PBE/PlusLib/src/PlusVolumeReconstruction;C:/D/PBE/PlusLib-bin/src/PlusVolumeReconstruction;C:/D/PBE/PlusLib/src/PlusDataCollection/Haptics;C:/D/PBE/PlusLib-bin/src/PlusDataCollection/Haptics;C:/D/PBE/PlusLib/src/PlusDataCollection;C:/D/PBE/PlusLib-bin/src/PlusDataCollection;C:/D/PBE/PlusLib/src/PlusDataCollection/FakeTracking;C:/D/PBE/PlusLib/src/PlusDataCollection/ImageProcessor;C:/D/PBE/PlusLib/src/PlusDataCollection/SavedDataSource;C:/D/PBE/PlusLib/src/PlusDataCollection/UsSimulatorVideo;C:/D/PBE/PlusLib/src/PlusDataCollection/VirtualDevices;C:/D/PBE/PlusLib/src/PlusDataCollection/ChRobotics;C:/D/PBE/PlusLib/src/PlusDataCollection/MicrochipTracking;C:/D/PBE/PlusLib/src/PlusDataCollection/OpenIGTLink;C:/D/PBE/Deps/OpenIGTLink-bin;C:/D/PBE/Deps/OpenIGTLink/Source;C:/D/PBE/Deps/OpenIGTLink/Source/igtlutil;C:/D/PBE/PlusLib-bin/src/PlusCalibration;C:/D/PBE/PlusLib/src/PlusCalibration/PatternLocAlgo;C:/D/PBE/PlusLib/src/PlusCalibration/vtkBrachyStepperPhantomRegistrationAlgo;C:/D/PBE/PlusLib/src/PlusCalibration/vtkCenterOfRotationCalibAlgo;C:/D/PBE/PlusLib/src/PlusCalibration/vtkLineSegmentationAlgo;C:/D/PBE/PlusLib/src/PlusCalibration/vtkPhantomLandmarkRegistrationAlgo;C:/D/PBE/PlusLib/src/PlusCalibration/vtkPhantomLinearObjectRegistrationAlgo;C:/D/PBE/PlusLib/src/PlusCalibration/vtkPivotCalibrationAlgo;C:/D/PBE/PlusLib/src/PlusCalibration/vtkProbeCalibrationAlgo;C:/D/PBE/PlusLib/src/PlusCalibration/vtkSpacingCalibAlgo;C:/D/PBE/PlusLib/src/PlusCalibration/vtkTemporalCalibrationAlgo;C:/D/PBE/PlusLib/src/PlusServer;C:/D/PBE/PlusLib-bin/src/PlusServer;C:/D/PBE/PlusLib/src/PlusServer/Commands")
  INCLUDE_DIRECTORIES(${PLUSLIB_INCLUDE_DIRS_TEMP})

  # Add link directories needed to use Plus library
  # Use a temporary CMake variable to store the list of directory paths
  # (it is required because this way directory names
  # that do not contain space and those that do contain space
  # work equally well).
  SET(PLUSLIB_LIBRARY_DIRS_TEMP "C:/D/PBE/bin")
  LINK_DIRECTORIES(${PLUSLIB_LIBRARY_DIRS_TEMP})

  SET(PLUSLIB_BUILD_SHARED_LIBS ON)

  ## Find SVN
  SET (SVN_FOUND TRUE)
  IF ( SVN_FOUND )
    SET( Subversion_SVN_EXECUTABLE "C:/Program Files/TortoiseSVN/bin/svn.exe" CACHE INTERNAL "")
  ENDIF (SVN_FOUND)

  ## Set Plus version
  SET(PLUSLIB_VERSION "2.5.0")
  SET(PLUSLIB_VERSION_MAJOR 2)
  SET(PLUSLIB_VERSION_MINOR 5)
  SET(PLUSLIB_VERSION_PATCH 0)
  SET(PLUSLIB_PLATFORM Win64)
  SET(PLUSLIB_REVISION 4819)

  SET(PLUS_ULTRASONIX_SDK_MAJOR_VERSION )
  SET(PLUS_ULTRASONIX_SDK_MINOR_VERSION )
  SET(PLUS_ULTRASONIX_SDK_PATCH_VERSION )

  ## Set third party lib path variables
  SET(ICCAPTURING_BIN_DIR "")
  SET(NDIOAPI_BINARY_DIR "")
  SET(ULTRASONIX_SDK_BINARY_DIR "")
  SET(ATC_TRAKSTAR_BINARY_DIR "C:/D/PBE/PlusLib/tools/Ascension/trakSTAR_940041_RevE/bin")
  SET(USDIGITAL_SEI_BINARY_DIR "C:/D/PBE/PlusLib/tools/UsDigital/SEI_5.22/bin")

  ## Set Plus use variables
  SET(PLUS_USE_3dConnexion_TRACKER OFF)
  SET(PLUS_USE_Ascension3DG OFF)
  SET(PLUS_USE_Ascension3DGm OFF)
  SET(PLUS_USE_BKPROFOCUS_CAMERALINK )
  SET(PLUS_USE_BKPROFOCUS_VIDEO OFF)
  SET(PLUS_TEST_BKPROFOCUS OFF)
  SET(PLUS_USE_BRACHY_TRACKER OFF)
  SET(PLUS_USE_CAPISTRANO_VIDEO OFF)  
  SET(PLUS_USE_EPIPHAN OFF)
  SET(PLUS_USE_ICCAPTURING_VIDEO OFF)
  SET(PLUS_USE_INTELREALSENSE OFF)
  SET(PLUS_USE_INTERSONSDKCXX_VIDEO OFF)
  SET(PLUS_USE_INTERSON_VIDEO OFF)
  SET(PLUS_USE_IntuitiveDaVinci OFF)
  SET(PLUS_USE_MICRONTRACKER OFF)
  SET(PLUS_USE_MMF_VIDEO OFF)
  SET(PLUS_USE_NDI OFF)
  SET(PLUS_USE_NDI_CERTUS OFF)
  SET(PLUS_USE_OPTIMET_CONOPROBE OFF)
  SET(PLUS_USE_OPTITRACK OFF)
  SET(PLUS_USE_PHIDGET_SPATIAL_TRACKER OFF)
  SET(PLUS_USE_PHILIPS_3D_ULTRASOUND OFF)
  SET(PLUS_USE_STEALTHLINK OFF)
  SET(PLUS_USE_TELEMED_VIDEO OFF)
  SET(PLUS_USE_THORLABS_VIDEO OFF)
  SET(PLUS_USE_ULTRASONIX_VIDEO OFF)
  SET(PLUS_USE_USDIGITALENCODERS_TRACKER OFF)
  SET(PLUS_USE_VFW_VIDEO OFF)
  SET(PLUS_USE_NVIDIA_DVP OFF)

  SET(PLUSLIB_DEPENDENCIES vtkxio;vtkPlusCommon;vtkPlusOpenIGTLink;vtkPlusImageProcessing;vtkPlusUsSimulator;vtkPlusVolumeReconstruction;vtkPlusHaptics;vtkPlusDataCollection;vtkPlusCalibration;vtkPlusServer )

  macro(INCLUDE_PLUSLIB_MS_PROJECTS)
    IF ( ${CMAKE_GENERATOR} MATCHES "Visual Studio 9" )
      INCLUDE_EXTERNAL_MSPROJECT(vtkPlusCommon;"C:/D/PBE/PlusLib-bin/src/PlusCommon/vtkPlusCommon.vcproj")
      INCLUDE_EXTERNAL_MSPROJECT(vtkPlusUsSimulator;"C:/D/PBE/PlusLib-bin/src/PlusUsSimulator/vtkPlusUsSimulator.vcproj";vtkPlusCommon)
      INCLUDE_EXTERNAL_MSPROJECT(vtkPlusImageProcessing;"C:/D/PBE/PlusLib-bin/src/PlusImageProcessing/vtkPlusImageProcessing.vcproj";vtkPlusCommon)
      INCLUDE_EXTERNAL_MSPROJECT(vtkPlusDataCollection;"C:/D/PBE/PlusLib-bin/src/PlusDataCollection/vtkPlusDataCollection.vcproj";vtkPlusCommon;vtkPlusUsSimulator;vtkPlusImageProcessing)
      INCLUDE_EXTERNAL_MSPROJECT(vtkPlusCalibration;"C:/D/PBE/PlusLib-bin/src/PlusCalibration/vtkPlusCalibration.vcproj";vtkPlusCommon)
      INCLUDE_EXTERNAL_MSPROJECT(vtkPlusVolumeReconstruction;"C:/D/PBE/PlusLib-bin/src/PlusVolumeReconstruction/vtkPlusVolumeReconstruction.vcproj";vtkPlusCommon;vtkPlusDataCollection)
      INCLUDE_EXTERNAL_MSPROJECT(vtkxio;"C:/D/PBE/PlusLib-bin/src/Utilities/xio/vtkxio.vcproj")
      IF (OFF)
        INCLUDE_EXTERNAL_MSPROJECT()
      ENDIF()
      IF (OFF)
        INCLUDE_EXTERNAL_MSPROJECT()
      ENDIF()
      IF (ON)
        INCLUDE_EXTERNAL_MSPROJECT(vtkPlusOpenIGTLink;"C:/D/PBE/PlusLib-bin/src/PlusOpenIGTLink/vtkPlusOpenIGTLink.vcproj";vtkPlusCommon)
      ENDIF()
    IF (OFF)
        INCLUDE_EXTERNAL_MSPROJECT()
      ENDIF()
    ENDIF ()
  endmacro()

  SET(DOXYGEN_EXECUTABLE "C:/Program Files/doxygen/bin/doxygen.exe")
  SET(DOXYGEN_DOT_EXECUTABLE "C:/Program Files (x86)/Graphviz2.38/bin/dot.exe")
  SET(DOXYGEN_HHC_EXECUTABLE "C:/D/PBE/PlusLib/tools/HtmlHelp/hhc.exe")
  SET(PLUSLIB_DOCUMENTATION_TARGET_IN_ALL )
  SET(PLUSLIB_BUILD_DOCUMENTATION ON)


  CMAKE_POLICY(POP)

ENDIF(NOT PLUSLIB_USE_FILE_INCLUDED)
