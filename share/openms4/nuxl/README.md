# NuXL Custom Presets

This directory contains custom presets for the OpenNuXL tool. These presets define the nucleotide configurations and fragment adducts used in the cross-linking analysis.

## JSON Format

The JSON file should contain a dictionary where each key is a preset name and each value is a dictionary with the following keys:

- `marker_ions`: Explicit default marker-ion set, exactly `"RNA"` or `"DNA"`. Preset names do not select this chemistry.
- `target_nucleotides`: List of nucleotides and their empirical formulas (e.g., `"U=C9H13N2O9P"`)
- `mapping`: List of nucleotide mappings (e.g., `"U->U"`)
- `can_cross_link`: String of nucleotides that can form cross-links (e.g., `"U"` for RNA or `"T"` for DNA)
- `modifications`: List of modifications that can be applied to nucleotides (e.g., `"U:"`, `"U:-H2O"`)
- `fragment_adducts`: List of fragment adducts that can be generated from nucleotides (e.g., `"U:C9H10N2O5;U-H3PO4"`)

## Example

See the default `nuxl_presets.json` in the share folder for all presets for RNA and DNA.
## Existing custom files

An existing custom file whose preset key exactly matches a bundled key may omit `marker_ions`; it inherits that bundled key's setting. Every new key must declare the field. Invalid values (including null) fail; they never select DNA implicitly. For example, `"my-protocol": {"marker_ions": "RNA", ...}` selects RNA markers without a naming convention. Explicit `NuXL:presets=none` parameter handling is unchanged.

An inherited limitation remains: DEB/NM names trigger a global methionine neutral-loss mutation, and displaying bundled presets during option registration loads those entries. This behavior predates package extraction and is unchanged here; marker-ion metadata does not fix or replace that separate chemistry behavior.
