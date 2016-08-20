# CMake generated Testfile for 
# Source directory: C:/D/PBE/PlusLib/src/PlusDataCollection/Testing
# Build directory: C:/D/PBE/PlusLib-bin/src/PlusDataCollection/Testing
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(TimestampFilteringTest "C:/D/PBE/bin/TimestampFilteringTest" "--source-seq-file=C:/D/PBE/PlusLibData/TestImages/TimestampFilteringTest.mha" "--averaged-items-for-filtering=20" "--max-timestamp-difference=0.08" "--min-stdev-reduction-factor=3.0" "--transform=IdentityToIdentityTransform")
set_tests_properties(TimestampFilteringTest PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
add_test(vtkDataCollectorTest1 "C:/D/PBE/bin/vtkDataCollectorTest1" "--config-file=C:/D/PBE/PlusLibData/ConfigFiles/Testing/PlusDeviceSet_DataCollectionOnly_SavedDataset.xml" "--video-buffer-seq-file=C:/D/PBE/PlusLibData/TestImages/WaterTankBottomTranslationVideoBuffer.mha" "--tracker-buffer-seq-file=C:/D/PBE/PlusLibData/TestImages/WaterTankBottomTranslationTrackerBuffer-trimmed.mha" "--rendering-off")
set_tests_properties(vtkDataCollectorTest1 PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
add_test(vtkDataCollectorTest2 "C:/D/PBE/bin/vtkDataCollectorTest2" "--video-buffer-seq-file=C:/D/PBE/PlusLibData/TestImages/WaterTankBottomTranslationVideoBuffer.mha" "--tracker-buffer-seq-file=C:/D/PBE/PlusLibData/TestImages/WaterTankBottomTranslationTrackerBuffer-trimmed.mha" "--config-file=C:/D/PBE/PlusLibData/ConfigFiles/Testing/PlusDeviceSet_DataCollectionOnly_SavedDataset.xml" "--acq-time-length=5" "--verbose=3")
set_tests_properties(vtkDataCollectorTest2 PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
add_test(vtk3DDataCollectorTest1 "C:/D/PBE/bin/vtk3DDataCollectorTest1" "--config-file=C:/D/PBE/PlusLibData/ConfigFiles/Testing/PlusDeviceSet_DataCollectionOnly_3DSavedDataset.xml" "--minimum=0" "--maximum=253" "--mean=8.08658" "--median=0" "--standard-deviation=21.6785" "--xDimension=112" "--yDimension=112" "--zDimension=48" "--verbose=3")
set_tests_properties(vtk3DDataCollectorTest1 PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
add_test(ReplayRecordedDataTest "C:/D/PBE/bin/ReplayRecordedDataTest" "--config-file=C:/D/PBE/PlusLibData/ConfigFiles/Testing/PlusDeviceSet_OpenIGTLinkTestServer.xml")
set_tests_properties(ReplayRecordedDataTest PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR")
add_test(vtkDataCollectorFileTest "C:/D/PBE/bin/vtkDataCollectorFileTest" "--config-file=C:/D/PBE/PlusLibData/ConfigFiles/Testing/PlusDeviceSet_DataCollectionOnly_File.xml")
set_tests_properties(vtkDataCollectorFileTest PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
add_test(PlusVersion "C:/D/PBE/bin/PlusVersion")
add_test(vtkMetaImageSequenceIOTest "C:/D/PBE/bin/vtkMetaImageSequenceIOTest" "--img-seq-file=C:/D/PBE/PlusLibData/TestImages/MetaImageSequenceIOTest1.mhd" "--output-img-seq-file=MetaImageSequenceIOTest1Output.mha")
set_tests_properties(vtkMetaImageSequenceIOTest PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
add_test(ViewSequenceFileTest "C:/D/PBE/bin/ViewSequenceFile" "--source-seq-file=C:/D/PBE/PlusLibData/TestImages/SpinePhantomFreehand.mha" "--config-file=C:/D/PBE/PlusLibData/ConfigFiles/Testing/PlusDeviceSet_SpinePhantomFreehandReconstructionOnly.xml" "--image-to-reference-transform=ImageToTracker" "--rendering-off")
set_tests_properties(ViewSequenceFileTest PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
add_test(itkFcsvReaderTest1 "C:/D/PBE/bin/itkFcsvReaderTest1" "C:/D/PBE/PlusLibData/TestImages/FcsvReaderTest1.fcsv")
set_tests_properties(itkFcsvReaderTest1 PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
add_test(itkFcsvWriterTest1 "C:/D/PBE/bin/itkFcsvWriterTest1" "C:/D/PBE/PlusLibData/TestImages/FcsvReaderTest1.fcsv" "C:/D/PBE/PlusLib-bin/src/PlusDataCollection/Testing/FcsvWriterTest1.fcsv")
set_tests_properties(itkFcsvWriterTest1 PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
add_test(vtkPlusDeviceFactoryTest "C:/D/PBE/bin/vtkPlusDeviceFactoryTest")
add_test(TransformInterpolationTest "C:/D/PBE/bin/TransformInterpolationTest" "--source-seq-file=C:/D/PBE/PlusLibData/TestImages/TransformInterpolationTest.mha" "--transform=ProbeToTracker" "--max-rotation-difference=1.0" "--max-translation-difference=0.5")
