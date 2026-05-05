cmake_minimum_required(VERSION 3.20)
project(sstl_tests C CXX)

include(CTest)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(SSTL_ROOT "" CACHE PATH "Path to the SSTL implementation root containing include/sstl")
if(NOT SSTL_ROOT)
  message(FATAL_ERROR "Set -DSSTL_ROOT=<implementation-root>. The non-deprecated workspace currently contains only the test package.")
endif()

set(SSTL_INCLUDE_DIR "${SSTL_ROOT}/include")
if(NOT EXISTS "${SSTL_INCLUDE_DIR}/sstl")
  message(FATAL_ERROR "SSTL include directory not found: ${SSTL_INCLUDE_DIR}/sstl")
endif()

set(SSTL_TEST_SUPPORT "${CMAKE_CURRENT_LIST_DIR}/test_support")
set(SSTL_ERROR_POLICY "SSTL_RETURN" CACHE STRING "SSTL_ON_ERROR value for this configure")
set_property(CACHE SSTL_ERROR_POLICY PROPERTY STRINGS SSTL_RETURN SSTL_PANIC SSTL_UB)
option(SSTL_ENABLE_COVERAGE "Enable host coverage flags" OFF)
option(SSTL_ENABLE_FREESTANDING_PROBES "Build freestanding compile/link probes" OFF)
option(SSTL_ENABLE_LIBFUZZER "Build libFuzzer targets instead of seed replays" OFF)
set(SSTL_COVERAGE_INPUT "" CACHE FILEPATH "Coverage report input for scripts/coverage_report.py")
set(SSTL_COVERAGE_FORMAT "lcov" CACHE STRING "Coverage input format: llvm-json, lcov, or gcov-summary")
set_property(CACHE SSTL_COVERAGE_FORMAT PROPERTY STRINGS llvm-json lcov gcov-summary)

# These tests deliberately call capacity/bounds/error paths. They are valuable
# in RETURN lanes, where soft-failure sentinels are the specified result, and
# in targeted PANIC tests that install a recoverable panic hook. They must not
# be registered in the generic PANIC or UB lanes: default PANIC terminates, and
# UB gives the implementation license to omit recovery checks and marks any
# retained guard as unreachable by design.
set(SSTL_CONTRACT_VIOLATION_TESTS
  cpp03_vector_capacity
  cpp03_vector_capacity_parameter_set
  cpp03_vector_swap
  cpp03_vector_resize
  cpp03_string_capacity
  cpp03_string_mutation_members
  cpp03_forward_list_contracts
  cpp03_zero_capacity
  cpp03_public_family_contracts
  cpp03_coverage_gap_contracts
  stress_cpp03_vector_cycles
  parity_c_cpp_vector_trace
  c99_vector_capacity
  c99_vector_capacity_parameter_set
  c99_vector_extended_ops
  c99_string_capacity
  c99_string_extended_ops
  c99_string_view_contracts
  c99_queue_stack_priority_queue
  c99_zero_capacity
  c99_compile_probe
  c99_algorithm_helpers_coverage)

function(add_sstl_ctest name)
  if((SSTL_ERROR_POLICY STREQUAL "SSTL_PANIC" OR SSTL_ERROR_POLICY STREQUAL "SSTL_UB")
      AND "${name}" IN_LIST SSTL_CONTRACT_VIOLATION_TESTS)
    return()
  endif()
  add_test(NAME ${name} COMMAND ${name})
endfunction()

function(add_sstl_common_warnings target language_std)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" OR CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    if(language_std STREQUAL "cxx03")
      target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror -std=c++03 -pedantic-errors)
    else()
      target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror -std=c99 -pedantic-errors)
    endif()
  endif()
endfunction()

function(add_sstl_coverage target)
  if(SSTL_ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Coverage branch reports should describe SSTL source decisions, not
    # compiler-generated exception cleanup edges around constructors,
    # assignments, and destructors. The instrumented coverage lane therefore
    # disables C++ exceptions for test executables; normal validation lanes
    # still build with the toolchain defaults.
    target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      target_compile_options(${target} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
      target_link_options(${target} PRIVATE -fprofile-instr-generate -fcoverage-mapping)
    else()
      target_compile_options(${target} PRIVATE --coverage)
      target_link_options(${target} PRIVATE --coverage)
    endif()
  endif()
endfunction()

function(add_sstl_cpp03_test name source)
  add_executable(${name} ${source})
  target_include_directories(${name} PRIVATE
    "${SSTL_INCLUDE_DIR}"
    "${SSTL_TEST_SUPPORT}/cpp03"
    "${SSTL_TEST_SUPPORT}/common")
  set_target_properties(${name} PROPERTIES
    CXX_STANDARD 98
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)
  if(NOT "NO_POLICY" IN_LIST ARGN)
    target_compile_definitions(${name} PRIVATE SSTL_ON_ERROR=${SSTL_ERROR_POLICY})
  endif()
  target_compile_definitions(${name} PRIVATE SSTL_TEST_ITERATOR_DEBUG=1)
  add_sstl_common_warnings(${name} cxx03)
  add_sstl_coverage(${name})
  add_sstl_ctest(${name})
endfunction()

function(add_sstl_c99_test name source)
  add_executable(${name} ${source} "${SSTL_TEST_SUPPORT}/c99/noalloc_wrappers.c")
  target_include_directories(${name} PRIVATE
    "${SSTL_INCLUDE_DIR}"
    "${SSTL_TEST_SUPPORT}/c99"
    "${SSTL_TEST_SUPPORT}/common")
  set_target_properties(${name} PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF)
  if(NOT "NO_POLICY" IN_LIST ARGN)
    target_compile_definitions(${name} PRIVATE SSTL_ON_ERROR=${SSTL_ERROR_POLICY})
  endif()
  add_sstl_common_warnings(${name} c99)
  add_sstl_coverage(${name})
  # Coverage runtimes such as libgcov allocate internally at process exit.
  # Keep heap wrappers enabled for normal validation lanes, but leave them off
  # in coverage builds so the instrumentation can flush counters safely.
  if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang" AND NOT SSTL_ENABLE_COVERAGE)
    target_link_options(${name} PRIVATE
      -Wl,--wrap=malloc
      -Wl,--wrap=calloc
      -Wl,--wrap=realloc
      -Wl,--wrap=aligned_alloc
      -Wl,--wrap=posix_memalign
      -Wl,--wrap=memalign
      -Wl,--wrap=free)
  endif()
  add_sstl_ctest(${name})
endfunction()

function(add_sstl_python_test name source)
  add_test(
    NAME ${name}
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_LIST_DIR}/${source}" --root "${CMAKE_CURRENT_LIST_DIR}")
endfunction()

add_sstl_cpp03_test(cpp03_vector_capacity tests/cpp03/vector/test_vector_capacity.cpp)
add_sstl_cpp03_test(cpp03_policy_matrix tests/cpp03/policy/test_policy_matrix.cpp)
add_sstl_cpp03_test(cpp03_family_policy_capacity tests/cpp03/policy/test_family_policy_capacity.cpp)
add_sstl_cpp03_test(cpp03_vector_try_policy tests/cpp03/vector/test_vector_try_policy.cpp)
add_sstl_cpp03_test(cpp03_vector_capacity_parameter_set tests/cpp03/vector/test_vector_capacity_parameter_set.cpp)
add_sstl_cpp03_test(cpp03_vector_lifetime tests/cpp03/vector/test_vector_lifetime.cpp)
add_sstl_cpp03_test(cpp03_vector_swap tests/cpp03/vector/test_vector_swap.cpp)
add_sstl_cpp03_test(cpp03_vector_resize tests/cpp03/vector/test_vector_resize.cpp)
add_sstl_cpp03_test(cpp03_string_capacity tests/cpp03/string/test_string_capacity.cpp)
add_sstl_cpp03_test(cpp03_string_find_compare tests/cpp03/string/test_string_find_compare.cpp)
add_sstl_cpp03_test(cpp03_string_mutation_members tests/cpp03/string/test_string_mutation_members.cpp)
add_sstl_cpp03_test(cpp03_string_view_contracts tests/cpp03/string_view/test_string_view_contracts.cpp)
add_sstl_cpp03_test(cpp03_family_lifetime tests/cpp03/lifetime/test_family_lifetime.cpp)
add_sstl_cpp03_test(cpp03_list_transfer tests/cpp03/list/test_list_transfer.cpp)
add_sstl_cpp03_test(cpp03_list_same_container_splice tests/cpp03/list/test_list_same_container_splice.cpp)
add_sstl_cpp03_test(cpp03_list_insert_erase_merge tests/cpp03/list/test_list_insert_erase_merge.cpp)
add_sstl_cpp03_test(cpp03_forward_list_contracts tests/cpp03/list/test_forward_list_contracts.cpp)
add_sstl_cpp03_test(cpp03_forward_list_algorithms tests/cpp03/list/test_forward_list_algorithms.cpp)
add_sstl_cpp03_test(cpp03_map_erase tests/cpp03/map/test_map_erase.cpp)
add_sstl_cpp03_test(cpp03_deque_flat_invalidation tests/cpp03/iterator/test_deque_flat_invalidation.cpp)
add_sstl_cpp03_test(cpp03_static_layout tests/cpp03/layout/test_static_layout.cpp)
add_sstl_cpp03_test(cpp03_zero_capacity tests/cpp03/zero_capacity/test_zero_capacity.cpp)
add_sstl_cpp03_test(cpp03_compile_probe tests/cpp03/compile_probe/sstl_cxx03_probe.cpp)
add_sstl_cpp03_test(cpp03_default_policy_debug tests/cpp03/compile_probe/test_default_policy_debug.cpp NO_POLICY)
add_sstl_cpp03_test(cpp03_default_policy_custom_panic_hook tests/cpp03/compile_probe/test_default_policy_custom_panic_hook.cpp NO_POLICY)
add_sstl_cpp03_test(cpp03_default_policy_release tests/cpp03/compile_probe/test_default_policy_release.cpp NO_POLICY)
add_sstl_cpp03_test(cpp03_tracked_compile_probe tests/cpp03/compile_probe/sstl_cxx03_tracked_probe.cpp)
add_sstl_cpp03_test(cpp03_public_family_contracts tests/cpp03/compile_probe/test_public_family_contracts.cpp)
add_sstl_cpp03_test(cpp03_mixed_c_cpp_panic_hook tests/cpp03/compile_probe/test_mixed_c_cpp_panic_hook.cpp)
add_sstl_cpp03_test(cpp03_vector_std_oracle tests/cpp03/parity/test_vector_std_oracle.cpp)
add_sstl_cpp03_test(cpp03_algorithm_oracle tests/cpp03/algorithm/test_algorithm_oracle.cpp)
add_sstl_cpp03_test(cpp03_algorithm_extended_oracle tests/cpp03/algorithm/test_algorithm_extended_oracle.cpp)
add_sstl_cpp03_test(cpp03_algorithm_remaining_oracle tests/cpp03/algorithm/test_algorithm_remaining_oracle.cpp)
add_sstl_cpp03_test(cpp03_coverage_gap_contracts tests/cpp03/coverage/test_coverage_gap_contracts.cpp)
add_sstl_cpp03_test(property_cpp03_vector_random_trace tests/properties/cpp03/prop_vector_random_trace.cpp)
add_sstl_cpp03_test(fuzz_cpp03_vector_replay tests/fuzz/cpp03/fuzz_vector_trace.cpp)
add_sstl_cpp03_test(stress_cpp03_vector_cycles tests/stress/cpp03/stress_vector_cycles.cpp)
add_sstl_cpp03_test(parity_c_cpp_vector_trace tests/parity/test_c_cpp_vector_trace.cpp)
add_sstl_cpp03_test(runtime_interface_comparison tests/runtime/test_interface_runtime_comparison.cpp)
add_sstl_python_test(package_documentation_integrity scripts/check_docs.py)

add_sstl_c99_test(c99_vector_capacity tests/c99/vector/test_vector_capacity.c)
add_sstl_c99_test(c99_policy_matrix tests/c99/vector/test_vector_policy_matrix.c)
add_sstl_c99_test(c99_vector_try_policy tests/c99/vector/test_vector_try_policy.c)
add_sstl_c99_test(c99_vector_capacity_parameter_set tests/c99/vector/test_vector_capacity_parameter_set.c)
add_sstl_c99_test(c99_vector_extended_ops tests/c99/vector/test_vector_extended_ops.c)
add_sstl_c99_test(c99_string_capacity tests/c99/string/test_string_capacity.c)
add_sstl_c99_test(c99_string_policy_capacity tests/c99/string/test_string_policy_capacity.c)
add_sstl_c99_test(c99_string_extended_ops tests/c99/string/test_string_extended_ops.c)
add_sstl_c99_test(c99_string_view_contracts tests/c99/string_view/test_string_view_contracts.c)
add_sstl_c99_test(c99_queue_stack_priority_queue tests/c99/adapters/test_queue_stack_priority_queue.c)
add_sstl_c99_test(c99_list_extended_ops tests/c99/list/test_list_extended_ops.c)
add_sstl_c99_test(c99_map_erase tests/c99/map/test_map_erase.c)
add_sstl_c99_test(c99_flat_unordered_semantics tests/c99/map/test_flat_unordered_semantics.c)
add_sstl_c99_test(c99_zero_capacity tests/c99/zero_capacity/test_zero_capacity.c)
add_sstl_c99_test(c99_static_layout_offsets tests/c99/zero_capacity/test_c_layout_offsets.c)
add_sstl_c99_test(c99_compile_probe tests/c99/compile_probe/sstl_c99_probe.c)
add_sstl_c99_test(c99_default_policy_debug tests/c99/compile_probe/test_default_policy_debug.c NO_POLICY)
add_sstl_c99_test(c99_default_policy_release tests/c99/compile_probe/test_default_policy_release.c NO_POLICY)
add_sstl_c99_test(c99_algorithm_helpers_coverage tests/c99/coverage/test_algorithm_helpers.c)
add_sstl_c99_test(c99_algorithm_contracts tests/c99/algorithm/test_algorithm_contracts.c)
add_sstl_c99_test(reg_zero_capacity_no_zero_length_arrays tests/regression/reg_zero_capacity_no_zero_length_arrays.c)

if(SSTL_ENABLE_FREESTANDING_PROBES)
  add_executable(freestanding_cpp03_probe tests/freestanding/freestanding_cpp03_probe.cpp)
  target_include_directories(freestanding_cpp03_probe PRIVATE "${SSTL_INCLUDE_DIR}")
  target_compile_definitions(freestanding_cpp03_probe PRIVATE SSTL_ON_ERROR=${SSTL_ERROR_POLICY})
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(freestanding_cpp03_probe PRIVATE -std=c++03 -ffreestanding -fno-exceptions -fno-rtti -nostdlib)
    target_link_options(freestanding_cpp03_probe PRIVATE -nostdlib)
  endif()

  add_executable(freestanding_c99_probe tests/freestanding/freestanding_c99_probe.c)
  target_include_directories(freestanding_c99_probe PRIVATE "${SSTL_INCLUDE_DIR}")
  target_compile_definitions(freestanding_c99_probe PRIVATE SSTL_ON_ERROR=${SSTL_ERROR_POLICY})
  if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(freestanding_c99_probe PRIVATE -std=c99 -ffreestanding -nostdlib)
    target_link_options(freestanding_c99_probe PRIVATE -nostdlib)
  endif()
endif()

if(SSTL_ENABLE_LIBFUZZER AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_executable(fuzz_cpp03_vector_libfuzzer tests/fuzz/cpp03/fuzz_vector_trace.cpp)
  target_include_directories(fuzz_cpp03_vector_libfuzzer PRIVATE "${SSTL_INCLUDE_DIR}")
  target_compile_definitions(fuzz_cpp03_vector_libfuzzer PRIVATE SSTL_ON_ERROR=${SSTL_ERROR_POLICY} SSTL_FUZZING=1)
  target_compile_options(fuzz_cpp03_vector_libfuzzer PRIVATE -std=c++03 -fsanitize=fuzzer,address,undefined)
  target_link_options(fuzz_cpp03_vector_libfuzzer PRIVATE -fsanitize=fuzzer,address,undefined)
endif()

if(SSTL_ENABLE_COVERAGE AND SSTL_COVERAGE_INPUT)
  add_custom_target(coverage_summary
    COMMAND ${CMAKE_COMMAND} -E env PYTHONDONTWRITEBYTECODE=1
      python "${CMAKE_CURRENT_LIST_DIR}/scripts/coverage_report.py"
      --root "${CMAKE_CURRENT_LIST_DIR}"
      --input "${SSTL_COVERAGE_INPUT}"
      --format "${SSTL_COVERAGE_FORMAT}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    COMMENT "Checking SSTL coverage thresholds")
endif()
