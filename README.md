# Static‑Allocation STL‑Like Library (SSTL)

SSTL is a lightweight alternative to the C++ Standard Template Library for **C99** and **C++03** environments.  It provides a family of familiar containers, algorithms and utilities that operate entirely on **statically allocated memory**—no calls to `malloc`, `free`, `new` or `delete` are made at runtime.  This makes SSTL suitable for bare‑metal microcontrollers, safety‑critical systems and any environment where heap allocation is undesirable or unavailable.

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

Run helpers from the workspace root with `python tools/<script>.py …`, or double‑click them in a file explorer to open the GUI.  The auto‑detection logic attempts to find usable backends (Docker, Podman, WSL or local toolchains) and prompts for consent before performing any installation or environment setup.

- **`tools/sstl_run_tests.py`** — Compiles and runs the real C/C++ test suite.  It can use Docker, Podman, WSL or a local CMake/CTest toolchain.  The auto backend discovers available tools; installation flows ask before running installer commands.  Runtime CSV and coverage artefacts are copied into `artifacts/`.
- **`tools/sstl_run_doxygen.py`** — Generates Doxygen documentation from the embedded comments.  It tries the local Doxygen installation first and can optionally fall back to Docker, Podman or WSL when needed.  Output goes to `artifacts/doxygen/`.
- **`tools/sstl_build_bundles.py`** — Generates and verifies all SSTL YAML bundles.  The produced bundles are saved in `artifacts/`.
- **`tools/sstl_get_code_stats.py`** — Reports line and character statistics, with and without comments and whitespace.  It writes `artifacts/sstl-code-stats.csv` and `artifacts/sstl-code-stats.json` by default.
- **`tools/sstl_clean_generated.py`** — Scans for generated `__pycache__`, `.pytest_cache`, `build/` and generated Doxygen documentation folders.  You can supply `--yes` to delete without prompting; the GUI will also ask before removing anything.

### Common Commands

Here are a few useful commands to get you started.  These can be run from the repository root:

```bash
# Build and run all tests using the best available backend
python tools/sstl_run_tests.py --backend auto --runtime-report --quick-summary

# Run coverage‑only build via Docker
python tools/sstl_run_tests.py --backend docker --coverage-only

# Generate Doxygen documentation
python tools/sstl_run_doxygen.py --backend auto

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
```

## Doxygen Documentation

Every public header in SSTL contains Doxygen‑style comments.  To generate HTML documentation, install [Doxygen](https://www.doxygen.nl/) and run:

```sh
doxygen Doxyfile
```

If you do not have a `Doxyfile`, you can create a minimal one by running `doxygen -g` and editing the `INPUT` and `FILE_PATTERNS` fields to point at `include/sstl/` and `include/sstl/c/`.  Some users prefer to automate this step; you can invoke Doxygen from the provided `tools/sstl_run_doxygen.py` script, which will locate or install Doxygen and place the output in `artifacts/doxygen/`.

## Contributing

Contributions are welcome!  This project is the result of generative AI collaboration, but human input remains essential for refining usability, adding features, improving tests and enhancing documentation.  Please follow the guidelines below when contributing:

1. **Discuss first.**  Open an issue or join the discussion to propose a change.  Major design decisions should align with the design document and interface specification; changes may require updating those documents and the corresponding tests.
2. **Test‑driven development.**  Whenever possible, add or update tests before changing the implementation.  New features should come with corresponding tests and documentation.
3. **Respect the static‑allocation model.**  All contributions must uphold the guarantee that no dynamic allocation occurs.  If you propose a feature that could allocate memory, ensure there is a compile‑time switch or that the allocation is user‑supplied.
4. **Keep portability in mind.**  Ensure your changes compile under both C99 and C++03 using the supported compilers and freestanding modes.  Avoid relying on hosted library functions or newer language features.

## License

This project is released under the MIT licence.  See the [LICENSE](LICENSE) file for details.
