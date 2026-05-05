# Traceability

The suite maps SSTL validation cases into executable files:

For the gate definition and local assumptions that support this mapping, see
[Completeness Criteria](COMPLETENESS_CRITERIA.md) and [Assumptions](ASSUMPTIONS.md).

| Test plan item | Implemented files |
|---|---|
| TC-U01 capacity and full mutation | `tests/cpp03/vector/test_vector_capacity.cpp`, `tests/cpp03/string/test_string_capacity.cpp`, `tests/c99/vector/test_vector_capacity.c`, `tests/c99/string/test_string_capacity.c` |
| TC-U02 policy/try access | `tests/cpp03/vector/test_vector_try_policy.cpp`, `tests/c99/vector/test_vector_try_policy.c`, `tests/cpp03/policy/test_policy_matrix.cpp`, `tests/c99/vector/test_vector_policy_matrix.c`; PANIC/UB presets are configured through `SSTL_ERROR_POLICY`. |
| Non-vector policy/capacity breadth | `tests/cpp03/policy/test_family_policy_capacity.cpp`, `tests/c99/string/test_string_policy_capacity.c` |
| TC-U03 swap and iterator/storage semantics | `tests/cpp03/vector/test_vector_swap.cpp` |
| Vector resize contract | `tests/cpp03/vector/test_vector_resize.cpp` covers grow, shrink, over-capacity failure, and zero-capacity resize. |
| String extended mutators/searches | `tests/cpp03/string/test_string_mutation_members.cpp` and `tests/c99/string/test_string_extended_ops.c` cover assign, bounded append, insert, erase, replace, operator append, and character-set searches. |
| TC-U04 list transfer semantics | `tests/cpp03/list/test_list_transfer.cpp` |
| Same-container list splice | `tests/cpp03/list/test_list_same_container_splice.cpp` |
| List insert/erase/merge | `tests/cpp03/list/test_list_insert_erase_merge.cpp` and `tests/c99/list/test_list_extended_ops.c` cover iterator-positioned C++ list insertion/erasure plus destination-owned sorted merge and C macro parity. |
| Forward-list splice-after | `tests/cpp03/list/test_forward_list_contracts.cpp` |
| Forward-list algorithms | `tests/cpp03/list/test_forward_list_algorithms.cpp` and `tests/c99/list/test_list_extended_ops.c` cover remove, remove_if, unique, sort, and merge. |
| Map erase | `tests/cpp03/map/test_map_erase.cpp` and `tests/c99/map/test_map_erase.c` cover iterator/key erase and C key erase. |
| TC-U05 zero-capacity | `tests/cpp03/zero_capacity/test_zero_capacity.cpp`, `tests/c99/zero_capacity/test_zero_capacity.c`, `tests/regression/reg_zero_capacity_no_zero_length_arrays.c` |
| Capacity boundary parameter set | `tests/cpp03/vector/test_vector_capacity_parameter_set.cpp`, `tests/c99/vector/test_vector_capacity_parameter_set.c` cover `0,1,2,16,1024`. |
| Non-vector capacity parameter set | `tests/cpp03/policy/test_family_policy_capacity.cpp` covers string/deque/list and associative full boundaries; `tests/c99/string/test_string_policy_capacity.c` covers C string `0,1,2,16,1024`. |
| TC-I01 deterministic STL oracle | `tests/cpp03/parity/test_vector_std_oracle.cpp`, `tests/properties/cpp03/prop_vector_random_trace.cpp`, `tests/cpp03/algorithm/test_algorithm_oracle.cpp`, `tests/cpp03/string/test_string_find_compare.cpp` |
| TC-I02 C/C++ parity | `tests/parity/test_c_cpp_vector_trace.cpp` |
| C extended vector parity | `tests/c99/vector/test_vector_extended_ops.c` covers generated C resize, insert, and erase. |
| TC-I03 compile/noalloc/freestanding | `tests/cpp03/compile_probe/sstl_cxx03_probe.cpp`, `tests/c99/compile_probe/sstl_c99_probe.c`, `tests/freestanding/freestanding_cpp03_probe.cpp`, `tests/freestanding/freestanding_c99_probe.c`, noalloc support headers |
| C++03 Tracked compile probe | `tests/cpp03/compile_probe/sstl_cxx03_tracked_probe.cpp` |
| Public family contracts | `tests/cpp03/compile_probe/test_public_family_contracts.cpp` covers array/span/deque/list/forward_list/map/set/flat/unordered/adaptor/utility smoke behavior. |
| Family lifetime | `tests/cpp03/lifetime/test_family_lifetime.cpp` covers deque/list/forward_list/unordered_map/unordered_set/optional/variant with `Tracked`. |
| string_view contracts | `tests/cpp03/string_view/test_string_view_contracts.cpp`, `tests/c99/string_view/test_string_view_contracts.c`, and the public C compile probe cover C++ and C view APIs. |
| STATIC-LAYOUT | `tests/cpp03/layout/test_static_layout.cpp`, `tests/regression/reg_zero_capacity_no_zero_length_arrays.c` |
| ITER-INVALIDATION deviations | `tests/cpp03/iterator/test_deque_flat_invalidation.cpp` plus list/forward-list transfer tests |
| Extended algorithms | `tests/cpp03/algorithm/test_algorithm_extended_oracle.cpp` expands non-modifying, modifying, heap, partition, set, and numeric coverage. |
| Remaining algorithms | `tests/cpp03/algorithm/test_algorithm_remaining_oracle.cpp` covers the remaining §5.16 algorithm families called out by reassessment v3. |
| Iterator debug hook | `test_support/cpp03/iterator_contract.hpp` defines the required test-only validity hook used by `tests/cpp03/iterator/test_deque_flat_invalidation.cpp` so invalidation can be asserted without dereferencing invalid iterators. |
| TC-I04 target smoke | Docker GCC `host-freestanding-probes` lane; evidence in `manifests/docker_validation.yaml`. |
| Mutation smoke | `scripts/mutation_smoke.py`; evidence in `manifests/mutation_summary.yaml`. |
| TC-E01/E02 consumer adoption | CMake presets and compile probes are the minimal consumer samples. |
| Documentation render/link integrity | `scripts/check_docs.py` validates package Markdown structure plus local file and heading links; evidence in `manifests/documentation_summary.yaml`. |

Machine-readable traceability lives in `manifests/test_manifest.yaml`.
