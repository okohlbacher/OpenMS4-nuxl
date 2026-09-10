# Changes

## 1.0.0 experimental extraction

- Move OpenNuXL and its private search backend into an installed-SDK consumer package; retain NuXL report serialization and all generic file formats in Core.
- Move the 25 experiment presets and resolve them relative to the NuXL executable. Explicit missing custom files now fail instead of using defaults.
- Preserve three algorithm class tests and add native preset discovery/override tests plus the installed-data localized-adduct regression.

The extraction is an intentional experimental C++ API ownership change. The moved backend is no longer exported by Core; no existing Python API is removed by this package.
