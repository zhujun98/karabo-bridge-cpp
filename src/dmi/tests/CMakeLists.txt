set(DMI_TESTS test_treemodel.cpp)

add_executable(test_dmi_lib ${DMI_TESTS})

target_link_libraries(test_dmi_lib
    PRIVATE
        dmi_lib
        Qt5::Test
        gmock
        gmock_main
        pthread)

target_include_directories(test_dmi_lib
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src)

add_custom_target(dmi_test COMMAND test_dmi_lib DEPENDS test_dmi_lib)
