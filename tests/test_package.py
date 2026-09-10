#!/usr/bin/env python3
"""Build-free package ownership checks; native/scientific CTest remains required."""
import json
from pathlib import Path
import re
import unittest

PACKAGE = Path(__file__).resolve().parents[1]


class PackageContract(unittest.TestCase):
    def test_manifest_matches_sources_and_tests(self):
        migration = json.loads((PACKAGE / 'migration.json').read_text())
        moved = migration['moved']
        self.assertEqual(len([item for item in moved if item['new_path'].startswith('include/')]), 11)
        self.assertEqual(len([item for item in moved if item['new_path'].startswith('source/')]), 9)
        for item in moved + migration['copied_pending_original_removal'] + migration['runtime_resources']:
            self.assertTrue((PACKAGE / item['new_path']).is_file(), item['new_path'])
        cmake = (PACKAGE / 'CMakeLists.txt').read_text()
        for source in (PACKAGE / 'source').glob('*.cpp'):
            self.assertIn('source/' + source.name, cmake)
        for name in migration['class_tests_moved']:
            self.assertIn(name.removesuffix('_test'), cmake)
            self.assertNotIn('OpenMS/test_config.h', (PACKAGE / 'tests' / (name + '.cpp')).read_text())
        self.assertEqual(migration['class_tests_moved'],
                         ['NuXLFragmentAdductDefinition_test', 'NuXLModificationsGenerator_test', 'NuXLParameterParsing_test'])

    def test_only_tool_and_data_are_installed(self):
        cmake = (PACKAGE / 'CMakeLists.txt').read_text()
        installed_targets = re.findall(r'install\(TARGETS\s+(\w+)', cmake)
        self.assertEqual(installed_targets, [])
        self.assertIn('openms4_install_tools(nuxl "${CMAKE_INSTALL_BINDIR}" ${package_tools})', cmake)
        helper = (PACKAGE / 'cmake/OpenMS4Tools.cmake').read_text()
        self.assertIn('install(TARGETS ${tool} RUNTIME DESTINATION', helper)
        self.assertNotIn('install(DIRECTORY include', cmake)
        self.assertIn('add_library(nuxl_backend STATIC', cmake)
        for header in (PACKAGE / 'include').rglob('*.h'):
            self.assertNotIn('OPENMS_DLLAPI', header.read_text(), header)
        self.assertFalse((PACKAGE / 'source/NuXLReport.cpp').exists())
        self.assertFalse((PACKAGE / 'source/NuXLMarkerIonExtractor.cpp').exists())
        self.assertNotIn('../core', cmake)
        self.assertNotIn('add_subdirectory', cmake)
        tool = json.loads((PACKAGE / 'tools.json').read_text())
        self.assertEqual(tool['package'], 'OpenMSNuXL')
        self.assertEqual([entry['name'] for entry in tool['tools']], ['OpenNuXL'])

    def test_package_presets_and_private_include_closure(self):
        presets = json.loads((PACKAGE / 'share/openms4/nuxl/nuxl_presets.json').read_text())
        self.assertEqual(len(presets), 25)
        self.assertEqual(presets['RNA-UV (U)']['can_cross_link'], 'U')
        self.assertEqual(presets['none']['target_nucleotides'], [])
        for name, preset in presets.items():
            self.assertEqual(preset['marker_ions'], 'RNA' if name.startswith('RNA-') else 'DNA')
        source = (PACKAGE / 'source/NuXLPresets.cpp').read_text()
        self.assertNotIn('getOpenMSDataPath', source)
        self.assertIn('File::getExecutablePath()', source)
        self.assertIn('NuXL presets file not found:', source)
        retained = {'NuXLReport.h', 'NuXLMarkerIonExtractor.h'}
        for directory in ['include', 'source', 'src', 'tests']:
            for path in (PACKAGE / directory).rglob('*'):
                if path.suffix in {'.cpp', '.h'}:
                    for name in re.findall(r'#include\s*<OpenMS/ANALYSIS/NUXL/([^>]+)>', path.read_text()):
                        self.assertTrue(name in retained or (PACKAGE / 'include/OpenMS/ANALYSIS/NUXL' / name).is_file(), (path, name))


if __name__ == '__main__':
    unittest.main(verbosity=2)
