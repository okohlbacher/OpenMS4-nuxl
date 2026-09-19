// Copyright (c) 2002-present, The OpenMS Team -- EKU Tuebingen, ETH Zurich, and FU Berlin
// SPDX-License-Identifier: BSD-3-Clause
//
// --------------------------------------------------------------------------
// $Maintainer: Timo Sachsenberg $
// $Authors: Timo Sachsenberg $
// --------------------------------------------------------------------------

#include <OpenMS/ANALYSIS/NUXL/NuXLPresets.h>
#include <OpenMS/SYSTEM/File.h>
#include <OpenMS/SYSTEM/PathUtils.h>
#include <NuXLDataPath.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>

using json = nlohmann::json;

namespace OpenMS
{
  namespace NuXLPresets
  {
    namespace
    {
      std::string presetsPath(const std::string& custom_presets_file)
      {
        const std::string executable_directory = File::getExecutablePath();
        if (custom_presets_file.empty() && executable_directory.empty())
        {
          throw std::runtime_error("Cannot locate the NuXL executable directory for presets");
        }
        std::filesystem::path base = OpenMS::to_path(executable_directory);
        // Package managers expose the tool through a symlink in their own bin
        // directory, and macOS reports the path as invoked rather than the real
        // one, so the presets would be searched beside the link. Resolve the
        // executable itself before applying the relative install layout.
        std::error_code link_error;
        const auto resolved = std::filesystem::canonical(base / "OpenNuXL", link_error);
        if (!link_error)
        {
          base = resolved.parent_path();
        }
        const std::filesystem::path path = custom_presets_file.empty()
          ? base / NUXL_DATA_FROM_EXECUTABLE / "nuxl_presets.json"
          : OpenMS::to_path(custom_presets_file);
        const auto utf8 = path.lexically_normal().generic_u8string();
        const std::string filename(reinterpret_cast<const char*>(utf8.data()), utf8.size());
        if (!std::filesystem::is_regular_file(path))
        {
          throw std::runtime_error("NuXL presets file not found: " + filename);
        }
        return filename;
      }
    }

    StringList getAllPresetsNames(const std::string& custom_presets_file)
    {
      StringList presets;
      
      // Determine which JSON file to use
      const std::string json_path = presetsPath(custom_presets_file);
      
      if (File::exists(json_path))
      {
        OPENMS_LOG_INFO << "Found presets file: " << json_path << std::endl;
      }      

      if (File::exists(json_path))
      {
        try
        {
          std::ifstream file{OpenMS::to_path(json_path)};
          json j;
          file >> j;
          
          // Add all presets to the list
          for (auto it = j.begin(); it != j.end(); ++it)
          {
            presets.push_back(it.key());
          }
        }
        catch (const std::exception& e)
        {
          OPENMS_LOG_WARN << "Error reading presets from " << json_path << ": " << e.what() << std::endl;
        }
      }
      else
      {
        OPENMS_LOG_WARN << "Presets file not found: " << json_path << std::endl;
      }
      return presets;
    }
    
    void getPresets(const std::string& p,
      const std::string& custom_presets_file,
      StringList& nucleotides, 
      StringList& mapping, 
      StringList& modifications, 
      StringList& fragment_adducts, 
      std::string& can_cross_link,
      bool& default_marker_ions_RNA)
    {
      StringList presets = getAllPresetsNames(custom_presets_file);
      OPENMS_LOG_INFO << "Found presets: " << presets.size() << std::endl;
      for (const std::string& s : presets)
      {
        OPENMS_LOG_DEBUG << s << std::endl;
      }
      // Check if preset exists
      bool found = find(presets.begin(), presets.end(), p) != presets.end();
      if (!found)
      {
        throw std::runtime_error("Error: unknown preset '" + p + "'.");
      }

      // Try to load presets from JSON file
      const std::string json_path = presetsPath(custom_presets_file);
      
      if (File::exists(json_path))
      {
        try
        {
          std::ifstream file{OpenMS::to_path(json_path)};
          json j;
          file >> j;
          
          // Check if the requested preset exists in the JSON file
          if (j.contains(p.c_str()))
          {
            const auto& preset = j[p.c_str()];

            // Preset names are display labels, not a reliable chemical contract.
            // Preserve old custom files only for an exact bundled preset key.
            json marker_ions = preset.value("marker_ions", json());
            if (marker_ions.is_null() && !custom_presets_file.empty() && !preset.contains("marker_ions"))
            {
              std::ifstream bundled_file{OpenMS::to_path(presetsPath(""))};
              json bundled;
              bundled_file >> bundled;
              if (bundled.contains(p))
              {
                marker_ions = bundled.at(p).value("marker_ions", json());
              }
            }
            if (!marker_ions.is_string() || (marker_ions != "RNA" && marker_ions != "DNA"))
            {
              throw std::runtime_error("Preset '" + p + "' requires marker_ions set to 'RNA' or 'DNA'.");
            }
            default_marker_ions_RNA = marker_ions == "RNA";
            
            // Load nucleotides
            if (preset.contains("target_nucleotides"))
            {
              nucleotides.clear();
              for (const auto& nuc : preset["target_nucleotides"])
              {
                nucleotides.push_back(nuc.get<std::string>());
              }
            }
            
            // Load mapping
            if (preset.contains("mapping"))
            {
              mapping.clear();
              for (const auto& map : preset["mapping"])
              {
                mapping.push_back(map.get<std::string>());
              }
            }
            
            // Load modifications
            if (preset.contains("modifications"))
            {
              modifications.clear();
              for (const auto& mod : preset["modifications"])
              {
                modifications.push_back(mod.get<std::string>());
              }
            }
            
            // Load fragment adducts
            if (preset.contains("fragment_adducts"))
            {
              fragment_adducts.clear();
              for (const auto& frag : preset["fragment_adducts"])
              {
                fragment_adducts.push_back(frag.get<std::string>());
              }
            }
            
            // Load can_cross_link
            if (preset.contains("can_cross_link"))
            {
              can_cross_link = preset["can_cross_link"].get<std::string>();
            }
            
            // Special handling for DEB and NM presets that need methionine loss
            if (StringUtils::hasSubstring(p, "DEB") || StringUtils::hasSubstring(p, "NM"))
            {
              // add special methionine loss
              auto r_ptr = const_cast<Residue*>(ResidueDB::getInstance()->getResidue('M'));
              r_ptr->addLossFormula(EmpiricalFormula("CH4S1"));
            }
            
            // Preset loaded successfully, return
            OPENMS_LOG_INFO << "Using preset '" << p << "' from " << json_path << std::endl;
            return;
          }
        }
        catch (const std::exception& e)
        {
          // If there's an error reading the JSON file, throw an error
          OPENMS_LOG_WARN << "Error reading presets from " << json_path << ": " << e.what() << std::endl;
          throw std::runtime_error(std::string("Error reading presets: ") + e.what());
        }
      }
      else
      {
        throw std::runtime_error("Error: presets file not found.");
      }
    }
    
    // Overload that uses the default presets file
    void getPresets(const std::string& p, 
      StringList& nucleotides, 
      StringList& mapping, 
      StringList& modifications, 
      StringList& fragment_adducts, 
      std::string& can_cross_link,
      bool& default_marker_ions_RNA)
    {
      getPresets(p, "", nucleotides, mapping, modifications, fragment_adducts, can_cross_link, default_marker_ions_RNA);
    }
  }
}
