# Product Requirements Document: Static-Allocation STL‑Like Library (SSTL)

## Introduction

The Static‑Allocation STL‑Like Library (SSTL) is a lightweight, embedded‑friendly alternative to the C++ Standard Template Library (STL).  The project delivers a set of generic containers, algorithms and utility types for both **C99** and **C++03** that mirror the familiar STL APIs but **never allocate from the heap**.  Capacity is a compile‑time constant, storage is embedded inline within each object, and all operations are designed to work in bare‑metal or freestanding environments.  SSTL is distributed as raw source code, is header‑only by default, and is licensed under the MIT license.

The purpose of this document is to outline the product’s scope, goals, functional and non‑functional requirements, assumptions, success criteria and release plan.  It serves as the primary reference for stakeholders, developers and testers to ensure that SSTL delivers a consistent and verifiable experience across C and C++.

## Goals and Objectives

1. **Zero dynamic allocation:** Provide a container library that operates entirely on statically allocated memory without calling `malloc`, `free`, `new` or `delete`.  Users can safely link the library in systems where heap allocation is prohibited or unavailable.
2. **STL‑fluent interface:** Mirror the names and behaviour of standard STL containers so that developers familiar with `std::vector`, `std::map` and related types can migrate to SSTL with minimal code changes.  Where features are omitted (e.g., allocator parameters, move semantics), provide documented substitutes.
3. **C and C++ parity:** Offer parallel APIs for C++03 and C99.  C++ users interact with class templates like `sstl::vector<T,N>`, while C users instantiate typed structures via macros such as `SSTL_VECTOR_DECLARE(my_vec, int, 16)`.  Both languages expose a consistent set of operations and error‑handling policies.
4. **Deterministic behaviour:** Guarantee deterministic complexity and memory usage.  All operations complete in predictable time; no hidden dynamic allocations are performed.
5. **Freestanding compatibility:** Ensure the library builds under `-ffreestanding` and links without the standard C or C++ runtime.  Only minimal headers (`<stddef.h>`, `<stdint.h>`, `<string.h>`, `<limits.h>`) are required in C, and `<cstddef>`, `<cstring>` and `<new>` in C++.  No use of hosted‑only functionality (e.g., streams, exceptions, RTTI) is permitted.
6. **Comprehensive testing:** Accompany the interface with a rigorous test plan that covers unit, integration and end‑to‑end scenarios across multiple capacities, element types, policies and target architectures.  Aim for ≥95 % line coverage and ≥90 % branch coverage on the SSTL translation units.

## User Needs and Use Cases

### Embedded developers

- Operate on fixed‑size collections in microcontrollers or DSPs where heap allocation is disabled.  They need familiar data structures without worrying about `malloc` failure.
- Avoid vendor‑specific libraries; reuse code across C and C++ projects with a unified API surface.
- Rely on deterministic memory usage to satisfy real‑time constraints and safety‑critical certifications (e.g., MISRA C).  SSTL’s compile‑time capacities facilitate static analysis.

### STL‑fluent programmers porting to embedded

- Quickly migrate existing C++ STL code to an environment that forbids dynamic allocation.  They benefit from the migration‑compatibility rule (MCR) that preserves the names and semantics of non‑allocating STL members and flags unsupported ones at compile time.
- Use C++03 features only, since many embedded toolchains do not support modern dialects.  They expect `copy` semantics instead of move, no variadic templates, and no initializer lists.

### C programmers seeking generic containers

- Define typed containers with macros and operate on them via functions like `my_vec_push_back(&v, x)`.  They expect explicit size queries, capacity checks and out‑parameter returns rather than exceptions.
- Adopt optional byte‑generic versions for code‑size‑critical projects where function bodies operate on `void*` and element sizes.

## Functional Requirements

### Container families

SSTL provides a family of containers analogous to the standard library.  For each entry `X` below, a C++03 template `sstl::X<T,N>` and a C macro instantiation family `SSTL_X_DECLARE(NAME,T,CAP)`/`SSTL_X_DEFINE(NAME,T,CAP)` must exist.  `N`/`CAP` denotes the compile‑time capacity and cannot be altered at runtime.

| Container | Description | Priority |
|---|---|---|
| **array<T,N>** | Fixed‑size wrapper around `T[N]` with STL `begin()/end()`, `at()`, `operator[]`, `front()`, `back()` and `size()`; no mutation of capacity. | P0 (core) |
| **vector<T,N>** | Dynamic‑size sequence with inline buffer; supports `size()`, `capacity()`, `empty()`, `full()`, `push_back`, `pop_back`, `insert`, `erase`, `clear`, `assign` and iterator operations.  Overflow handled per error policy. | P0 |
| **string<N>** | Null‑terminated `char[N+1]` wrapper with `size()`, `capacity()`, `push_back`, `append`, `assign`, `find`, `substr` and conversion to/from C strings; uses unsigned indices. | P0 |
| **span<T> / string_view** | Non‑owning view over existing contiguous storage; provides read‑only access with `data()`, `size()`, iterators; no capacity or ownership. | P0 |
| **deque<T,N>** | Ring buffer supporting push/pop at both ends and random access; uses static inline buffer sized for `N` elements. | P1 |
| **queue<T,N> / stack<T,N>** | Adapters over `deque` (queue) and `vector` (stack) providing `push()`, `pop()`, `front()`/`back()`/`top()`. | P1 |
| **priority_queue<T,N>** | Binary heap implemented on static array; supports `push()`, `pop()`, `top()` and returns `bool` on push failure. | P1 |
| **forward_list<T,N> / list<T,N>** | Singly and doubly linked lists with fixed node pool; provide `push_front`, `pop_front`, `insert_after`, `erase_after`, `splice`, `merge`, `sort`, `reverse`.  Cross‑container transfers require destination capacity and return `bool`. | P1 |
| **set/map<...,N>** and **flat_set/map<...,N>** | Ordered associative containers implemented either as RB‑trees (node‑based) or sorted arrays; support key comparison functions and search/insert/erase operations. | P1 |
| **unordered_set/map<...,N>** | Hash table with fixed bucket count; user can override the number of buckets `B`; open addressing or separate chaining designs are acceptable if deterministic. | P2 (future release) |
| **bitset<N>** | Container for `N` bits; supports bitwise operations, `set()`, `reset()`, `flip()`, `count()`, and indexing. | P2 |
| **optional<T>** | Lightweight tagged union that stores either a `T` or no value; implements `emplace`, `has_value`, `value_or`, `reset`. | P2 |
| **variant<...>** | Fixed‑arity tagged union (`variant2`, `variant3`, `variant4`) storing one of several types; provides `index()`, `visit()`, and `holds_alternative<T>()`. | P2 |
| **inplace_function<Sig,Sz>** | Type‑erased callable with static inline storage; limited arity `function0/1/2/3`; stores either function pointer + context pointer or small callable object. | P2 |

### Algorithms and utilities

1. **Subsets of `<algorithm>`:** Provide stateless algorithms (e.g., `copy`, `fill`, `sort`, `find`, `accumulate`, `transform`) implemented without heap or recursion.  Overload to operate on iterators of SSTL containers or plain pointers.
2. **Iterator abstraction:** All iterators satisfy the STL iterator category requirements where feasible.  Contiguous containers expose `T*` iterators; node‑based containers have nested iterator classes with `operator++`, `operator--`, comparison and dereference.  C iterators are opaque and manipulated via `NAME_iterator_begin`, `NAME_iterator_next`, etc.
3. **Error policy:** A compile‑time macro `SSTL_ON_ERROR` selects one of:
   - **PANIC:** call user‑supplied `void sstl_panic(const char *msg)` and abort; no return value.
   - **RETURN (default in release):** operations return `bool` or `NULL` to indicate failure (e.g., push into full container, `at()` out of range).  C++ `at()` still returns `T&` and will call panic only in PANIC mode.
   - **UB:** perform no checks and let undefined behaviour occur; used to minimise code size on constrained devices.

   A family of `try_*` functions bypasses the global policy and always uses RETURN semantics (e.g., `try_push_back`, `try_at`).
4. **Constructors and assignment:** C++03 containers support default construction, copy construction, and copy assignment.  No move semantics or variadic templates; `assign(const T*, size_type)` and `assign(InputIt, InputIt)` substitute for initializer‑list forms.  C API exposes explicit initialisation (`NAME_init`) and copy operations (`NAME_assign`).
5. **Iterator invalidation:** Follows STL rules where the underlying structure matches (e.g., `vector` invalidates on reallocation, which cannot occur here; `list` invalidates only the erased element).  Where SSTL deviates (e.g., ring buffer `deque`), stricter rules are documented.
6. **Swap semantics:** For containers with inline storage, `swap(a,b)` exchanges elements up to `min(a.size,b.size)`, then moves the remainder; complexity is linear, not constant.
7. **Zero‑capacity instantiation:** Containers may be defined with `N==0`; they compile cleanly under C99/C++03.  `capacity()==0`, `empty()==true`, `full()==true` and all mutators return failure per policy.  No zero‑length arrays are used internally.
8. **Header layout:** Public headers reside in `<sstl/*.hpp>` for C++ and `<sstl/c/*.h>` for C.  The library is header‑only by default; defining `SSTL_EXTERN` before including an implementation header emits out‑of‑line definitions into one `.c` or `.cpp` translation unit for code‑size control.

### Interface and migration rules

1. **Migration Compatibility Rule (MCR):** For each STL container `std::X` with an SSTL counterpart, all non‑allocating, non‑throwing methods that depend only on stored elements must be present in SSTL with the same name and behaviour (e.g., `size`, `empty`, `begin`, `end`, `front`, `back`, `insert`, `erase`).  Methods that allocate (`reserve`, `shrink_to_fit`) or require C++11 features (e.g., `emplace`, initializer‑list constructors) are omitted.  Overflows and invalid operations are routed through the error policy.
2. **Namespace and prefixing:** All C++ entities reside in the `sstl` namespace.  C functions and types begin with the user‑provided `NAME_` prefix; macros use `SSTL_` to avoid global namespace pollution.  Avoid the prefix `etl` to prevent confusion with existing libraries.
3. **Comparator and hashing:** Users may provide custom comparator or hash functors.  C++ defaults are `sstl::less<T>`, `sstl::equal_to<T>` and `sstl::hash<T>` specialised for integers, pointers and `sstl::string<N>`.  C defaults include `sstl_cmp_int`, `sstl_cmp_uint`, `sstl_cmp_str` and `sstl_hash_bytes` (FNV‑1a).  Hash table sizes may be overridden via template parameter `B`.

## Non‑Functional Requirements

### Performance and complexity

- **Deterministic complexity:** All container operations must match or exceed the asymptotic complexity of their STL counterparts.  `vector` push and pop are amortised O(1) with no reallocation; `map`/`set` operations are O(log N); algorithms such as `sort` operate in O(N log N).  There must be no hidden allocation or recursion that could cause unpredictable delays.
- **Memory footprint:** Storage is fully inline.  The size of `sstl::vector<T,N>` equals `sizeof(T)*N + sizeof(size_type) + alignment padding`.  No metadata pointers or hidden heap handles exist.  C macros produce POD structs containing an array and a `size` field.  For node‑based containers, a pool of `N` nodes plus pointers is embedded; in total memory usage remains O(N * sizeof(node)).
- **Thread safety:** SSTL offers no internal synchronization.  Concurrent access requires external protection by the user.  The library must be reentrant as long as separate containers are manipulated independently.
- **Code size:** The default header‑only form should compile with optimisation settings typical of embedded compilers.  Users may define `SSTL_EXTERN` to emit one `.c` file containing function bodies to improve link‑time deduplication and reduce flash usage.

### Portability and compliance

- **Language standards:** C++ code must compile with `-std=c++03 -pedantic-errors` and no exceptions or RTTI.  C code must compile with `-std=c99 -pedantic-errors` and rely only on standard headers listed above.  Compilers tested include GCC, Clang and `arm-none-eabi-gcc`; IAR, Keil, ArmClang and MSVC are best‑effort.
- **Freestanding mode:** The library must compile under `-ffreestanding -nostdlib` on the supported compilers.  Symbol names from the C/C++ runtime (e.g., `malloc`, `printf`) should not appear in the object files; a no‑allocation audit ensures zero references.
- **MISRA C alignment:** Code should align with MISRA C 2012 guidelines where possible; however, strict compliance is not required.  Dangerous constructs like recursion, dynamic allocation and unspecified behaviour are avoided.
- **Binary compatibility:** There is no attempt to maintain ABI compatibility across compilers or between C++ and C; all types are defined in headers and may vary in layout.  Nevertheless, the objects are trivially relocatable (POD or user‑declared relocatable), enabling placement in special sections such as `.dtcm` or `.ccmram` via user attributes.

### Quality and testing

1. **Requirement coverage:** Each functional requirement is mapped to at least one test case in the accompanying test plan.  The traceability matrix (Section 7.5 of the interface specification) must be maintained and all tests must pass before release.
2. **Release gates:** The test harness enforces the following gates:
   - All mandatory compile profiles (C99 and C++03) compile without errors (`G‑01`).
   - The no‑allocation audit reports zero intercepted heap symbols (`G‑02`).
   - Every requirement has at least one passing test (`G‑03`).
   - All unit, integration and end‑to‑end tests pass (`G‑04`).
   - Zero‑capacity instantiations compile cleanly with no zero‑length array extensions (`G‑05`).
   - Line coverage ≥95 % and branch coverage ≥90 % (`G‑06`).

3. **Test data:** A set of canonical test data is defined (primitive types, various capacities, lifecycle tracking types, deterministic PRNG seeds, cross‑container ranges, freestanding profiles, ARM/AVR configs, sample markdown) and is reused across test cases to ensure reproducibility.

### Documentation and distribution

1. **Packaging:** Source code is released as a set of headers under `include/sstl/` and `include/sstl/c/`, along with one optional `.c` or `.cpp` file for `SSTL_EXTERN`.  Documentation comprises the design document, interface specification, test plan and this PRD.  A small sample program demonstrating SSTL integration in C++03 and C99 is provided to validate installation and usage.
2. **Versioning:** Follow Semantic Versioning.  The initial release is version 1.0.0 covering P0 and P1 components.  A follow‑on release (v1.1) may add P2 features such as hash tables, `bitset`, `optional`, `variant` and `inplace_function` while preserving backward compatibility.

## Out of Scope

- **Dynamic memory:** The library does not provide any heap allocation or allocators.  There is no support for dynamic growth, `reserve()`, `shrink_to_fit()` or memory pools in v1.0.
- **Modern C++ features:** No support for move semantics, variadic templates, initializer lists, `constexpr`, `noexcept`, type traits, `nullptr`, ranged for loops or other features introduced after C++03.
- **Streams, locale, regex, chrono, filesystem and threading primitives:** These are outside the scope of SSTL and remain the responsibility of the application or other libraries.
- **Default hashing/traversal algorithms with unpredictable performance:** Only deterministic algorithms with well‑defined complexity are included.  No fallback to host runtime functions (e.g., `qsort`) is used.
- **High‑impact features such as concurrency or persistent storage:** These are deferred to future versions or external libraries.

## Assumptions and Dependencies

1. Developers will supply appropriate panic handlers when using the PANIC error policy.  In embedded systems, this may tie into watchdog resets or logging mechanisms.
2. Users must choose a compile‑time capacity `N` large enough for their workloads.  Overflow behaviour depends on the selected error policy; in RETURN mode the operation fails gracefully.
3. The element type `T` used in containers must be copy constructible and assignable.  For C containers, `T` must be a POD or satisfy a documented byte‑copyable contract; non‑POD types require manual cleanup before removal.
4. For pointer‑based containers (maps, lists), users provide comparator or hash functions consistent with the semantics of the keys to ensure correct ordering and hashing.
5. Build systems will include the necessary compile flags (`-std=c99` or `-std=c++03`, `-pedantic-errors`, `-ffreestanding` for freestanding builds).  Environment‑specific integration (linker scripts, memory section placement) is handled outside SSTL.

## Success Metrics and Acceptance Criteria

1. **Functional completeness:** All P0 and P1 containers and algorithms defined in this PRD must be implemented and callable via both the C and C++ APIs.  Optional P2 components may be stubbed or deferred to v1.1.
2. **No hidden allocations:** A build configured with the no‑allocation audit must show zero calls to `malloc`, `free`, `new`, `delete` or their variants.  Static analysis and runtime symbol interception verify this criterion.
3. **Deterministic behaviour:** Unit tests demonstrate that container operations respect capacity limits, return correct error codes, maintain invariants and meet documented complexity.  Cross‑language parity tests confirm identical logical behaviour of C and C++ implementations.
4. **Portability:** The library must compile and run on at least one 32‑bit desktop compiler (gcc/clang) and one embedded cross‑compiler (e.g., `arm-none-eabi-gcc`) in both C and C++ modes.  Freestanding tests must link without referencing host runtime symbols.
5. **Compliance with MCR:** A migration assessment on a representative STL codebase (provided in integration tests) must demonstrate that the only required changes are type aliases and capacity/error policy selection.  Any missing methods should produce compilation errors rather than linking or runtime errors.
6. **Test coverage and release gates:** All gates described in the non‑functional section must be met.  Failing to achieve the coverage or passing thresholds blocks the release.

## Milestones and Release Plan

The project will be delivered in phases to manage complexity and allow feedback:

1. **Design and specification (completed):** Finalise the design document and interface specification (completed as of 2026‑04‑28 and 2026‑05‑02 respectively).  Resolve all open design questions and freeze the API.
2. **Prototype implementation (Month 0–1):** Develop initial versions of P0 containers (`array`, `vector`, `string`, `span`) in C and C++.  Focus on core functionality, static memory layout and error policy plumbing.  Verify compile‑time constraints and run basic unit tests.
3. **P1 development (Month 1–3):** Implement `deque`, `queue`, `stack`, `priority_queue`, `list`, `forward_list` and ordered associative containers (`map`, `set`, `flat_map`, `flat_set`).  Complete the algorithm subset and iterators.  Introduce the no‑allocation audit harness and integration tests.  Reach a feature‑complete v1.0 candidate by the end of Month 3.
4. **Quality assurance and documentation (Month 3–4):** Expand the test suite, measure coverage, perform cross‑language and cross‑architecture tests, and fix defects.  Prepare documentation, examples and packaging.  Achieve release‑gate thresholds.  Publish v1.0.0.
5. **Maintenance and P2 features (Month 4+):** After v1.0 is released, begin work on P2 features (`unordered_map/set`, `bitset`, `optional`, `variantN`, `inplace_function`) as minor version updates.  Consider user feedback and requests for external buffer adoption (v1.1).  Explore modern‑C++ optional backports in a v2.0 branch.

## Future Considerations

While SSTL v1.0 intentionally targets older language standards and embedded constraints, future iterations may:

- Explore a modern C++11/14 variant that supports move semantics, variadic templates, `constexpr` and other enhancements for hosted environments.
- Provide optional external‑buffer modes to adopt memory supplied by the user at runtime, enabling dynamic capacities within static footprints.
- Introduce concurrency primitives (mutexes, lock‑free queues) once memory barriers and portability concerns are fully addressed.
- Evaluate additional container types such as `tuple`, `multimap`, `multiset` or `regex` if demand arises.

## Conclusion

This Product Requirements Document outlines the vision, scope and detailed requirements for the Static‑Allocation STL‑Like Library.  By focusing on zero dynamic allocation, parity with the familiar STL API, deterministic behaviour and cross‑language compatibility, SSTL aims to empower embedded and safety‑critical developers with a robust, drop‑in alternative to the standard library.  Adhering to the specified functional and non‑functional requirements, test coverage and release milestones will ensure that the resulting library meets the needs of its users and lays a solid foundation for future enhancements.