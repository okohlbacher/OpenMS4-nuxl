# OpenMS NuXL

This standalone package owns **OpenNuXL**, its private search algorithms and its experiment presets. It consumes installed, pinned OpenMS Core and CLI SDKs. It does not configure Core, TOPP, Python, desktop or app source trees.

NuXL report serializers and their `NuXLMarkerIonExtractor` record/extraction dependency remain in Core, together with generic mzML, FASTA, idXML, mzTab and Parquet readers/writers. The private backend calls these SDK APIs; Core never links this package. No library, algorithm headers or Python module is installed by NuXL.

Build with CMake 3.24+, C++23, a matching Core/CLI SDK and the header-only `nlohmann_json` CMake package. Tests additionally require Core TestSupport; optional scientific fixture tests require the pinned OpenMSTestData package. Native dependencies must match the installed SDK's build profile.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_PREFIX_PATH='/path/to/sdk;/path/to/dependencies' \
  -DCMAKE_INSTALL_PREFIX=/path/to/nuxl \
  -DOPENMS4_REGRESSION_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
cmake --install build
```

The dependency lock enforces exact SDK revisions. Published builds also set `OPENMS4_REQUIRE_CLEAN_SOURCE=ON`; archives must assert their own source revision and dirty status explicitly, as required by the shared package helper. Reconfigure after changing source identity. Initial extraction pins are refreshed by the parent in dependency order after rebuilding Core/CLI.

The install contains `bin/OpenNuXL`, `share/openms4/tools/nuxl.tools.tsv`, and `share/openms4/nuxl/nuxl_presets.json`. Presets resolve relative to the executable, using the relative GNUInstallDirs layout; moving the complete prefix keeps that relationship intact. They no longer come from Core's `OPENMS_DATA_PATH`. `-NuXL:presets_file` selects a custom JSON file; an explicit missing file fails instead of silently falling back to defaults. Core's own chemistry/format runtime data is still required through its SDK.

Three existing class tests move with the backend unchanged apart from removing unused source-tree `test_config.h` includes. A fourth native test checks default/custom preset loading and missing-file rejection. The Core NuXLReport test retains all five mass/sequence cases and uses generic modification definitions as fixture input. The optional package scientific test searches the existing small OpenNuXL fixture and checks exact localized/unlocalized sequences plus the serialized adduct definition. Full score/TSV comparisons and fresh-process format round trips remain in the installed TestData suite, including its documented platform conditions.

Run the build-free ownership checks independently with:

```sh
python3 tests/test_package.py
```

These source checks are not native validation. Extraction-time source checks passed; compilation, native class tests, search fixtures and complete installed-prefix relocation are coordinated by the parent after the new SDK pins are available. See `migration.json` for every original path, original content hash, retained Core boundary and moved test registration.
