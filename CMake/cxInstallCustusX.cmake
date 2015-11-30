# This file is part of CustusX, an Image Guided Therapy Application.
#
# Copyright (C) 2008- SINTEF Technology & Society, Medical Technology
#
# CustusX is fully owned by SINTEF Medical Technology (SMT). CustusX source
# code and binaries can only be used by SMT and those with explicit permission
# from SMT. CustusX shall not be distributed to anyone else.
#
# CustusX is a research tool. It is NOT intended for use or certified for use
# in a normal clinical setting. SMT does not take responsibility for its use
# in any way.
#
# See CustusX_License.txt for more information.

###############################################################################
#
# Installation script.
# Run this after all other actions. Requires that the bundle has been created.
#
###############################################################################


include(cxInstallUtilities)

if(CX_APPLE)
#    find_package(XCTest)
#    message(STATUS "xctest libs: " ${XCTest_LIBRARIES})
#    message(STATUS "CMAKE_SYSTEM_FRAMEWORK_PATH: " ${CMAKE_SYSTEM_FRAMEWORK_PATH})
    set(CX_MACOSX_DEVELOPER_FRAMEWORK_PATH "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/Library/Frameworks")
#    message(STATUS "CMAKE_SYSTEM_FRAMEWORK_PATH: " $ENV{DEVELOPER_FRAMEWORKS_DIR})
    
endif()

cx_install_apply_customizable_properties()
cx_install_configuration_files()

cx_install_windows_runtime_libs(${CX_INSTALL_ROOT_DIR}/bin)

#--------------------------------------------------------------------------------
# Bundle utilities
#--------------------------------------------------------------------------------
set(CX_MAIN_EXECUTABLE "${CX_INSTALL_BINARY_DIR}/${CX_BUNDLE_NAME}")
if(CX_WINDOWS)
	set( CX_MAIN_EXECUTABLE "${CX_MAIN_EXECUTABLE}.exe")
endif()

cx_install_add_library_dirs(
    ${CX_MACOSX_DEVELOPER_FRAMEWORK_PATH}
    ${XCTest_LIBRARIES}
    ${ULTERIUS_BIN_DIR}
    ${QT_LIBRARY_DIRS} #remove?
    ${QT_BINARY_DIR} #remove?
    ${GEStreamer_LIBRARY_DIRS}
    ${OpenCV_LIB_DIR}
    ${OpenCV_DIR}/bin
    ${OPENCL_LIBRARY_DIRS}
    ${IGSTK_DIR}/bin
    ${IGSTK_DIR}/lib
    ${OpenIGTLink_LIBRARY_DIRS}
    ${VTK_DIR}/lib
    ${VTK_DIR}/bin
    ${CTK_DIR}/CTK-build/bin
    ${CTK_DIR}/DCMTK-build/bin
    ${CTK_DCMTK_DIR}/lib
    ${CTK_DCMTK_DIR}/bin
    ${ITK_DIR}/lib
    ${ITK_DIR}/bin
    ${Level-Set-Segmentation_LIBRARY_DIRS}
    ${Tube-Segmentation-Framework_LIBRARY_DIRS}
    ${FAST_LIBRARY_DIRS}
    ${OpenCLUtilityLibrary_LIBRARY_DIRS}
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/plugins
)

#message(STATUS "QT_LIB: "   ${QT_LIBRARY_DIRS}) - empty
#message(STATUS "QT_BIN: "   ${QT_BINARY_DIR}) - empty

if(CX_WINDOWS)
    find_qt_bin_dir(QT_BIN_DIR)
    find_qt_plugin_dir(QT_PLUGINS_DIR)
    set(RUNTIME_ENVIRONMENT
        ${CX_ALL_LIBRARY_DIRS}
        ${QT_BIN_DIR}
        ${QT_PLUGINS_DIR}
    )

    file(TO_NATIVE_PATH "${RUNTIME_ENVIRONMENT}" RUNTIME_ENVIRONMENT)
    file(WRITE
        "${CustusX_BINARY_DIR}/bin/set_run_environment.bat"
        "@echo off
rem This file is automatically generated by the CustusXs CMake files.
echo Setting up a runtime environment for executables in the CustusX project.
set OLD_PATH=%PATH%
set PATH=%PATH%;${RUNTIME_ENVIRONMENT}
call %*
set PATH=%OLD_PATH%
        "
        )
endif(CX_WINDOWS)

cx_install_all_stored_targets(${CX_INSTALL_BINARY_DIR})

cx_fixup_and_add_qtplugins_to_bundle(
	"${CX_MAIN_EXECUTABLE}"
    "${CX_INSTALL_BINARY_DIR}"
    "${CX_ALL_LIBRARY_DIRS}"
)

include(CPack)

#hiding packages created by ctk: we install the plugins ourselves
cpack_add_component(Development HIDDEN DISABLED)
cpack_add_component(RuntimePlugins HIDDEN DISABLED)

