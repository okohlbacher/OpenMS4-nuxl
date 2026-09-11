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

Three existing class tests move with the backend unchanged apart from removing unused source-tree `test_config.h` includes. A fourth native test checks default/custom preset loading and missing-file rejection. The Core NuXLReport test retains all five mass/sequence cases and uses generic modification definitions as fixture input. The optional package scientific tests search the existing small OpenNuXL fixture with default parameters and a new custom preset name, checking exact localized/unlocalized sequences plus the serialized adduct definition. A missing custom name must return exit 6 with its specific diagnostic. Names are validated after selecting the actual preset file, so custom names are not limited to the bundled list. New custom keys must specify `marker_ions: "RNA"` or `"DNA"`; exact bundled keys in legacy custom files inherit the corresponding bundled setting when the field is absent. Names no longer determine default marker chemistry. Native tests check both complete eight-ion sets and their masses, missing/invalid metadata rejection, and a scientific custom search whose key is `custom-protocol`. See the preset [schema and inherited methionine-loss limitation](share/openms4/nuxl/README.md). Full score/TSV comparisons and fresh-process format round trips remain in the installed TestData suite, including its documented platform conditions.

## Continuous integration, releases and Homebrew

`.github/workflows/nuxl.yml` builds, tests, installs and packages OpenNuXL on
Linux x64/arm64, macOS x64/arm64 and Windows x64, and builds the two macOS cask
payloads. Every job downloads the pinned Core SDK release, verifies its checksum,
then builds the pinned CLI from source and installs it beside Core; the native
jobs additionally install the pinned OpenMSTestData package and run the scientific
fixture tests with `OPENMS4_REGRESSION_TESTS=ON`. `tools/ci/run.py` and
`tools/ci/run_homebrew.py` carry the exact commands, record every command with its
runtime in `results/commands.json`, and check that the executable, the tool
manifest and the presets are actually installed before packaging.

Both dependencies are public, so CI needs no credentials: the pinned CLI and
TestData packages are ordinary checkouts, and the scientific fixture tests run in
every platform job.

Pushing a `nuxl-v*` tag runs `.github/workflows/release.yml`, which refuses to
publish unless a successful branch CI run exists for that exact revision, verifies
all seven checksums, and then republishes those tested archives as a prerelease.

On macOS the tool installs from the repository tap:

```sh
brew trust --formula okohlbacher/openms4-core/openms4-core
brew trust --cask okohlbacher/openms4-nuxl/openms4-nuxl
brew tap okohlbacher/openms4-core https://github.com/okohlbacher/OpenMS4-core
brew tap okohlbacher/openms4-nuxl https://github.com/okohlbacher/OpenMS4-nuxl
brew install --cask okohlbacher/openms4-nuxl/openms4-nuxl
```

Trust both packages before tapping: recent Homebrew refuses to read an untrusted
tap, so a `brew tap` that precedes its trust step fails outright. The cask payload
carries the CLI runtime, the presets and the tool manifest, and depends on the
`openms4-core` formula for the SDK itself. `brew` downloads release assets
anonymously, so cask installs require the release assets of this repository to be
publicly readable.

`Casks/openms4-nuxl.rb` is generated from a published release rather than written
by hand, so its checksums always describe assets that exist:

```sh
python3 tools/ci/update_cask.py --tag nuxl-v1.0.0-ci.1
```

Commit the generated cask; `.github/workflows/homebrew-cask.yml` then installs it
on both macOS architectures, runs `OpenNuXL --help` and `-write_ini`, and
uninstalls it again.

Run the build-free ownership checks independently with:

```sh
python3 tests/test_package.py
```

These source checks are not native validation. Extraction-time source checks passed; compilation, native class tests, search fixtures and complete installed-prefix relocation are coordinated by the parent after the new SDK pins are available. See `migration.json` for every original path, original content hash, retained Core boundary and moved test registration.
