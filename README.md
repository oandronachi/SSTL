**Why this project matters**: SSTL is a constrained C/C++ infrastructure project used as an experiment in AI-assisted SDLC. It combines PRD-driven development, explicit architectural constraints, generated design/code/test artifacts, human review, TDD, guardrail validation, and deterministic memory rules.

# Static-Allocation STL-Like Library (SSTL)

[![CI](https://github.com/oandronachi/SSTL/actions/workflows/ci.yml/badge.svg)](https://github.com/oandronachi/SSTL/actions/workflows/ci.yml)
[![License](https://img.shields.io/github/license/oandronachi/SSTL)](./LICENSE)
[![C99](https://img.shields.io/badge/C-99-555555?logo=c&logoColor=white)](./README.md)
[![C++03](https://img.shields.io/badge/C%2B%2B-03-00599C?logo=cplusplus&logoColor=white)](./README.md)
[![Heap-free](https://img.shields.io/badge/heap--free-runtime-2ea44f)](./README.md)

SSTL is a lightweight alternative to the C++ Standard Template Library for C99 and C++03 environments. It provides a family of familiar containers, algorithms, and utilities that operate entirely on statically allocated memory — no calls to `malloc`, `free`, `new`, or `delete` are made at runtime. This makes SSTL suitable for bare-metal microcontrollers, safety-critical systems, and any environment where heap allocation is undesirable or unavailable.

SSTL was developed as an AI-assisted, human-in-the-loop engineering experiment. The project explores how generative AI can be used to accelerate the design and implementation of constrained C/C++ infrastructure while keeping a human responsible for the concept, constraints, architectural direction, review, and validation.

## Human contribution / Author’s role

This project should not be understood as a traditional solo, line-by-line manual implementation. It was created through an AI-assisted development process in which my role was to define the problem space, steer the engineering direction, review generated outputs, and keep the project aligned with its intended constraints.

My contribution included:

- defining the overall concept of a static-allocation STL-like library for constrained C and C++ environments;
- setting the core constraints: no runtime dynamic allocation, C99/C++03 compatibility, embedded-friendly design, predictable capacity handling, and portability across toolchains;
- guiding the AI-generated design and implementation work through iterative prompting, review, and correction;
- identifying when generated solutions diverged from the intended architecture, memory model, API style, or embedded-systems assumptions;
- providing feedback to redirect the implementation toward deterministic memory use, explicit capacity handling, and C/C++ API parity;
- reviewing the generated artifacts for consistency with the project goals and expected usage model;
- using tests, specifications, and iterative feedback as guardrails to validate and refine the result.

The AI systems assisted with generating design material, implementation code, documentation, and test-related artifacts. The human role was to orchestrate this process: define what the project should be, evaluate whether the generated work matched that intent, detect incorrect or unsuitable directions, and steer the project back toward the desired architecture.

SSTL is therefore best viewed as an AI-assisted engineering and validation experiment. It demonstrates concept ownership, architectural steering, requirements clarification, technical review, embedded-systems judgment, and the disciplined use of generative AI for constrained C/C++ software development.

## Overview

* **Generative AI origin.**  SSTL’s design and implementation were created and validated by generative AI systems using a human‑in‑the‑loop (HITL) process.  The design document, interface specification and test plan were drafted by ChatGPT, Gemini and Claude through a structured debate mechanism, with changes accepted only when consensus was reached.  Development code was synthesised by OpenAI Codex with guidance from ChatGPT and guard‑rail reviews by Claude and ChatGPT.  Every generated artefact was validated against the design and test specifications using a test‑driven development (TDD) approach.
* **Static memory only.**  All containers embed their storage inline and expose a compile‑time capacity parameter `N`/`CAP`.  Operations such as `push_back` return a boolean to indicate success when capacity is exhausted.  No dynamic allocation, exceptions or runtime type information are used.
* **C and C++ parity.**  For each container there is a C++ template `sstl::X<T,N>` and a matching C macro family `SSTL_X_DECLARE(NAME,T,CAP)`/`SSTL_X_DEFINE(NAME,T,CAP)`.  C functions return status codes and use out‑parameters in place of references and exceptions.  Both languages share the same algorithm library and error‑handling policies.
* **Embedded‑friendly.**  The code builds with `-std=c99` or `-std=c++03` and is freestanding‑compatible.  Only minimal headers (`<stddef.h>`, `<stdint.h>`, `<string.h>`, `<limits.h>` in C; `<cstddef>`, `<cstring>` and `<new>` in C++) are used.  The error policy (`SSTL_ON_ERROR`) can be configured globally.
* **Documented and tested.**  All public APIs are annotated with Doxygen comments, so the documentation is embedded in the source.  A comprehensive test suite (unit, integration and end‑to‑end) accompanies the library.  Helper scripts support building, testing, documenting and cleaning the project, and they work both as command‑line tools and via a simple GUI when double‑clicked.

## Getting Started

### Requirements

* A C99 or C++03 compiler (e.g. GCC ≥ 4.8, Clang ≥ 3.4, arm‑none‑eabi‑gcc).  The library is header‑only by default; defining `SSTL_EXTERN` before including an SSTL header emits definitions into a translation unit for improved code‑size optimisation.
* [CMake](https://cmake.org/) ≥ 3.14 if you wish to build the bundled tests.
* Python 3.8+ to run the helper scripts under `tools/` (see below).

### Installation

1. **Clone the repository.**  The `include/sstl/` and `include/sstl/c/` directories contain the C++ and C headers respectively.  You can add `include/` to your compiler’s include path.
2. **Optional out‑of‑line definitions.**  If code size is a concern, define `SSTL_EXTERN` in one translation unit before including the SSTL headers to emit function bodies into a single `.c` or `.cpp` file:

   ```cpp
   // sstl_translation_unit.cpp
   #define SSTL_EXTERN
   #include <sstl/vector.hpp>
   #include <sstl/string.hpp>
   // include other containers here
   ```

   Compile `sstl_translation_unit.cpp` and link it with your application; include the headers normally elsewhere without defining `SSTL_EXTERN`.

## Usage Examples

The following examples demonstrate basic usage of the `vector` container in both C++ and C.  Refer to the Doxygen comments in the headers for details on other containers (`string`, `map`, `list`, `deque`, etc.).

### C++ Example

```cpp
#include <sstl/vector.hpp>
#include <iostream>

int main() {
    // Define a statically‑sized vector of 8 integers
    sstl::vector<int, 8> v;

    // Push a few values; push_back returns false if the vector is full
    v.push_back(10);
    v.push_back(20);
    v.push_back(30);

    // Access elements using operator[], at() or iterators
    std::cout << "First element: " << v[0] << "\n";
    std::cout << "Last element:  " << v.back() << "\n";

    // Iterate using standard algorithms
    for (sstl::vector<int,8>::iterator it = v.begin(); it != v.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";

    // Check capacity and size
    std::cout << "Capacity: " << v.capacity() << ", size: " << v.size() << "\n";
    return 0;
}
```

### C Example

```c
#include <stdio.h>
#include <sstl/c/sstl_vector.h>

// Declare and define a vector type with capacity 8 for integers
SSTL_VECTOR_DECLARE(int_vec_t, int, 8);
SSTL_VECTOR_DEFINE(int_vec_t, int, 8);

int main(void) {
    int_vec_t v;
    int_vec_t_init(&v);        // initialise; sets size to 0

    // Push values; returns true on success, false if full
    int_vec_t_push_back(&v, 10);
    int_vec_t_push_back(&v, 20);
    int_vec_t_push_back(&v, 30);

    // Read values via at() (panics or returns NULL on out‑of‑bounds depending on policy)
    for (size_t i = 0; i < int_vec_t_size(&v); ++i) {
        int *p = int_vec_t_at(&v, i);
        printf("%d ", *p);
    }
    printf("\n");

    // Clean up
    int_vec_t_clear(&v);
    return 0;
}
```

The C API avoids exceptions and references; functions return a boolean status and use out‑parameters or pointers for results.  A corresponding set of `try_*` functions (e.g. `try_push_back`, `try_at`) always return a status indicator instead of calling the global panic handler.

### Other Containers and Algorithms

SSTL also provides:

* **`sstl::string<N>`** and `SSTL_STRING_DECLARE/DEFINE` for static strings with null‑termination and common string operations (`push_back`, `append`, `assign`, `find`, `substr`).
* **Node containers** such as `list`, `forward_list`, `queue`, `stack`, `priority_queue` and associative containers (`map`, `set`, `flat_map`, `flat_set`).  These embed a fixed pool of nodes and return `bool` from insert operations to indicate exhaustion of the node pool.
* **Algorithms** mirroring a subset of `<algorithm>` (e.g. `copy`, `fill`, `sort`, `find`, `accumulate`).  These are implemented as template functions in C++ or standalone functions in C; they operate on iterators or raw pointers.

### Error Policy

SSTL uses a compile‑time error policy selected with `SSTL_ON_ERROR`:

- `SSTL_PANIC` calls a user‑supplied panic handler such as `sstl_panic(const char *msg)`.
- `SSTL_RETURN` makes fallible operations return `bool`, `NULL`, or a status value.
- `SSTL_UB` contract-required caller checks may be omitted; violations are undefined behavior.

`try_*` functions always use return‑style behaviour regardless of the global policy.

## Testing and Helper Tools

The repository includes a comprehensive set of tests and helper scripts.  All helper scripts live in the **`tools/`** directory.  Each `sstl_*.py` script can be run as a command‑line tool from a terminal or, when double‑clicked in a file explorer, will attempt to launch a simple Tkinter GUI.  This dual usage allows you to integrate the tools into automated pipelines or use them interactively without requiring additional dependencies.

### Implementation Workspace

The workspace is organised as follows:

* **`include/sstl/`** — header‑only C++ implementation.
* **`include/sstl/c/`** — header‑only C implementation.
* **`testing/`** — C/C++ tests, support code, CMake configuration and test manifests.
* **`tools/`** — local helper scripts used for building, testing and documentation.  See below for details.
* **`artifacts/`** — generated output (bundles, CSV/JSON reports, runtime/coverage evidence, Doxygen output).  Generated files should be treated as disposable unless you are capturing them as validation evidence.
* **`documentation/`** — hand‑authored documentation inputs and older generated documentation snapshots.

### Helper Scripts

Run helpers from the workspace root with `python tools/<script>.py …`, or double‑click them in a file explorer to open the GUI.  The GUIs share common window sizing, button-state and double-click launch behavior, so they keep their controls visible and disable actions that cannot run with the currently available backend.  The auto‑detection logic attempts to find usable backends (Docker, Podman, WSL or local tools) and prompts for consent before performing any installation or environment setup.  In the test runner GUI, validation buttons and coverage buttons are separate aligned rows; coverage can run through auto fallback or a strict Docker, Podman, WSL or local backend selection.

- **`tools/sstl_run_tests.py`** — Compiles and runs the real C/C++ test suite.  It can use Docker, Podman, WSL or a local CMake/CTest toolchain.  The local validation backend is enabled only after a tiny host C/C++ CMake configure/build probe passes; local coverage additionally requires `lcov` and a GNU/Clang-style coverage probe.  The auto backend discovers available tools; installation flows ask before running installer commands, and the separate preparation flow can try to start or bootstrap installed backends.  Runtime CSV and coverage artefacts are copied into `artifacts/`.  Optional vendor compiler lanes can run host-installed tools or configured container images for ArmClang, GNU Arm Embedded, Keil/ArmCC and IAR/ICCARM probes.
- **`tools/sstl_run_doxygen.py`** — Generates Doxygen documentation from the embedded comments.  It purges the previous runner-owned Doxygen artifacts before each generation, then writes a fresh deterministic `artifacts/doxygen/Doxyfile` and HTML output under `artifacts/doxygen/html/`.  It tries the local Doxygen installation first and can optionally fall back to Docker, Podman or WSL when needed.  Backend-specific GUI buttons are disabled until that backend is reachable; `--install-missing` prints setup options and `--prepare-backends` can start Docker Desktop, start/init Podman or install Doxygen/Graphviz inside WSL after confirmation.
- **`tools/sstl_build_bundles.py`** — Generates and verifies all SSTL YAML bundles.  The produced bundles are saved in `artifacts/`.
- **`tools/sstl_get_code_stats.py`** — Reports line and character statistics, with and without comments and whitespace.  It writes `artifacts/sstl-code-stats.csv` and `artifacts/sstl-code-stats.json` by default.
- **`tools/sstl_clean_generated.py`** — Scans for generated `__pycache__`, `.pytest_cache`, `build/`, generated Doxygen documentation folders, and optionally the full `artifacts/` contents.  The default `all` group keeps the historical cache/build/Doxygen cleanup; use `--groups artifacts` to purge every immediate child under `artifacts/` while keeping the folder itself, or `--groups artifacts build` for artifacts plus build output.  You can supply `--yes` to delete without prompting; the GUI will also ask before removing anything.

### Common Commands

Here are a few useful commands to get you started.  These can be run from the repository root:

```bash
# Build and run all tests using the best available backend
python tools/sstl_run_tests.py --backend auto --runtime-report --quick-summary

# Run coverage‑only build via Docker
python tools/sstl_run_tests.py --backend docker --coverage-only

# Ask before installing missing runner backends/tooling
python tools/sstl_run_tests.py --backend auto --install-missing

# Non-interactive runner backend/tooling install flow
python tools/sstl_run_tests.py --backend auto --install-missing --yes-install

# Try to make installed but unavailable backends ready
python tools/sstl_run_tests.py --backend auto --prepare-backends
python tools/sstl_run_tests.py --backend podman --prepare-backends --yes-prepare
python tools/sstl_run_tests.py --backend wsl --prepare-backends --wsl-distro Ubuntu --yes-prepare

# Force a specific WSL distro instead of the machine default
python tools/sstl_run_tests.py --backend wsl --wsl-distro Ubuntu

# Run vendor compiler container probes with the backend's container engine
python tools/sstl_run_tests.py --backend docker --vendor-container-images
python tools/sstl_run_tests.py --backend podman --vendor-container-images

# Run the predefined public GNU Arm Embedded image nominees
python tools/sstl_run_tests.py --list-vendor-container-nominees
python tools/sstl_run_tests.py --vendor-container-nominees
python tools/sstl_run_tests.py --vendor-container-nominee wischner-arm-none-eabi-1_1_0

# Provide an explicit vendor compiler image
python tools/sstl_run_tests.py --vendor-container-images --vendor-container-image arm-none-eabi=my/arm-gcc:latest
python tools/sstl_run_tests.py --vendor-container-images --vendor-container-image arm-none-eabi@my_gcc=my/arm-gcc:latest

# Override the engine and skip image-backed lanes whose images are absent
python tools/sstl_run_tests.py --backend local --vendor-container-images --vendor-container-engine podman --vendor-container-pull never

# Generate Doxygen documentation
python tools/sstl_run_doxygen.py --backend auto
python tools/sstl_run_doxygen.py --backend auto --install-missing
python tools/sstl_run_doxygen.py --backend auto --prepare-backends
python tools/sstl_run_doxygen.py --backend wsl --wsl-distro Ubuntu --prepare-backends --yes-prepare

# Build and verify YAML bundles
python tools/sstl_build_bundles.py --verify

# Report code statistics
python tools/sstl_get_code_stats.py

# Clean generated files (dry run)
python tools/sstl_clean_generated.py --dry-run

# Selectively clean cache and build artefacts
python tools/sstl_clean_generated.py --groups cache build --dry-run

# Clean documentation artefacts (dry run)
python tools/sstl_clean_generated.py --groups documentation --dry-run

# Purge all generated artefacts, leaving the artifacts/ folder itself
python tools/sstl_clean_generated.py --groups artifacts --dry-run

# Purge all generated artefacts and build folders
python tools/sstl_clean_generated.py --groups artifacts build --dry-run
```

Vendor container image keys are `armclang`, `arm-none-eabi`, `keil-armcc` and `iar-iccarm`.  You can pass images with repeated `--vendor-container-image KEY=IMAGE` or `--vendor-container-image KEY@LABEL=IMAGE` arguments, or by setting `SSTL_VENDOR_CONTAINER_ARMCLANG_IMAGE`, `SSTL_VENDOR_CONTAINER_ARM_NONE_EABI_IMAGE`, `SSTL_VENDOR_CONTAINER_KEIL_ARMCC_IMAGE` and `SSTL_VENDOR_CONTAINER_IAR_ICCARM_IMAGE`.  `KEY@LABEL=IMAGE` is useful when validating multiple images for the same compiler key because each label gets its own lane names.

The predefined public `arm-none-eabi` nominees are available through `--vendor-container-nominees`, `--vendor-container-nominee NAME`, and `--list-vendor-container-nominees`: `wischner-arm-none-eabi-1_1_0`, `jafee-arm-none-eabi-15_2_rel1`, `jafee-arm-none-eabi-14_3_rel1`, and `gonzarub-arm-none-eabi-13_3`.

Missing vendor images are controlled by `--vendor-container-pull ask|never|always`.  The default is `ask`: the runner asks before pulling a configured missing image.  If the user denies the pull, approval is unavailable, or the pull fails because of network/registry issues, the dependent vendor image lanes are skipped and `testing/manifests/vendor_compiler_summary.yaml` records the reason.  `--install-missing` is separate; it only covers runner backends/tooling such as Docker, Podman, WSL, CMake and Ninja.

Backend preparation is also separate from installation and vendor image pulls.  Use `--prepare-backends` when Docker, Podman or WSL is installed but not currently usable.  It can start Docker Desktop and wait for `docker info`, initialize/start a Podman machine and wait for `podman info`, or run the WSL package setup command inside an Ubuntu/Debian-like distro.  The WSL preparation command installs both validation tools and the `lcov` coverage tool.  Use `--yes-prepare` only when those preparation actions are allowed non-interactively.  Host application installers still require `--install-missing` and, for non-interactive execution, `--yes-install`.

On Windows, WSL defaults vary by machine.  Docker Desktop, Podman and Rancher may register internal WSL distros that have `/bin/sh` but are not suitable for SSTL validation.  The runner only selects a non-internal validation distro that already has `cmake`, `ninja`, `gcc`, `g++`, `clang` and `clang++`; WSL coverage additionally requires `lcov`.  WSL coverage reports line/branch thresholds, but because branch counters vary noticeably across host distro compiler versions, the canonical threshold gate remains the Docker/local coverage lane.  Use `--wsl-distro NAME` or `SSTL_WSL_DISTRO=NAME` to force a known Ubuntu/Debian distro.  If the tools are missing, install them inside WSL with `sudo apt-get update && sudo apt-get install -y cmake ninja-build gcc g++ clang libclang-rt-dev util-linux python3 lcov`.

Docker and Podman discovery checks both the executable and the live engine connection.  Auto fallback skips a container backend when the daemon, Desktop app, VM or socket is not reachable.  For Podman on Windows, start Podman Desktop or run `podman machine init` followed by `podman machine start`, then verify with `podman info`.

Local backend discovery does more than check for `cmake` and `ctest` on `PATH`.  The runner checks that `testing/build/` can create and remove probe files, then configures and builds a tiny C99/C++98 CMake project in a disposable OS temp directory.  Local Coverage additionally requires `lcov` and verifies that the local compiler accepts GNU/Clang `--coverage` instrumentation flags.  This means Visual Studio can make Local validation available, while Local Coverage normally needs a GCC/Clang-style toolchain.

The runner avoids baking the local checkout name into generated commands.  Local, WSL and vendor-lane invocations pass repository-relative paths where the called tools support them, and WSL resolves the current checkout from its inherited working directory.  Docker and Podman still receive a runtime-derived host bind path for `.:/work` because container engines require a concrete host mount source, but the displayed command and generated vendor-lane summaries render that mount relative to the checkout.

## Doxygen Documentation

Every public header in SSTL contains Doxygen‑style comments.  To generate HTML documentation through the repository runner, run:

```sh
python tools/sstl_run_doxygen.py --backend auto
```

The runner writes a deterministic `artifacts/doxygen/Doxyfile`, purges the prior runner-owned outputs (`artifacts/doxygen/html`, `Doxyfile`, warning log and summary), then generates fresh HTML into `artifacts/doxygen/html/`.  It can use local Doxygen, Docker, Podman or a non-internal WSL distro.  Use `--install-missing` for setup recommendations and `--prepare-backends` when Docker/Podman/WSL is installed but not currently reachable or missing Doxygen tools.

## Contributing

Contributions are welcome!  This project is the result of generative AI collaboration, but human input remains essential for refining usability, adding features, improving tests and enhancing documentation.  Please follow the guidelines below when contributing:

1. **Discuss first.**  Open an issue or join the discussion to propose a change.  Major design decisions should align with the design document and interface specification; changes may require updating those documents and the corresponding tests.
2. **Test‑driven development.**  Whenever possible, add or update tests before changing the implementation.  New features should come with corresponding tests and documentation.
3. **Respect the static‑allocation model.**  All contributions must uphold the guarantee that no dynamic allocation occurs.  If you propose a feature that could allocate memory, ensure there is a compile‑time switch or that the allocation is user‑supplied.
4. **Keep portability in mind.**  Ensure your changes compile under both C99 and C++03 using the supported compilers and freestanding modes.  Avoid relying on hosted library functions or newer language features.

## License

This project is released under the MIT licence.  See the [LICENSE](LICENSE) file for details.
