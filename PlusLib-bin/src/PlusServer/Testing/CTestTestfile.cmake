# CMake generated Testfile for 
# Source directory: C:/D/PBE/PlusLib/src/PlusServer/Testing
# Build directory: C:/D/PBE/PlusLib-bin/src/PlusServer/Testing
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(PlusServer "C:/D/PBE/bin/PlusServer" "--config-file=C:/D/PBE/PlusLibData/ConfigFiles/Testing/PlusDeviceSet_OpenIGTLinkTestServer.xml" "--running-time=5" "--testing-config-file=C:/D/PBE/PlusLibData/ConfigFiles/Testing/PlusDeviceSet_OpenIGTLinkTestClient.xml")
set_tests_properties(PlusServer PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING")
add_test(PlusServerOpenIGTLinkCommandsTest "C:/D/PBE/bin/PlusServerRemoteControl" "--server-config-file=C:/D/PBE/PlusLibData/ConfigFiles/Testing/PlusDeviceSet_OpenIGTLinkCommandsTest.xml" "--run-tests")
set_tests_properties(PlusServerOpenIGTLinkCommandsTest PROPERTIES  FAIL_REGULAR_EXPRESSION "ERROR;WARNING" TIMEOUT "90")
