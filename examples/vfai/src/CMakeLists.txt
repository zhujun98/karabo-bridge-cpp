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

set(VFAI_SOURCES vf_mainwindow.cpp
                 vf_databroker.cpp
                 vf_imageprocessor.cpp
                 vf_datasource_widget.cpp
                 vf_listmodel.cpp
                 vf_treemodel.cpp
                 vf_treeview.cpp
                 vf_delegate.cpp
                 vf_lineanalysis_widget.cpp
                 vf_imageanalysis_widget.cpp
                 vf_sourceitem.cpp)

# vfai_lib is also used in unittest
add_library(vfai_lib ${VFAI_SOURCES})

target_link_libraries(vfai_lib
    PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Charts
        karabo-bridge
        ${OpenCV_LIBS}
        ${TBB_LIBRARIES})

target_include_directories(vfai_lib
    PUBLIC
        ${TBB_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/include)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # suppress qDebug() << ...
    target_compile_definitions(vfai_lib PRIVATE -DQT_NO_DEBUG_OUTPUT)
endif()

add_executable(vfai vf_main.cpp)

target_link_libraries(vfai PRIVATE vfai_lib)
