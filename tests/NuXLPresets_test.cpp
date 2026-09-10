// Copyright (c) 2002-present, OpenMS Inc. -- EKU Tuebingen, ETH Zurich, and FU Berlin
// SPDX-License-Identifier: BSD-3-Clause
//
// --------------------------------------------------------------------------
// $Maintainer: Timo Sachsenberg $
// $Authors: OpenMS contributors $
// --------------------------------------------------------------------------

#include <OpenMS/ANALYSIS/NUXL/NuXLPresets.h>
#include <OpenMS/CONCEPT/ClassTest.h>
#include <OpenMS/ANALYSIS/NUXL/NuXLParameterParsing.h>
#include <algorithm>
#include <fstream>
#include <stdexcept>

using namespace OpenMS;

START_TEST(NuXLPresets, "$Id$")

START_SECTION((package presets resolve relative to the executable))
{
  const StringList names = NuXLPresets::getAllPresetsNames();
  TEST_EQUAL(names.size(), 25)
  TEST_TRUE(std::find(names.begin(), names.end(), "RNA-UV (U)") != names.end())
  TEST_TRUE(std::find(names.begin(), names.end(), "none") != names.end())
  StringList nucleotides, mapping, modifications, fragments;
  std::string can_cross_link;
  bool default_marker_ions_RNA = false;
  NuXLPresets::getPresets("RNA-UV (U)", nucleotides, mapping, modifications, fragments, can_cross_link, default_marker_ions_RNA);
  TEST_STRING_EQUAL(can_cross_link, "U")
  TEST_TRUE(default_marker_ions_RNA)
  TEST_TRUE(! nucleotides.empty())
  TEST_TRUE(! fragments.empty())
}
END_SECTION

START_SECTION((an explicit custom preset file is authoritative))
{
  std::string filename;
  NEW_TMP_FILE(filename)
  {
    std::ofstream output(filename);
    output
      << R"({"custom":{"marker_ions":"RNA","target_nucleotides":["U=C9H13N2O9P"],"mapping":["U->U"],"modifications":["U:-H2O"],"fragment_adducts":["U:C3O;C3O"],"can_cross_link":"U"}})";
  }
  const StringList names = NuXLPresets::getAllPresetsNames(filename);
  TEST_EQUAL(names.size(), 1)
  ABORT_IF(names.empty())
  TEST_STRING_EQUAL(names.front(), "custom")
  StringList nucleotides, mapping, modifications, fragments;
  std::string can_cross_link;
  bool default_marker_ions_RNA = false;
  NuXLPresets::getPresets("custom", filename, nucleotides, mapping, modifications, fragments, can_cross_link, default_marker_ions_RNA);
  TEST_EQUAL(nucleotides.size(), 1)
  ABORT_IF(nucleotides.empty())
  TEST_STRING_EQUAL(nucleotides.front(), "U=C9H13N2O9P")
  TEST_STRING_EQUAL(can_cross_link, "U")
  TEST_TRUE(default_marker_ions_RNA)
  TEST_EXCEPTION(std::runtime_error, NuXLPresets::getPresets("RNA-UV (U)", filename, nucleotides, mapping, modifications, fragments, can_cross_link, default_marker_ions_RNA))
}
END_SECTION

START_SECTION((a missing explicit file must not fall back to installed presets))
{
  std::string filename;
  NEW_TMP_FILE(filename)
  bool rejected = false;
  try
  {
    NuXLPresets::getAllPresetsNames(filename);
  }
  catch (const std::runtime_error& error)
  {
    rejected = true;
    TEST_STRING_EQUAL(error.what(), "NuXL presets file not found: " + filename)
  }
  TEST_TRUE(rejected)
}
END_SECTION


START_SECTION((explicit marker chemistry determines the complete default ion set independently of the preset name))
{
  std::string filename;
  NEW_TMP_FILE(filename)
  const StringList rna_formulas = {"C9H13N2O9P", "C9H14N3O8P", "C10H14N5O8P", "C10H14N5O7P",
    "C4H4N2O2", "C4H5N3O", "C5H5N5O", "C5H5N5"};
  const StringList dna_formulas = {"C10H15N2O8P", "C9H14N3O7P", "C10H14N5O7P", "C10H14N5O6P",
    "C5H6N2O2", "C4H5N3O", "C5H5N5O", "C5H5N5"};
  for (const std::string chemistry : {"RNA", "DNA"})
  {
    {
      std::ofstream output(filename);
      output << R"({"neutral-key":{"marker_ions":")" << chemistry
        << R"(","target_nucleotides":[],"mapping":[],"modifications":[],"fragment_adducts":[],"can_cross_link":""}})";
    }
    StringList nucleotides, mapping, modifications, fragments;
    std::string can_cross_link;
    bool default_marker_ions_RNA = chemistry != "RNA";
    NuXLPresets::getPresets("neutral-key", filename, nucleotides, mapping, modifications, fragments, can_cross_link, default_marker_ions_RNA);
    TEST_EQUAL(default_marker_ions_RNA, chemistry == "RNA")
    // No explicit adducts: these ions must come from the selected default set.
    const auto adducts = NuXLParameterParsing::getFeasibleFragmentAdducts(
      "U", "C9H13N2O9P", {}, {}, true, default_marker_ions_RNA);
    const auto& expected = chemistry == "RNA" ? rna_formulas : dna_formulas;
    TEST_EQUAL(adducts.marker_ions.size(), expected.size())
    for (const auto& formula : expected)
    {
      const EmpiricalFormula empirical_formula(formula);
      const auto ion = std::find_if(adducts.marker_ions.begin(), adducts.marker_ions.end(),
        [&](const auto& candidate) { return candidate.formula == empirical_formula; });
      TEST_TRUE(ion != adducts.marker_ions.end())
      ABORT_IF(ion == adducts.marker_ions.end())
      TEST_REAL_SIMILAR(ion->mass, empirical_formula.getMonoWeight())
    }
  }
}
END_SECTION

START_SECTION((only exact bundled names permit legacy missing metadata and explicit invalid values are rejected))
{
  std::string filename;
  NEW_TMP_FILE(filename)
  {
    std::ofstream output(filename);
    output << R"json({"RNA-UV (U)":{},"DNA-UV":{},"RNA-custom":{},"bad":{"marker_ions":"unknown"},"null":{"marker_ions":null}})json";
  }
  StringList nucleotides, mapping, modifications, fragments;
  std::string can_cross_link;
  bool default_marker_ions_RNA = false;
  NuXLPresets::getPresets("RNA-UV (U)", filename, nucleotides, mapping, modifications, fragments, can_cross_link, default_marker_ions_RNA);
  TEST_TRUE(default_marker_ions_RNA)
  NuXLPresets::getPresets("DNA-UV", filename, nucleotides, mapping, modifications, fragments, can_cross_link, default_marker_ions_RNA);
  TEST_EQUAL(default_marker_ions_RNA, false)
  for (const std::string name : {"RNA-custom", "bad", "null"})
  {
    bool rejected = false;
    try
    {
      NuXLPresets::getPresets(name, filename, nucleotides, mapping, modifications, fragments, can_cross_link, default_marker_ions_RNA);
    }
    catch (const std::runtime_error& error)
    {
      rejected = true;
      TEST_STRING_EQUAL(error.what(), "Error reading presets: Preset '" + name + "' requires marker_ions set to 'RNA' or 'DNA'.")
    }
    TEST_TRUE(rejected)
  }
}
END_SECTION

END_TEST
