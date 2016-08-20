# CMake generated Testfile for 
# Source directory: C:/D/PBE/PlusLib/src/PlusUsSimulator/Testing
# Build directory: C:/D/PBE/PlusLib-bin/src/PlusUsSimulator/Testing
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(vtkPlusUsSimulatorRunTestLinear "C:/D/PBE/bin/vtkPlusUsSimulatorTest" "--config-file=C:/D/PBE/PlusLibData/ConfigFiles/Testing/PlusDeviceSet_UsSimulatorAlgoTestLinear.xml" "--transforms-seq-file=C:/D/PBE/PlusLibData/TestImages/SpinePhantom2Freehand.mha" "--output-us-img-file=simulatorOutputLinear.mha" "--use-compression=false")
set_tests_properties(vtkPlusUsSimulatorRunTestLinear PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
add_test(vtkPlusUsSimulatorCompareToBaselineTestLinear "C:/Program Files/CMake/bin/cmake.exe" "-E" "compare_files" "C:/D/PBE/bin/Release/Output/simulatorOutputLinear.mha" "C:/D/PBE/PlusLibData/TestImages/UsSimulatorOutputSpinePhantom2LinearBaseline.mha")
set_tests_properties(vtkPlusUsSimulatorCompareToBaselineTestLinear PROPERTIES  DEPENDS "vtkPlusUsSimulatorRunTestLinear")
add_test(vtkPlusUsSimulatorRunTestCurvilinear "C:/D/PBE/bin/vtkPlusUsSimulatorTest" "--config-file=C:/D/PBE/PlusLibData/ConfigFiles/Testing/PlusDeviceSet_UsSimulatorAlgoTestCurvilinear.xml" "--transforms-seq-file=C:/D/PBE/PlusLibData/TestImages/SpinePhantom2Freehand.mha" "--output-us-img-file=simulatorOutputCurvilinear.mha" "--use-compression=false")
set_tests_properties(vtkPlusUsSimulatorRunTestCurvilinear PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
add_test(vtkPlusUsSimulatorCompareToBaselineTestCurvilinear "C:/Program Files/CMake/bin/cmake.exe" "-E" "compare_files" "C:/D/PBE/bin/Release/Output/simulatorOutputCurvilinear.mha" "C:/D/PBE/PlusLibData/TestImages/UsSimulatorOutputSpinePhantom2CurvilinearBaseline.mha")
set_tests_properties(vtkPlusUsSimulatorCompareToBaselineTestCurvilinear PROPERTIES  DEPENDS "vtkPlusUsSimulatorRunTestCurvilinear")
