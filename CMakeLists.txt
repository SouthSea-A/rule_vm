cmake_minimum_required(VERSION 3.20)

project(
    rule_vm
    VERSION 0.1.0
    DESCRIPTION "Header-only C++23 self-modifying and serializable rule VM"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(RULE_VM_BUILD_EXAMPLES "Build rule_vm examples" ON)
option(RULE_VM_BUILD_TESTS "Build rule_vm tests" ON)
option(RULE_VM_BUILD_PERF "Build optional performance probe" OFF)
option(RULE_VM_INSTALL "Generate install rules" ON)

include(GNUInstallDirs)

if(MSVC)
    add_compile_options(/EHsc /utf-8 /std:c++latest)
else()
    add_compile_options(-Wall -Wextra)
endif()

# ---------------------------------------------------------------------------
# Header-only kernel library
# ---------------------------------------------------------------------------
add_library(rule_vm INTERFACE)
add_library(rule_vm::rule_vm ALIAS rule_vm)

target_compile_features(rule_vm INTERFACE cxx_std_23)
target_include_directories(
    rule_vm
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# ---------------------------------------------------------------------------
# Example layer: rule_desg
# ---------------------------------------------------------------------------
add_library(rule_vm_example INTERFACE)
add_library(rule_vm::rule_vm_example ALIAS rule_vm_example)

target_link_libraries(rule_vm_example INTERFACE rule_vm)
target_include_directories(
    rule_vm_example
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/example>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# ---------------------------------------------------------------------------
# Examples
# ---------------------------------------------------------------------------
if(RULE_VM_BUILD_EXAMPLES)
    foreach(demo IN ITEMS basic manual self_modify codegen serialize copy error template)
        add_executable(demo_${demo} example/demo_${demo}.cpp)
        target_link_libraries(demo_${demo} PRIVATE rule_vm_example)
        target_include_directories(demo_${demo} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/example)
    endforeach()

    target_compile_definitions(
        demo_template
        PRIVATE RULE_VM_EXAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/example/programs")
endif()

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(RULE_VM_BUILD_TESTS)
    enable_testing()

    function(rule_vm_add_test target source)
        add_executable(${target} tests/${source})
        target_link_libraries(${target} PRIVATE rule_vm_example)
        target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
        add_test(NAME ${target} COMMAND ${target})
    endfunction()

    rule_vm_add_test(text_load_tests text_load_tests.cpp)
    rule_vm_add_test(text_block_tests text_block_tests.cpp)
    rule_vm_add_test(text_const_tests text_const_tests.cpp)
    rule_vm_add_test(text_error_tests text_error_tests.cpp)
    rule_vm_add_test(text_alloc_failure_tests text_alloc_failure_tests.cpp)
    rule_vm_add_test(text_selfmodify_examples_tests text_selfmodify_examples_tests.cpp)
    rule_vm_add_test(text_stress_tests text_stress_tests.cpp)
    rule_vm_add_test(serialize_tests serialize_tests.cpp)
    rule_vm_add_test(rule_design_tests rule_design_tests.cpp)
    rule_vm_add_test(rule_kit_public_tests rule_kit_public_tests.cpp)
    rule_vm_add_test(rule_kit_ref_tests rule_kit_ref_tests.cpp)

    rule_vm_add_test(manager_cursor_tests manager_cursor_tests.cpp)
    target_include_directories(manager_cursor_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel/manager)

    rule_vm_add_test(kernel_runtime_tests kernel_runtime_tests.cpp)
    target_include_directories(kernel_runtime_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel/rule)

    rule_vm_add_test(rollback_tests rollback_tests.cpp)
    target_include_directories(rollback_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel/rule)

    if(RULE_VM_BUILD_PERF)
        add_executable(perf_tests tests/perf_tests.cpp)
        target_link_libraries(perf_tests PRIVATE rule_vm_example)
        target_include_directories(perf_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
    endif()
endif()

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------
if(RULE_VM_INSTALL)
    include(CMakePackageConfigHelpers)

    install(TARGETS rule_vm rule_vm_example EXPORT rule_vmTargets)

    install(
        FILES LICENSE README.md
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/rule_vm)

    install(
        DIRECTORY src/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h")

    install(
        DIRECTORY example/rule_design
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h")

    install(
        FILES example/rule_design.h example/rule_design_text.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    install(
        EXPORT rule_vmTargets
        NAMESPACE rule_vm::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rule_vm)

    configure_package_config_file(
        cmake/rule_vmConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/rule_vmConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rule_vm)

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/rule_vmConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/rule_vmConfig.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/rule_vmConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rule_vm)
endif()
