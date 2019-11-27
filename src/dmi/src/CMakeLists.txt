##############################################################################
# Copyright (c) 2019, European X-Ray Free-Electron Laser Facility GmbH
# All rights reserved.
#
# You should have received a copy of the 3-Clause BSD License along with this
# program. If not, see <https://opensource.org/licenses/BSD-3-Clause>
#
# Author: Jun Zhu, zhujun981661@gmail.com
##############################################################################

# Versioning
# ==========

configure_file(dmi_version.cpp.in dmi_version.cpp @ONLY)
add_library(dmi_version STATIC ${CMAKE_CURRENT_BINARY_DIR}/dmi_version.cpp)

# DMI
# ===

set(dmi_GUI_SOURCES gui/mainwindow.cpp
                    gui/datasource_widget.cpp
                    gui/datasource_model.cpp
                    gui/lineanalysis_widget.cpp
                    gui/imageanalysis_widget.cpp)

set(dmi_PIPELINE_SOURCES pipeline/databroker.cpp
                         pipeline/imageprocessor.cpp
                         pipeline/sourceitem.cpp)

# dmi_lib is also used in unittest
add_library(dmi_lib ${dmi_GUI_SOURCES} ${dmi_PIPELINE_SOURCES})

target_link_libraries(dmi_lib
    PRIVATE
        dmi_version
    PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Charts
        karabo-bridge
        ${OpenCV_LIBS}
        ${TBB_LIBRARIES})

target_include_directories(dmi_lib
    PUBLIC
        ${rxcpp_INCLUDE_DIRS}
        ${TBB_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/include)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # suppress qDebug() << ...
    target_compile_definitions(dmi_lib PRIVATE -DQT_NO_DEBUG_OUTPUT)
endif()

add_executable(dmi dmi_main.cpp)

target_link_libraries(dmi PRIVATE dmi_lib)
