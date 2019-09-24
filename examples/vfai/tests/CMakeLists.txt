cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

set(VFAI_TESTS test_treemodel.cpp)

add_executable(test_vfai_lib ${VFAI_TESTS})

target_link_libraries(test_vfai_lib
    PRIVATE
        vfai_lib
        Qt5::Test
        gmock
        gmock_main
        pthread)

target_include_directories(test_vfai_lib
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src)

add_custom_target(vtest COMMAND test_vfai_lib DEPENDS test_vfai_lib)
