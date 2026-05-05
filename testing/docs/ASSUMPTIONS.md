# Assumptions

This test package captures the current SSTL public contract and validation expectations. The non-deprecated workspace now contains local SSTL headers under `include/sstl`, so the host lanes can be run directly against the implementation root.

Closed local assumptions:

- Docker GCC freestanding probes now provide the local target-smoke substitute for this workspace.
- Deterministic Docker mutation smoke now checks representative C and C++ full-capacity mutants.

Deferred optional surfaces:

- `SSTL_EXTERN` remains intentionally deferred. The current C API is the single-header, macro-instantiated typed API, so generated functions remain `static` and header-local.
- The byte-generic C core over `void*`, element size, and capacity remains intentionally deferred. The shipped C surface is the typed macro API; runtime and no-allocation validation cover that public surface directly.

Run `scripts/check_completeness.py --sstl-root <implementation-root> --run-cmake` after changing the implementation or test package.

Test support headers `test_support/cpp03/noalloc_audit.hpp` and `test_support/cpp03/tracked.hpp` are intentionally single-translation-unit helpers. Each test executable includes them from one source file; multi-TU consumers should move the definitions to one `.cpp` file before reuse. `noalloc_audit.hpp` is a tripwire: allocation exits immediately with a console message, while the counter assertion verifies only the non-failing path.
