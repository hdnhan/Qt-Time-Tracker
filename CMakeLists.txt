cmake_minimum_required(VERSION 3.16)

project(TimeTracker VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# CMAKE MODULES
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(CPM)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Test)
message(STATUS "INFO 5: ${Qt5Core_INCLUDE_DIRS}")
message(STATUS "INFO 6: ${Qt6Core_INCLUDE_DIRS}")

file(GLOB_RECURSE PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

if (APPLE)
    set(ICON ${CMAKE_CURRENT_SOURCE_DIR}/assets/timer.png)
    set(MACOSX_BUNDLE_ICON_FILE timer.png)
    set_source_files_properties(${ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TimeTracker
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${ICON}
    )
    # https://doc.qt.io/qt-6/qt-add-resources.html
    qt_add_resources(TimeTracker "Resources"
        PREFIX "/Resources"
        FILES assets/start.png
    )
    # Configure logging level: https://doc.qt.io/qt-6/debug.html
    target_compile_definitions(TimeTracker PRIVATE QT_NO_DEBUG_OUTPUT=0)

# Define target properties for Android with Qt 6 as:
#    set_property(TARGET TimeTracker APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(TimeTracker SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(TimeTracker
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(TimeTracker PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.TimeTracker)
endif()
set_target_properties(TimeTracker PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS TimeTracker
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TimeTracker)
endif()

# Testing
if(ENABLE_TESTING AND ENABLE_COVERAGE)
    include(CodeCoverage)
    append_coverage_compiler_flags()
endif()

if (ENABLE_TESTING)
    # Create a library for testing
    add_library(TimeTrackerLib STATIC ${PROJECT_SOURCES})
    target_link_libraries(TimeTrackerLib PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
endif()

if(ENABLE_TESTING)
    cpmaddpackage("gh:google/googletest#v1.15.2")
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()