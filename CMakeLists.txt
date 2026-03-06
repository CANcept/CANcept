cmake_minimum_required(VERSION 3.16)

# 1. Project Configuration
project(CanBusManager
        VERSION 1.0.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 2. Options
option(ENABLE_TESTS "Build tests using GoogleTest and Benchmark" ON)
option(ENABLE_CLANG_TIDY "Enable static analysis with Clang-Tidy" OFF)
option(ENABLE_DOCS "Enable the documentation target" ON)
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)
option(DISABLE_TEST_LOGGING "Disable console logging during tests" ON)

# Compiler Flags
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(--coverage -O0 -g)
        add_link_options(--coverage)
    endif()
endif()

# 3. Static Analysis (Clang-Tidy)
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        message(STATUS "Clang-Tidy found: ${CLANG_TIDY_EXE}")
        set(DO_CLANG_TIDY "${CLANG_TIDY_EXE};-header-filter=${CMAKE_SOURCE_DIR}/src/.*")
    else()
        message(WARNING "Clang-Tidy enabled but not found.")
    endif()
endif()

# 4. Dependencies (Runtime Libraries)
find_package(Qt6 COMPONENTS Widgets PrintSupport OpenGL Svg Test REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(external/spdlog)
add_subdirectory(external/entt)
add_subdirectory(external/libsockcanpp)
add_subdirectory(external/qwt)

set(RAPIDCSV_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/rapidcsv/src")

# 5. Core Library
set(ENTRY_POINT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/app_root/entry_point/main.cpp")

set(RESOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc")

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "src/*.cpp")
list(FILTER SOURCE_FILES EXCLUDE REGEX "src/app_root/entry_point/main.cpp")

add_library(${PROJECT_NAME}Lib STATIC ${SOURCE_FILES} ${RESOURCE_FILE})
if(CLANG_TIDY_EXE)
    set_target_properties(${PROJECT_NAME}Lib PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
endif()

target_include_directories(${PROJECT_NAME}Lib PUBLIC
        src
        ${RAPIDCSV_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}Lib PUBLIC
        Qt6::Widgets
        Qt6::Svg
        EnTT::EnTT
        sockcanpp
        qwt
        Threads::Threads
        spdlog::spdlog
)

target_compile_definitions(${PROJECT_NAME}Lib
        PUBLIC
        LOGGING_LEVEL=$<IF:$<BOOL:${ENABLE_TESTS}>,$<IF:$<BOOL:${DISABLE_TEST_LOGGING}>,0,$<IF:$<CONFIG:Debug>,2,1>>,$<IF:$<CONFIG:Debug>,2,1>>
)

# 6. Executable
add_executable(${PROJECT_NAME} ${ENTRY_POINT_FILE} ${RESOURCE_FILE})
if(CLANG_TIDY_EXE)
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}Lib)

# 7. Documentation
if(ENABLE_DOCS)
    add_subdirectory(doc)
endif()

# 8. Testing
if(ENABLE_TESTS)
    enable_testing()

    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

    add_subdirectory(external/googletest)
    add_subdirectory(external/benchmark)

    function(add_bus_test TEST_NAME TEST_DIR LINK_LIB)
        file(GLOB_RECURSE TEST_SOURCES "${TEST_DIR}/*.cpp")
        if(TEST_SOURCES)
            add_executable(${TEST_NAME} ${TEST_SOURCES})
            target_link_libraries(${TEST_NAME} PRIVATE ${PROJECT_NAME}Lib ${LINK_LIB} GTest::gmock Qt6::Test)
            target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
            add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
        endif()
    endfunction()

    add_bus_test(UnitTests        "tests/unit"        GTest::gtest GTest::gmock Qt6::Test Qt6::Widgets)
    add_bus_test(IntegrationTests "tests/integration" GTest::gtest)
    add_bus_test(SystemTests      "tests/system"      GTest::gtest_main)
    add_bus_test(PerformanceTests "tests/performance" benchmark::benchmark_main)

    if(TARGET UnitTests AND TARGET IntegrationTests)
        set_tests_properties(IntegrationTests PROPERTIES DEPENDS UnitTests)
    endif()

    if(TARGET IntegrationTests AND TARGET SystemTests)
        set_tests_properties(SystemTests PROPERTIES DEPENDS IntegrationTests)
    endif()

    if(TARGET SystemTests AND TARGET PerformanceTests)
        set_tests_properties(PerformanceTests PROPERTIES DEPENDS SystemTests)
    endif()

    if(ENABLE_COVERAGE)
        find_program(LCOV_PATH lcov)
        find_program(GENHTML_PATH genhtml)

        if(LCOV_PATH AND GENHTML_PATH)
            add_custom_target(coverage_report
                    COMMAND find . -name '*.gcda' -delete 2>/dev/null || true
                    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                    COMMAND find . -path '*_autogen*' -name '*.gcda' -delete 2>/dev/null || true
                    COMMAND ${LCOV_PATH} --quiet --directory . --capture --output-file coverage_raw.info --ignore-errors gcov,mismatch,empty --rc branch_coverage=1 2>/dev/null
                    COMMAND ${LCOV_PATH} --quiet --extract coverage_raw.info '${CMAKE_SOURCE_DIR}/src/*' --output-file coverage_filtered.info --ignore-errors empty --rc branch_coverage=1
                    COMMAND ${LCOV_PATH} --quiet --remove coverage_filtered.info '*/view/*' '*/delegate/*' --output-file coverage.info --ignore-errors empty --rc branch_coverage=1
                    COMMAND ${GENHTML_PATH} --quiet --branch-coverage coverage.info --output-directory coverage_html --ignore-errors empty
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Processing coverage data into HTML..."
            )
        endif()
    endif()
endif()

#9. Format
find_program(CLANG_FORMAT_EXE NAMES "clang-format")
if(CLANG_FORMAT_EXE)
    file(GLOB_RECURSE ALL_SOURCE_FILES
            "src/*.cpp" "src/*.hpp"
            "tests/*.cpp" "tests/*.hpp"
    )

    add_custom_target(format
            COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_SOURCE_FILES}
            COMMENT "Formatting source code with clang-format..."
    )
else()
    message(WARNING "clang-format not found. 'format' target will not be available.")
endif()

include(GNUInstallDirs)
install(TARGETS CanBusManager
        DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES "assets/icon/app_root/can_bus.svg"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps"
        RENAME "CanBusManager.svg")
install(FILES "can_bus_manager.desktop"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")