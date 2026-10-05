# Copyright 2026 IBM Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

include_guard()

# ---------------------------------------------------------------------------
# Coverage instrumentation — supports two back-ends:
#
#   Clang  (-fprofile-instr-generate / -fcoverage-mapping)
#     Report targets: coverage, coverage-full
#     Tools required: llvm-cov, llvm-profdata
#
#   GCC    (--coverage / gcov)
#     Report targets: coverage, coverage-full
#     Tools required: lcov, genhtml
#
# Usage:
#   cmake -B build -DGWHISPER_ENABLE_COVERAGE=ON -DGWHISPER_BUILD_TESTS=ON
#   cmake --build build
#   cmake --build build --target coverage       # unit tests only
#   cmake --build build --target coverage-full  # unit + function tests merged
#
# Produces (both targets):
#   build/coverage/index.html  – HTML report
#   build/coverage/lcov.info   – lcov trace file (importable by most CI tools)
# ---------------------------------------------------------------------------

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # ---- Clang: LLVM source-based coverage ----
    find_program(LLVM_COV      NAMES llvm-cov
        HINTS
            /Library/Developer/CommandLineTools/usr/bin
            /usr/bin
            /usr/local/bin
    )
    find_program(LLVM_PROFDATA NAMES llvm-profdata
        HINTS
            /Library/Developer/CommandLineTools/usr/bin
            /usr/bin
            /usr/local/bin
    )

    if(NOT LLVM_COV OR NOT LLVM_PROFDATA)
        if(APPLE)
            message(FATAL_ERROR
                "GWHISPER_ENABLE_COVERAGE=ON requires llvm-cov and llvm-profdata. "
                "Install Xcode Command Line Tools: xcode-select --install")
        else()
            message(FATAL_ERROR
                "GWHISPER_ENABLE_COVERAGE=ON requires llvm-cov and llvm-profdata. "
                "Install the LLVM toolchain, e.g.: apt-get install llvm  (Debian/Ubuntu) "
                "or  dnf install llvm  (Fedora/RHEL).")
        endif()
    endif()

    message(STATUS "Coverage: backend      = Clang (LLVM source-based)")
    message(STATUS "Coverage: llvm-cov      = ${LLVM_COV}")
    message(STATUS "Coverage: llvm-profdata = ${LLVM_PROFDATA}")

    set(_COVERAGE_BACKEND "CLANG")

    add_library(coverage_config INTERFACE)
    target_compile_options(coverage_config INTERFACE
        -fprofile-instr-generate
        -fcoverage-mapping
    )
    target_link_options(coverage_config INTERFACE
        -fprofile-instr-generate
    )

else()
    # ---- GCC: gcov-based coverage ----
    find_program(LCOV    NAMES lcov)
    find_program(GENHTML NAMES genhtml)

    if(NOT LCOV OR NOT GENHTML)
        message(WARNING
            "GWHISPER_ENABLE_COVERAGE=ON with GCC requires lcov and genhtml for report "
            "generation (e.g.: dnf install lcov). Binaries will be instrumented but the "
            "'coverage' and 'coverage-full' report targets will not be available.")
    else()
        message(STATUS "Coverage: lcov    = ${LCOV}")
        message(STATUS "Coverage: genhtml = ${GENHTML}")
    endif()

    message(STATUS "Coverage: backend = GCC (gcov / --coverage)")

    set(_COVERAGE_BACKEND "GCC")

    add_library(coverage_config INTERFACE)
    target_compile_options(coverage_config INTERFACE --coverage)
    target_link_options(coverage_config INTERFACE --coverage)
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter)

# ---------------------------------------------------------------------------
# Helper to register a test executable for the coverage report.
# Call once per instrumented test binary AFTER add_test() has been called.
#
#   gwhisper_coverage_target(
#       TEST_BINARY  gwhisper_tests          # CMake target name
#       SOURCE_DIRS  ${CMAKE_SOURCE_DIR}/src  # dirs to include in report
#   )
# ---------------------------------------------------------------------------
function(gwhisper_coverage_target)
    cmake_parse_arguments(_COV "" "TEST_BINARY" "SOURCE_DIRS" ${ARGN})

    set(_outdir   ${CMAKE_BINARY_DIR}/coverage)
    set(_binary   $<TARGET_FILE:${_COV_TEST_BINARY}>)

    if(_COVERAGE_BACKEND STREQUAL "CLANG")
        set(_profraw  ${CMAKE_BINARY_DIR}/coverage.profraw)
        set(_profdata ${CMAKE_BINARY_DIR}/coverage.profdata)

        # Build a space-separated --sources string for embedding inside bash -c "..."
        set(_source_str "")
        foreach(_dir IN LISTS _COV_SOURCE_DIRS)
            string(APPEND _source_str " --sources ${_dir}")
        endforeach()

        if(NOT TARGET coverage)
            add_custom_target(coverage
                COMMENT "Running instrumented tests and generating coverage report..."
                DEPENDS ${_COV_TEST_BINARY}
                COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=${_profraw}
                        ${_binary}
                COMMAND ${LLVM_PROFDATA} merge -sparse ${_profraw} -o ${_profdata}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${_outdir}
                COMMAND bash -c "${LLVM_COV} show ${_binary} --instr-profile=${_profdata} --format=html --output-dir=${_outdir}${_source_str} 2> >(grep -v 'mismatched data' >&2)"
                COMMAND bash -c "${LLVM_COV} export ${_binary} --instr-profile=${_profdata} --format=lcov${_source_str} > ${_outdir}/lcov.info 2> >(grep -v 'mismatched data' >&2)"
                COMMAND bash -c "${LLVM_COV} report ${_binary} --instr-profile=${_profdata}${_source_str} 2> >(grep -v 'mismatched data' >&2)"
                VERBATIM
            )
        endif()

    else()
        # GCC / gcov path
        if(NOT LCOV OR NOT GENHTML)
            message(STATUS "Coverage: skipping 'coverage' target (lcov/genhtml not found)")
            return()
        endif()

        # Build an --include pattern list for lcov
        set(_lcov_include "")
        foreach(_dir IN LISTS _COV_SOURCE_DIRS)
            list(APPEND _lcov_include "--include" "${_dir}/*")
        endforeach()

        if(NOT TARGET coverage)
            add_custom_target(coverage
                COMMENT "Running instrumented tests and generating gcov coverage report..."
                DEPENDS ${_COV_TEST_BINARY}
                COMMAND ${_binary}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${_outdir}
                COMMAND ${LCOV} --capture
                        --directory ${CMAKE_BINARY_DIR}
                        --output-file ${_outdir}/lcov.info
                        ${_lcov_include}
                COMMAND ${GENHTML} ${_outdir}/lcov.info
                        --output-directory ${_outdir}
                VERBATIM
            )
        endif()
    endif()
endfunction()

# ---------------------------------------------------------------------------
# gwhisper_coverage_full_target(
#       UNIT_TEST_BINARY  gwhisper_tests
#       GWHISPER_BINARY   gwhisper
#       SOURCE_DIRS       ${CMAKE_SOURCE_DIR}/src
# )
#
# Registers a "coverage-full" target that:
#   1. Runs the instrumented unit-test binary
#   2. Runs all CTest function tests so every gwhisper subprocess writes
#      its own profile file
#   3. Merges all raw profiles and produces the combined HTML + lcov report
# ---------------------------------------------------------------------------
function(gwhisper_coverage_full_target)
    cmake_parse_arguments(_COV "" "UNIT_TEST_BINARY;GWHISPER_BINARY;TESTSERVER_BINARY" "SOURCE_DIRS" ${ARGN})

    set(_outdir          ${CMAKE_BINARY_DIR}/coverage-full)
    set(_unit_binary     $<TARGET_FILE:${_COV_UNIT_TEST_BINARY}>)
    set(_gwhisper_bin    $<TARGET_FILE:${_COV_GWHISPER_BINARY}>)
    set(_testserver_bin  $<TARGET_FILE:${_COV_TESTSERVER_BINARY}>)
    set(_func_test_dir   ${CMAKE_SOURCE_DIR}/tests/functionTests)
    set(_resources_dir   ${_func_test_dir}/resources/)
    set(_certs_dir       ${CMAKE_BINARY_DIR}/tests/testServer/cert-key-pair/)

    if(_COVERAGE_BACKEND STREQUAL "CLANG")
        set(_profraw_unit         ${CMAKE_BINARY_DIR}/coverage-unit.profraw)
        set(_profraw_func_pattern ${CMAKE_BINARY_DIR}/coverage-func-%p.profraw)
        set(_profdata             ${CMAKE_BINARY_DIR}/coverage-full.profdata)

        set(_source_str "")
        foreach(_dir IN LISTS _COV_SOURCE_DIRS)
            string(APPEND _source_str " --sources ${_dir}")
        endforeach()

        if(NOT TARGET coverage-full)
            add_custom_target(coverage-full
                COMMENT "Running unit + function tests and generating merged coverage report..."
                DEPENDS ${_COV_UNIT_TEST_BINARY} ${_COV_GWHISPER_BINARY} ${_COV_TESTSERVER_BINARY}

                COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=${_profraw_unit}
                        ${_unit_binary}

                COMMAND ${CMAKE_COMMAND} -E env
                        LLVM_PROFILE_FILE=${_profraw_func_pattern}
                        ${Python3_EXECUTABLE} ${_func_test_dir}/runFunctionTest.py
                            ${_gwhisper_bin} ${_testserver_bin} ${_resources_dir} ${_certs_dir}
                            ${_func_test_dir}/completionTests.txt

                COMMAND ${CMAKE_COMMAND} -E env
                        LLVM_PROFILE_FILE=${_profraw_func_pattern}
                        ${Python3_EXECUTABLE} ${_func_test_dir}/runFunctionTest.py
                            ${_gwhisper_bin} ${_testserver_bin} ${_resources_dir} ${_certs_dir}
                            ${_func_test_dir}/rpcExecutionTests.txt

                COMMAND ${CMAKE_COMMAND} -E env
                        LLVM_PROFILE_FILE=${_profraw_func_pattern}
                        ${Python3_EXECUTABLE} ${_func_test_dir}/runFunctionTest.py
                            ${_gwhisper_bin} ${_testserver_bin} ${_resources_dir} ${_certs_dir}
                            ${_func_test_dir}/rpcTimeoutFunctionTests.txt

                COMMAND ${CMAKE_COMMAND} -E env
                        LLVM_PROFILE_FILE=${_profraw_func_pattern}
                        ${Python3_EXECUTABLE} ${_func_test_dir}/runFunctionTest.py
                            ${_gwhisper_bin} ${_testserver_bin} ${_resources_dir} ${_certs_dir}
                            ${_func_test_dir}/cacheFunctionTests.txt

                COMMAND ${CMAKE_COMMAND} -E env
                        LLVM_PROFILE_FILE=${_profraw_func_pattern}
                        ${Python3_EXECUTABLE} ${_func_test_dir}/runFunctionTest.py
                            ${_gwhisper_bin} ${_testserver_bin} ${_resources_dir} ${_certs_dir}
                            ${_func_test_dir}/sslFunctionTests.txt

                COMMAND bash -c "${LLVM_PROFDATA} merge -sparse ${_profraw_unit} ${CMAKE_BINARY_DIR}/coverage-func-*.profraw -o ${_profdata}"

                COMMAND ${CMAKE_COMMAND} -E make_directory ${_outdir}
                COMMAND bash -c "${LLVM_COV} show ${_unit_binary} --instr-profile=${_profdata} --format=html --output-dir=${_outdir}${_source_str} 2> >(grep -v 'mismatched data' >&2)"
                COMMAND bash -c "${LLVM_COV} export ${_unit_binary} --instr-profile=${_profdata} --format=lcov${_source_str} > ${_outdir}/lcov.info 2> >(grep -v 'mismatched data' >&2)"
                COMMAND bash -c "${LLVM_COV} report ${_unit_binary} --instr-profile=${_profdata}${_source_str} 2> >(grep -v 'mismatched data' >&2)"

                VERBATIM
            )
        endif()

    else()
        # GCC / gcov path
        if(NOT LCOV OR NOT GENHTML)
            message(STATUS "Coverage: skipping 'coverage-full' target (lcov/genhtml not found)")
            return()
        endif()

        set(_lcov_include "")
        foreach(_dir IN LISTS _COV_SOURCE_DIRS)
            list(APPEND _lcov_include "--include" "${_dir}/*")
        endforeach()

        if(NOT TARGET coverage-full)
            add_custom_target(coverage-full
            COMMENT "Running unit + function tests and generating merged gcov coverage report..."
            DEPENDS ${_COV_UNIT_TEST_BINARY} ${_COV_GWHISPER_BINARY} ${_COV_TESTSERVER_BINARY}

            COMMAND ${_unit_binary}

            COMMAND ${Python3_EXECUTABLE} ${_func_test_dir}/runFunctionTest.py
                    ${_gwhisper_bin} ${_testserver_bin} ${_resources_dir} ${_certs_dir}
                    ${_func_test_dir}/completionTests.txt

            COMMAND ${Python3_EXECUTABLE} ${_func_test_dir}/runFunctionTest.py
                    ${_gwhisper_bin} ${_testserver_bin} ${_resources_dir} ${_certs_dir}
                    ${_func_test_dir}/rpcExecutionTests.txt

            COMMAND ${Python3_EXECUTABLE} ${_func_test_dir}/runFunctionTest.py
                    ${_gwhisper_bin} ${_testserver_bin} ${_resources_dir} ${_certs_dir}
                    ${_func_test_dir}/rpcTimeoutFunctionTests.txt

            COMMAND ${Python3_EXECUTABLE} ${_func_test_dir}/runFunctionTest.py
                    ${_gwhisper_bin} ${_testserver_bin} ${_resources_dir} ${_certs_dir}
                    ${_func_test_dir}/cacheFunctionTests.txt

            COMMAND ${Python3_EXECUTABLE} ${_func_test_dir}/runFunctionTest.py
                    ${_gwhisper_bin} ${_testserver_bin} ${_resources_dir} ${_certs_dir}
                    ${_func_test_dir}/sslFunctionTests.txt

            COMMAND ${CMAKE_COMMAND} -E make_directory ${_outdir}
            COMMAND ${LCOV} --capture
                    --directory ${CMAKE_BINARY_DIR}
                    --output-file ${_outdir}/lcov.info
                    ${_lcov_include}
            COMMAND ${GENHTML} ${_outdir}/lcov.info
                    --output-directory ${_outdir}

            VERBATIM
            )
        endif()
    endif()
endfunction()
