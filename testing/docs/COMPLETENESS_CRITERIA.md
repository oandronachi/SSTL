# Completeness Criteria

The checker marks the package complete only when:

- `SSTL_ROOT/include/sstl` exists and can be used by CMake.
- Every `REQ-*` in `manifests/requirement_manifest.yaml` is referenced by at least one test.
- Requirement entries marked `trace_required: false` are reserved/non-normative slots and are retained for numbering continuity, not coverage accounting.
- Every API ID in `manifests/api_manifest.yaml` is referenced by at least one test.
- Every virtual path in `manifests/test_manifest.yaml` exists.
- Test-source header metadata (`test_id`, `req_ids`, `api_ids`, and `patterns`) matches the corresponding `manifests/test_manifest.yaml` entry.
- Package Markdown documentation passes the offline documentation integrity check: balanced fenced code blocks plus resolvable local file and heading links.
- No waiver remains open.
- Optional `--run-cmake` configure/build/test succeeds for the selected preset.

The release-level thresholds for this local implementation are: mandatory C99/C++03 compile lanes pass, no allocation audit reports zero heap calls, line coverage is at least 95%, branch coverage is at least 90%, and zero-capacity types compile without zero-length array extensions.

`REQ-RUNTIME-COMPARISON` is a local implementation validation requirement layered on top of the numbered v1.0 functional requirements. It exists so the runtime CSV/GUI/CLI observability lane remains traced without reopening the finalized numbered requirement set.

Coverage manifests store artifact paths relative to the extracted package/workspace when possible. The LCOV branch policy excludes compiler-generated exception-cleanup branches; the coverage lane also disables C++ exceptions so reported branch coverage describes source-level SSTL decisions rather than toolchain EH scaffolding.

Local presets now include RETURN, PANIC, UB, ASan/UBSan, coverage, freestanding probe, and libFuzzer configurations. They remain implementation-root driven: configure with `-DSSTL_ROOT=<local implementation root>`.

Use `scripts/coverage_report.py` to convert `llvm-cov` JSON, LCOV `.info`, or a gcov summary into `manifests/coverage_summary.yaml`; it exits non-zero below the §7.6 thresholds.

The runtime comparison report uses the first row in each comparison group as the baseline for `cpu_diff_percent` and `memory_diff_percent`. Very large memory percentages are expected for tiny view/adaptor objects when compared with heap-backed STL proxy rows because the denominator can be only a few bytes.

For local runs, `scripts/run_local_lanes.py --sstl-root <local implementation root>` executes the main presets in sequence. Coverage summaries can also be wired through CMake by configuring `host-coverage` with `-DSSTL_COVERAGE_INPUT=<report> -DSSTL_COVERAGE_FORMAT=<llvm-json|lcov|gcov-summary>` and building the `coverage_summary` target.

Use `scripts/check_docs.py --root <test package root>` to refresh `manifests/documentation_summary.yaml` and validate the REQ-08 documentation/link integrity lane directly.
