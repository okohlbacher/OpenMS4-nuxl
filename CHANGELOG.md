# Changes

## 1.0.0 experimental extraction

- Move OpenNuXL and its private search backend into an installed-SDK consumer package; retain NuXL report serialization and all generic file formats in Core.
- Move the 25 experiment presets and resolve them relative to the NuXL executable. Explicit missing custom files now fail instead of using defaults.
- Preserve three algorithm class tests and add native preset discovery/override tests plus the installed-data localized-adduct regression.
- Accept names introduced by a custom preset file after that file is selected; report unknown names with the standard illegal-parameters exit status. Retain scientific CLI tests for custom-name search and exact adduct localization.

- Select RNA/DNA default marker ions from explicit validated preset metadata, preserving every bundled setting and legacy exact bundled keys. New custom keys without metadata fail clearly; a neutral-name RNA scientific fixture and exact RNA/DNA ion tests cover the previous silent DNA fallback.

The extraction is an intentional experimental C++ API ownership change. The moved backend is no longer exported by Core; no existing Python API is removed by this package.
