# CMake generated Testfile for 
# Source directory: C:/D/PBE/PlusLib/src/PlusImageProcessing/Testing
# Build directory: C:/D/PBE/PlusLib-bin/src/PlusImageProcessing/Testing
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(vtkPlusRfToBrightnessConvertRunTest "C:/D/PBE/bin/RfProcessor" "--config-file=C:/D/PBE/PlusLibData/ConfigFiles/Testing/PlusDeviceSet_RfProcessingAlgoCurvilinearTest.xml" "--rf-file=C:/D/PBE/PlusLibData/TestImages/UltrasonixCurvilinearRfData.mha" "--output-img-file=outputUltrasonixCurvilinearBrightnessData.mha" "--use-compression=false" "--operation=BRIGHTNESS_CONVERT")
set_tests_properties(vtkPlusRfToBrightnessConvertRunTest PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
add_test(vtkPlusRfToBrightnessConvertCompareToBaselineTest "C:/Program Files/CMake/bin/cmake.exe" "-E" "compare_files" "C:/D/PBE/bin/Release/Output/outputUltrasonixCurvilinearBrightnessData_OutputChannel_ScanConvertOutput.mha" "C:/D/PBE/PlusLibData/TestImages/UltrasonixCurvilinearBrightnessData.mha")
set_tests_properties(vtkPlusRfToBrightnessConvertCompareToBaselineTest PROPERTIES  DEPENDS "vtkPlusRfToBrightnessConvertRunTest")
add_test(vtkPlusUsScanConvertCurvilinearRunTest "C:/D/PBE/bin/RfProcessor" "--config-file=C:/D/PBE/PlusLibData/ConfigFiles/Testing/PlusDeviceSet_RfProcessingAlgoCurvilinearTest.xml" "--rf-file=C:/D/PBE/PlusLibData/TestImages/UltrasonixCurvilinearRfData.mha" "--output-img-file=outputUltrasonixCurvilinearScanConvertedData.mha" "--use-compression=false" "--operation=BRIGHTNESS_SCAN_CONVERT")
set_tests_properties(vtkPlusUsScanConvertCurvilinearRunTest PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
add_test(vtkPlusUsScanConvertCurvilinearCompareToBaselineTest "C:/Program Files/CMake/bin/cmake.exe" "-E" "compare_files" "C:/D/PBE/bin/Release/Output/outputUltrasonixCurvilinearScanConvertedData_OutputChannel_ScanConvertOutput.mha" "C:/D/PBE/PlusLibData/TestImages/UltrasonixCurvilinearScanConvertedData.mha")
set_tests_properties(vtkPlusUsScanConvertCurvilinearCompareToBaselineTest PROPERTIES  DEPENDS "vtkPlusUsScanConvertCurvilinearRunTest")
add_test(vtkPlusUsScanConvertLinearRunTest "C:/D/PBE/bin/RfProcessor" "--config-file=C:/D/PBE/PlusLibData/ConfigFiles/Testing/PlusDeviceSet_RfProcessingAlgoLinearTest.xml" "--rf-file=C:/D/PBE/PlusLibData/TestImages/UltrasonixLinearRfData.mha" "--output-img-file=outputUltrasonixLinearScanConvertedData.mha" "--operation=BRIGHTNESS_SCAN_CONVERT" "--use-compression=false")
set_tests_properties(vtkPlusUsScanConvertLinearRunTest PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
add_test(vtkPlusUsScanConvertLinearCompareToBaselineTest "C:/Program Files/CMake/bin/cmake.exe" "-E" "compare_files" "C:/D/PBE/bin/Release/Output/outputUltrasonixLinearScanConvertedData_OutputChannel_ScanConvertOutput.mha" "C:/D/PBE/PlusLibData/TestImages/UltrasonixLinearScanConvertedData.mha")
set_tests_properties(vtkPlusUsScanConvertLinearCompareToBaselineTest PROPERTIES  DEPENDS "vtkPlusUsScanConvertLinearRunTest")
add_test(vtkPlusUsScanConvertBkCurvilinearRunTest "C:/D/PBE/bin/RfProcessor" "--config-file=C:/D/PBE/PlusLibData/ConfigFiles/PlusDeviceSet_fCal_BkProFocus_OpenIGTLinkTracker.xml" "--rf-file=C:/D/PBE/PlusLibData/TestImages/BkCurvilinearRfData.mhd" "--output-img-file=outputBkCurvilinearScanConvertedData.mha" "--operation=BRIGHTNESS_SCAN_CONVERT" "--use-compression=false")
set_tests_properties(vtkPlusUsScanConvertBkCurvilinearRunTest PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
add_test(vtkPlusUsScanConvertBkCurvilinearCompareToBaselineTest "C:/Program Files/CMake/bin/cmake.exe" "-E" "compare_files" "C:/D/PBE/bin/Release/Output/outputBkCurvilinearScanConvertedData_OutputChannel_VideoStream.mha" "C:/D/PBE/PlusLibData/TestImages/BkCurvilinearScanConvertedData.mha")
set_tests_properties(vtkPlusUsScanConvertBkCurvilinearCompareToBaselineTest PROPERTIES  DEPENDS "vtkPlusUsScanConvertBkCurvilinearRunTest")
add_test(DrawScanLinesCurvilinearRunTest "C:/D/PBE/bin/DrawScanLines" "--config-file=C:/D/PBE/PlusLibData/ConfigFiles/Testing/PlusDeviceSet_RfProcessingAlgoCurvilinearTest.xml" "--source-seq-file=C:/D/PBE/PlusLibData/TestImages/UltrasonixCurvilinearScanConvertedData.mha" "--output-seq-file=UltrasonixCurvilinearScanConvertedDataWithScanlines.mha")
set_tests_properties(DrawScanLinesCurvilinearRunTest PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
add_test(DrawScanLinesLinearRunTest "C:/D/PBE/bin/DrawScanLines" "--config-file=C:/D/PBE/PlusLibData/ConfigFiles/Testing/PlusDeviceSet_RfProcessingAlgoLinearTest.xml" "--source-seq-file=C:/D/PBE/PlusLibData/TestImages/UltrasonixLinearScanConvertedData.mha" "--output-seq-file=UltrasonixLinearScanConvertedDataWithScanlines.mha")
add_test(ExtractScanLinesCurvilinearRunTest "C:/D/PBE/bin/ExtractScanLines" "--config-file=C:/D/PBE/PlusLibData/ConfigFiles/Testing/SpineUltrasound-Lumbar-C5_config.xml" "--input-seq-file=C:/D/PBE/PlusLibData/TestImages/SpineUltrasound-Lumbar-C5.mha" "--output-seq-file=SpineUltrasound-Lumbar-C5_ScanLines.mha")
set_tests_properties(ExtractScanLinesCurvilinearRunTest PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
add_test(ExtractScanLinesLinearRunTest "C:/D/PBE/bin/ExtractScanLines" "--config-file=C:/D/PBE/PlusLibData/ConfigFiles/Testing/BoneUltrasound_L14_config.xml" "--input-seq-file=C:/D/PBE/PlusLibData/TestImages/BoneUltrasound_L14.mha" "--output-seq-file=BoneUltrasound_L14_ScanLines.mha")
set_tests_properties(ExtractScanLinesLinearRunTest PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
