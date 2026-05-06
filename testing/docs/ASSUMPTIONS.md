# Assumptions

This test package captures the current SSTL public contract and validation expectations. The non-deprecated workspace now contains local SSTL headers under `include/sstl`, so the host lanes can be run directly against the implementation root.

Closed local assumptions:

- Docker GCC freestanding probes now provide the local target-smoke substitute for this workspace.
- Deterministic Docker mutation smoke now checks representative C and C++ full-capacity mutants.
- The host C++ and C tests use small in-tree harnesses instead of Catch2 v2.x
  and Unity. This keeps the package dependency-light and preserves strict
  C++03/C99 compilation in every Docker and freestanding probe lane.
- Allocation tripwires are active in the normal RETURN, PANIC, UB, and
  ASan/UBSan lanes. The coverage lane disables those wrappers because the
  coverage runtime itself performs internal allocations. MSan and TSan disable
  the C++ operator-new tripwire because their runtimes provide mandatory
  allocation interceptors.
- Clang MSan and TSan presets are part of the local preset set and the
  containerized validation lane when Clang is available. The Docker lane uses
  unconfined seccomp plus `setarch x86_64 -R` so sanitizer shadow-memory
  mappings are stable in virtualized Linux environments.
- The package includes a dependency-light MISRA advisory scan over
  preprocessed representative C macro outputs. It is an advisory gate for the
  local package, not a claim of certified MISRA compliance.
- The FUZZ matrix includes deterministic byte-trace replays for vector,
  string, flat_map, and unordered_map, plus matching libFuzzer build targets
  behind the `host-libfuzzer` preset.
- Vendor compiler coverage is best-effort and installed-toolchain driven. The
  package now runs direct compile probes for MSVC `cl`, ArmClang, GNU Arm
  Embedded (`arm-none-eabi-gcc/g++`), Keil `armcc`, and IAR `iccarm` when those
  tools are discoverable. Missing proprietary/cross toolchains are recorded as
  skipped lanes in `manifests/vendor_compiler_summary.yaml`; any discovered
  compiler that rejects the probe fails the package test.
- The vendor compiler probe can also run against specialized container images
  configured by `SSTL_VENDOR_CONTAINER_ARMCLANG_IMAGE`,
  `SSTL_VENDOR_CONTAINER_ARM_NONE_EABI_IMAGE`,
  `SSTL_VENDOR_CONTAINER_KEIL_ARMCC_IMAGE`, and
  `SSTL_VENDOR_CONTAINER_IAR_ICCARM_IMAGE`, or by passing
  `--vendor-container-image KEY=IMAGE` through `tools/sstl_run_tests.py`.
  Missing configured images are not pulled silently: the default policy asks
  first, denied pulls and network/pull failures skip the dependent image-backed
  lanes, and the skip reason is recorded in `vendor_compiler_summary.yaml`.

Deferred optional surfaces:

- `SSTL_EXTERN` remains intentionally deferred. The current C API is the single-header, macro-instantiated typed API, so generated functions remain `static` and header-local.
- The byte-generic C core over `void*`, element size, and capacity remains intentionally deferred. The shipped C surface is the typed macro API; runtime and no-allocation validation cover that public surface directly.

Run `scripts/check_completeness.py --sstl-root <implementation-root> --run-cmake` after changing the implementation or test package.

Test support headers `test_support/cpp03/noalloc_audit.hpp` and `test_support/cpp03/tracked.hpp` are intentionally single-translation-unit helpers. Each test executable includes them from one source file; multi-TU consumers should move the definitions to one `.cpp` file before reuse. `noalloc_audit.hpp` is a tripwire: allocation exits immediately with a console message, while the counter assertion verifies only the non-failing path.
