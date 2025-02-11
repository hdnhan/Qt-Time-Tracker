include(GoogleTest)
file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

set(TEST_NAME "tests")
add_executable(${TEST_NAME} ${TEST_SOURCES})
target_link_libraries(${TEST_NAME} PRIVATE gtest Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Test)
target_link_libraries(${TEST_NAME} PRIVATE TimeTrackerLib)
target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
gtest_discover_tests(${TEST_NAME})

if(ENABLE_COVERAGE)
    set(COVERAGE_MAIN "coverage")
    set(COVERAGE_EXCLUDES
        "${PROJECT_SOURCE_DIR}/app/*"
        "${PROJECT_SOURCE_DIR}/cmake/*"
        "${PROJECT_SOURCE_DIR}/docs/*"
        "${PROJECT_SOURCE_DIR}/external/*"
        "${PROJECT_SOURCE_DIR}/tests/*"
        "${PROJECT_SOURCE_DIR}/build/*"
        "/usr/include/*"
    )
    set(COVERAGE_EXTRA_FLAGS)
    set(COVERAGE_DEPENDENCIES ${TEST_NAME})

    setup_target_for_coverage_gcovr_html(
        NAME
        ${COVERAGE_MAIN}
        EXECUTABLE
        ctest
        DEPENDENCIES
        ${COVERAGE_DEPENDENCIES}
        BASE_DIRECTORY
        ${CMAKE_SOURCE_DIR}
    )
endif()