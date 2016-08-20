/*=Plus=header=begin======================================================
  Program: Plus
  Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
  See License.txt for details.
=========================================================Plus=header=end*/

// This file is automatically generated from PlusExport.h.in by GENERATE_EXPORT_DIRECTIVE_FILE macro.

// .NAME __vtkPlusCalibrationExport - manage Windows system differences
// .SECTION Description
// The __vtkPlusCalibrationExport manages DLL export syntax differences
// between different operating systems.

#ifndef __vtkPlusCalibrationExport_h
#define __vtkPlusCalibrationExport_h

#if defined(WIN32) && !defined(PlusLib_STATIC)
 #if defined(vtkPlusCalibration_EXPORTS)
  #define vtkPlusCalibrationExport __declspec( dllexport )
 #else
  #define vtkPlusCalibrationExport __declspec( dllimport )
 #endif
#else
 #define vtkPlusCalibrationExport
#endif

#endif