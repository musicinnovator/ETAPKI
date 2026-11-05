cmake_minimum_required(VERSION 3.20)

project(ETAPKI VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ETAPKI_BUILD_TESTS "Build ETAPKI core tests" ON)

add_library(etapki_core
    src/core/analysis.cpp
    src/core/catalog.cpp
    src/core/generated_variant_data.cpp
    src/core/integrity.cpp
    src/core/midi.cpp
    src/core/preview.cpp
    src/core/revision.cpp
    src/core/transformation.cpp
    src/core/variant_registry.cpp
)

target_include_directories(etapki_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/core/include
)

if(MSVC)
    target_compile_options(etapki_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(etapki_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

add_executable(etapki_cli src/app/main.cpp)
target_link_libraries(etapki_cli PRIVATE etapki_core)

if(WIN32)
    add_executable(etapki_gui WIN32 src/app/gui_main.cpp)
    target_link_libraries(etapki_gui PRIVATE etapki_core comdlg32)
endif()

if(ETAPKI_BUILD_TESTS)
    enable_testing()
    add_executable(etapki_core_tests tests/core_tests.cpp)
    target_link_libraries(etapki_core_tests PRIVATE etapki_core)
    add_test(NAME etapki_core_tests COMMAND etapki_core_tests)
endif()
