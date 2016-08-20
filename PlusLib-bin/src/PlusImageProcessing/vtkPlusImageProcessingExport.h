/*=Plus=header=begin======================================================
  Program: Plus
  Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
  See License.txt for details.
=========================================================Plus=header=end*/

// This file is automatically generated from PlusExport.h.in by GENERATE_EXPORT_DIRECTIVE_FILE macro.

// .NAME __vtkPlusImageProcessingExport - manage Windows system differences
// .SECTION Description
// The __vtkPlusImageProcessingExport manages DLL export syntax differences
// between different operating systems.

#ifndef __vtkPlusImageProcessingExport_h
#define __vtkPlusImageProcessingExport_h

#if defined(WIN32) && !defined(PlusLib_STATIC)
 #if defined(vtkPlusImageProcessing_EXPORTS)
  #define vtkPlusImageProcessingExport __declspec( dllexport )
 #else
  #define vtkPlusImageProcessingExport __declspec( dllimport )
 #endif
#else
 #define vtkPlusImageProcessingExport
#endif

#endif
