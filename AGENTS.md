# NuXL package notes

Own OpenNuXL, the private static search backend, experiment presets and their tests. Consume installed pinned OpenMS Core/CLI SDKs; never add sibling source/build includes. Report and file-format readers/writers remain in Core. Keep the backend uninstalled and preserve existing scientific tests and fixtures.

Coordinate native builds with the parent task. Do not alter dependencies/vendors or weaken scientific comparisons. Run `python3 tests/test_package.py` for build-free ownership checks and all package CTest tests after native changes. Record source checks separately from native acceptance. Use Debug, C++23, existing OpenMS style and `[BUILD,FIX,TEST]`-style commit tags. No commits or pushes until coordinated.

Presets belong to `share/openms4/nuxl`, resolved relative to the executable; custom preset files are explicit and authoritative. Keep this independent of Core's runtime data path. A deployable prefix also requires matching Core/CLI runtime libraries and Core data.
