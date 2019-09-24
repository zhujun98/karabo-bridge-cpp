##############################################################################
# Copyright (c) 2019, European X-Ray Free-Electron Laser Facility GmbH
# All rights reserved.
#
# You should have received a copy of the 3-Clause BSD License along with this
# program. If not, see <https://opensource.org/licenses/BSD-3-Clause>
#
# Author: Jun Zhu, zhujun981661@gmail.com
##############################################################################

cmake_minimum_required(VERSION 3.1)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC_ON)
# MOC files are generated in the binary dir
set(CMAKE_INCLUDE_CURRENT_DIR ON)

project(vfai)

if (NOT TARGET karabo-bridge)
    find_package(karabo-bridge REQUIRED CONFIG)
endif()

# opencv
find_package(OpenCV 4.1.1 QUIET CONFIG
             HINTS ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/opencv_install)

# qt
find_package(Qt5 COMPONENTS Core Gui Widgets Charts QUIET)

# tbb
set(tbb_root_dir ${PROJECT_SOURCE_DIR}/thirdparty/tbb)
include(${tbb_root_dir}/cmake/TBBBuild.cmake)
tbb_build(TBB_ROOT ${PROJECT_SOURCE_DIR}/thirdparty/tbb
        CONFIG_DIR TBB_DIR
        MAKE_ARGS tbb_build_dir=${CMAKE_CURRENT_BINARY_DIR}/thirdparty/tbb
        tbb_build_prefix=tbb_cmake_build)
find_package(TBB COMPONENTS tbb tbbmalloc QUIET)

if (OpenCV_FOUND AND Qt5_FOUND AND Qt5Charts_FOUND AND TBB_FOUND)
    message(STATUS "Found OpenCV: ${OpenCV_VERSION}, ${OpenCV_INCLUDE_DIRS}")
    message(STATUS "Found Qt5: ${Qt5_VERSION}")
    message(STATUS "Found TBB: ${TBB_VERSION_MAJOR}.${TBB_VERSION_MINOR}")

    get_target_property(TBB_INCLUDE_DIRS TBB::tbb INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(TBB_LIBRARIES TBB::tbb IMPORTED_LOCATION_RELEASE)

    add_subdirectory(src)

    if (BUILD_TESTS)
        find_package(Qt5Test REQUIRED)
        add_subdirectory(tests)
    endif()

else()
    message(STATUS "Cannot find OpenCV >= 4.1.1 and/or Qt5 and/orr TBB!
            Stop building example project '${PROJECT_NAME}'")
endif()
