// Copyright (c) 2002-present, OpenMS Inc. -- EKU Tuebingen, ETH Zurich, and FU Berlin
// SPDX-License-Identifier: BSD-3-Clause
//
// --------------------------------------------------------------------------
// $Maintainer: Timo Sachsenberg $
// $Authors: OpenMS contributors $
// --------------------------------------------------------------------------

#include <OpenMS/ANALYSIS/NUXL/NuXLPresets.h>
#include <OpenMS/CONCEPT/ClassTest.h>
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
  NuXLPresets::getPresets("RNA-UV (U)", nucleotides, mapping, modifications, fragments, can_cross_link);
  TEST_STRING_EQUAL(can_cross_link, "U")
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
      << R"({"custom":{"target_nucleotides":["U=C9H13N2O9P"],"mapping":["U->U"],"modifications":["U:-H2O"],"fragment_adducts":["U:C3O;C3O"],"can_cross_link":"U"}})";
  }
  const StringList names = NuXLPresets::getAllPresetsNames(filename);
  TEST_EQUAL(names.size(), 1)
  ABORT_IF(names.empty())
  TEST_STRING_EQUAL(names.front(), "custom")
  StringList nucleotides, mapping, modifications, fragments;
  std::string can_cross_link;
  NuXLPresets::getPresets("custom", filename, nucleotides, mapping, modifications, fragments, can_cross_link);
  TEST_EQUAL(nucleotides.size(), 1)
  ABORT_IF(nucleotides.empty())
  TEST_STRING_EQUAL(nucleotides.front(), "U=C9H13N2O9P")
  TEST_STRING_EQUAL(can_cross_link, "U")
  TEST_EXCEPTION(std::runtime_error, NuXLPresets::getPresets("RNA-UV (U)", filename, nucleotides, mapping, modifications, fragments, can_cross_link))
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

END_TEST
