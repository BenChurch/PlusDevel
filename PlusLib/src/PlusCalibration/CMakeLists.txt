# --------------------------------------------------------------------------
# Sources

SET (PlusCalibration_SRCS
  PatternLocAlgo/PlusFidSegmentation.cxx
  PatternLocAlgo/PlusFidLineFinder.cxx
  PatternLocAlgo/PlusFidLabeling.cxx
  PatternLocAlgo/PlusFidPatternRecognitionCommon.cxx
  PatternLocAlgo/PlusFidPatternRecognition.cxx
  PatternLocAlgo/PlusPatternLocResultFile.cxx
  vtkProbeCalibrationAlgo/vtkPlusProbeCalibrationAlgo.cxx
  vtkProbeCalibrationAlgo/vtkPlusProbeCalibrationOptimizerAlgo.cxx
  vtkPivotCalibrationAlgo/vtkPlusPivotCalibrationAlgo.cxx
  vtkPhantomLandmarkRegistrationAlgo/vtkPlusPhantomLandmarkRegistrationAlgo.cxx
  vtkPhantomLandmarkRegistrationAlgo/vtkPlusLandmarkDetectionAlgo.cxx
  vtkPhantomLandmarkRegistrationAlgo/vtkPlusReadTrackedSignals.cxx
  vtkPhantomLinearObjectRegistrationAlgo/vtkPlusPhantomLinearObjectRegistrationAlgo.cxx
  vtkSpacingCalibAlgo/vtkPlusSpacingCalibAlgo.cxx
  vtkCenterOfRotationCalibAlgo/vtkPlusCenterOfRotationCalibAlgo.cxx
  vtkBrachyStepperPhantomRegistrationAlgo/vtkPlusBrachyStepperPhantomRegistrationAlgo.cxx
  vtkTemporalCalibrationAlgo/vtkPlusTemporalCalibrationAlgo.cxx
  vtkTemporalCalibrationAlgo/vtkPlusPrincipalMotionDetectionAlgo.cxx
  vtkLineSegmentationAlgo/vtkPlusLineSegmentationAlgo.cxx
  vtkPhantomLinearObjectRegistrationAlgo/Line.cxx
  vtkPhantomLinearObjectRegistrationAlgo/LinearObject.cxx
  vtkPhantomLinearObjectRegistrationAlgo/LinearObjectBuffer.cxx
  vtkPhantomLinearObjectRegistrationAlgo/Point.cxx
  vtkPhantomLinearObjectRegistrationAlgo/Plane.cxx
  vtkPhantomLinearObjectRegistrationAlgo/Planes.cxx
  vtkPhantomLinearObjectRegistrationAlgo/PointObservation.cxx
  vtkPhantomLinearObjectRegistrationAlgo/PointObservationBuffer.cxx
  vtkPhantomLinearObjectRegistrationAlgo/Reference.cxx
  vtkPhantomLinearObjectRegistrationAlgo/References.cxx
  )

SET (PlusCalibration_HDRS)
IF (MSVC OR ${CMAKE_GENERATOR} MATCHES "Xcode")
  SET (PlusCalibration_HDRS
    PatternLocAlgo/PlusFidSegmentation.h
    PatternLocAlgo/PlusFidLineFinder.h
    PatternLocAlgo/PlusFidLabeling.h
    PatternLocAlgo/PlusFidPatternRecognitionCommon.h
    PatternLocAlgo/PlusFidPatternRecognition.h
    PatternLocAlgo/PlusPatternLocResultFile.h
    vtkProbeCalibrationAlgo/vtkPlusProbeCalibrationAlgo.h
    vtkProbeCalibrationAlgo/vtkPlusProbeCalibrationOptimizerAlgo.h
    vtkPivotCalibrationAlgo/vtkPlusPivotCalibrationAlgo.h
    vtkPhantomLandmarkRegistrationAlgo/vtkPlusPhantomLandmarkRegistrationAlgo.h
    vtkPhantomLandmarkRegistrationAlgo/vtkPlusLandmarkDetectionAlgo.h
    vtkPhantomLandmarkRegistrationAlgo/vtkPlusReadTrackedSignals.h
    vtkPhantomLinearObjectRegistrationAlgo/vtkPlusPhantomLinearObjectRegistrationAlgo.h
    vtkSpacingCalibAlgo/vtkPlusSpacingCalibAlgo.h
    vtkCenterOfRotationCalibAlgo/vtkPlusCenterOfRotationCalibAlgo.h
    vtkBrachyStepperPhantomRegistrationAlgo/vtkPlusBrachyStepperPhantomRegistrationAlgo.h
    vtkTemporalCalibrationAlgo/vtkPlusTemporalCalibrationAlgo.h
    vtkTemporalCalibrationAlgo/vtkPlusPrincipalMotionDetectionAlgo.h
    vtkLineSegmentationAlgo/vtkPlusLineSegmentationAlgo.h
    vtkPhantomLinearObjectRegistrationAlgo/Line.h
    vtkPhantomLinearObjectRegistrationAlgo/LinearObject.h
    vtkPhantomLinearObjectRegistrationAlgo/LinearObjectBuffer.h
    vtkPhantomLinearObjectRegistrationAlgo/Point.cxx
    vtkPhantomLinearObjectRegistrationAlgo/Plane.h
    vtkPhantomLinearObjectRegistrationAlgo/Planes.h
    vtkPhantomLinearObjectRegistrationAlgo/PointObservation.h
    vtkPhantomLinearObjectRegistrationAlgo/PointObservationBuffer.h
    vtkPhantomLinearObjectRegistrationAlgo/Reference.h
    vtkPhantomLinearObjectRegistrationAlgo/References.h
    )
ENDIF ()

SET (PlusCalibration_INCLUDE_DIRS
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/PatternLocAlgo
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkBrachyStepperPhantomRegistrationAlgo
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkCenterOfRotationCalibAlgo
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkLineSegmentationAlgo
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkPhantomLandmarkRegistrationAlgo
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkPhantomLinearObjectRegistrationAlgo
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkPivotCalibrationAlgo
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkProbeCalibrationAlgo
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSpacingCalibAlgo
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkTemporalCalibrationAlgo
  CACHE INTERNAL "" FORCE)

# --------------------------------------------------------------------------
# Build the library

SET (PlusCalibration_LIBS
  ${VTK_LIBRARIES}
  ${ITK_LIBRARIES}
  vtkPlusCommon
  )

GENERATE_EXPORT_DIRECTIVE_FILE(vtkPlusCalibration)
ADD_LIBRARY( vtkPlusCalibration ${PlusCalibration_SRCS} ${PlusCalibration_HDRS} )
target_include_directories(vtkPlusCalibration PUBLIC ${PlusCalibration_INCLUDE_DIRS} )
TARGET_LINK_LIBRARIES(vtkPlusCalibration ${PlusCalibration_LIBS})

# --------------------------------------------------------------------------
# Set external MS project
SET (PLUSLIB_DEPENDENCIES ${PLUSLIB_DEPENDENCIES} vtkPlusCalibration CACHE INTERNAL "" FORCE)
LIST(REMOVE_DUPLICATES PLUSLIB_DEPENDENCIES)
# Add this variable to UsePlusLib.cmake.in INCLUDE_PLUSLIB_MS_PROJECTS macro
SET(vcProj_vtkPlusCalibration vtkPlusCalibration;"${PlusLib_BINARY_DIR}/src/PlusCalibration/vtkPlusCalibration.vcproj";vtkPlusCommon CACHE INTERNAL "" FORCE)

IF(PLUSBUILD_BUILD_PlusLib_TOOLS)
  #--------------------------------------------------------------------------------------------
  ADD_EXECUTABLE( ConvertFcsvToXml
    Tools/ConvertFcsvToXml.cxx
    )

  # Link the executable to the algo library.
  TARGET_LINK_LIBRARIES( ConvertFcsvToXml
    vtkPlusCalibration
    ITKCommon
    ${VTK_LIBRARIES}
    vtkPlusDataCollection
    )

  #--------------------------------------------------------------------------------------------
  ADD_EXECUTABLE( ConvertXmlToFcsv
    Tools/ConvertXmlToFcsv.cxx
    )

  # Link the executable to the algo library.
  TARGET_LINK_LIBRARIES( ConvertXmlToFcsv
    vtkPlusCalibration
    ITKCommon
    ${VTK_LIBRARIES}
    vtkPlusDataCollection
    )

  #--------------------------------------------------------------------------------------------
  ADD_EXECUTABLE(TemporalCalibration Tools/TemporalCalibration.cxx)
  TARGET_LINK_LIBRARIES( TemporalCalibration vtkPlusCommon vtkPlusCalibration ${VTK_LIBRARIES} vtkPlusDataCollection)
  GENERATE_HELP_DOC(TemporalCalibration)

  #--------------------------------------------------------------------------------------------
  ADD_EXECUTABLE(ProbeCalibration Tools/ProbeCalibration.cxx)
  TARGET_LINK_LIBRARIES(ProbeCalibration itkvnl itkvnl_algo ${VTK_LIBRARIES} vtkPlusCalibration vtkPlusDataCollection )
  GENERATE_HELP_DOC(ProbeCalibration)

  INSTALL(
    TARGETS
      ConvertFcsvToXml
      ConvertXmlToFcsv
      TemporalCalibration
      ProbeCalibration
    DESTINATION bin
    COMPONENT RuntimeExecutables
    )
ENDIF()

# --------------------------------------------------------------------------
# Install
#

INSTALL(TARGETS vtkPlusCalibration
  RUNTIME DESTINATION bin CONFIGURATIONS Release COMPONENT RuntimeLibraries
  LIBRARY DESTINATION lib CONFIGURATIONS Release COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION lib CONFIGURATIONS Release COMPONENT Development
  )

# --------------------------------------------------------------------------
# Testing
#

IF (BUILD_TESTING)
  ADD_SUBDIRECTORY (Testing)
ENDIF (BUILD_TESTING)
