##############################################################################
# Copyright (c) 2019, European X-Ray Free-Electron Laser Facility GmbH
# All rights reserved.
#
# You should have received a copy of the 3-Clause BSD License along with this
# program. If not, see <https://opensource.org/licenses/BSD-3-Clause>
#
# Author: Jun Zhu, zhujun981661@gmail.com
##############################################################################

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(kbcpp-dmi VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC_ON)
# MOC files are generated in the binary dir
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (NOT TARGET karabo-bridge)
    find_package(karabo-bridge REQUIRED CONFIG)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set(thirdparty_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/thirdparty)
set(thirdparty_INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/thirdparty)

function(setup_external_project NAME)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${thirdparty_BINARY_DIR}/${NAME}-stage
    )
    if(result)
        message(FATAL_ERROR "CMake step for ${NAME} failed: ${result}")
    endif()

    execute_process(
        COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${thirdparty_BINARY_DIR}/${NAME}-stage
    )
    if(result)
        message(FATAL_ERROR "Build step for ${NAME} failed: ${result}")
    endif()
endfunction()

# qt
find_package(Qt5 COMPONENTS Core Gui Widgets Charts REQUIRED)
message(STATUS "Found Qt5: ${Qt5_VERSION}")

# opencv
set(opencv_REQUIRED_VERSION 4.1.2)
set(opencv_INSTALL_DIR ${thirdparty_INSTALL_DIR}/opencv)

find_package(OpenCV ${opencv_REQUIRED_VERSION} QUIET CONFIG HINTS ${opencv_INSTALL_DIR})
if(NOT OpenCV_FOUND)
    configure_file(cmake/downloadOpencv.cmake.in ${thirdparty_BINARY_DIR}/opencv-stage/CMakeLists.txt)
    setup_external_project( opencv )

    find_package(OpenCV ${opencv_REQUIRED_VERSION} REQUIRED CONFIG HINTS ${opencv_INSTALL_DIR})
endif()
message(STATUS "Found OpenCV: ${OpenCV_VERSION}, ${OpenCV_INCLUDE_DIRS}")

# tbb
set(tbb_REQUIRED_VERSION 2019_U9)

configure_file(cmake/downloadTbb.cmake.in ${thirdparty_BINARY_DIR}/tbb-stage/CMakeLists.txt)
setup_external_project( tbb )

include(${thirdparty_BINARY_DIR}/tbb-src/cmake/TBBBuild.cmake)
tbb_build(TBB_ROOT ${thirdparty_BINARY_DIR}/tbb-src
          CONFIG_DIR TBB_DIR
          MAKE_ARGS tbb_build_dir=${thirdparty_BINARY_DIR}/tbb-build
                    tbb_build_prefix=tbb_cmake_build)
find_package(TBB REQUIRED COMPONENTS tbb tbbmalloc)
message(STATUS "Found TBB: ${TBB_VERSION_MAJOR}.${TBB_VERSION_MINOR}")

get_target_property(TBB_INCLUDE_DIRS TBB::tbb INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(TBB_LIBRARIES TBB::tbb IMPORTED_LOCATION_RELEASE)

# rxcpp
set(rxcpp_REQUIRED_VERSION 4.1.0)
set(rxcpp_INDLUCE_DIR ${thirdparty_BINARY_DIR}/rxcpp-src/Rx/v2/src)

find_path(rxcpp_INCLUDE_DIRS NAMES rxcpp/rx.hpp HINTS ${rxcpp_INDLUCE_DIR})
if(${rxcpp_INCLUDE_DIRS} STREQUAL "rxcpp_INCLUDE_DIRS-NOTFOUND")
    configure_file(cmake/downloadRxcpp.cmake.in ${thirdparty_BINARY_DIR}/rxcpp-stage/CMakeLists.txt)
    setup_external_project( rxcpp )
    set(rxcpp_INCLUDE_DIRS ${rxcpp_INCLUDE_DIR} FORCE)
endif()

# dmi
add_subdirectory(src)

if (BUILD_TESTS)
    find_package(Qt5Test REQUIRED)
    add_subdirectory(tests)
endif()
