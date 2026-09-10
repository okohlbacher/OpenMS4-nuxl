cmake_minimum_required(VERSION 3.24)
project(OpenMSNuXL VERSION 1.0.0 LANGUAGES CXX)
include(GNUInstallDirs)
include(CTest)
include(cmake/OpenMS4Dependencies.cmake)
if(BUILD_TESTING)
  openms4_find_core(COMPONENTS TestSupport)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
else()
  openms4_find_core()
endif()
openms4_find_package(OpenMSCLI)
find_package(nlohmann_json REQUIRED CONFIG)
include(cmake/OpenMS4Tools.cmake)
openms4_load_tools(package_tools)

# Keep the same executable/data layout in the build tree and a relocated install.
if(IS_ABSOLUTE "${CMAKE_INSTALL_BINDIR}" OR IS_ABSOLUTE "${CMAKE_INSTALL_DATADIR}")
  message(FATAL_ERROR "NuXL requires relative GNUInstallDirs paths for relocation")
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/$<0:>")
file(RELATIVE_PATH NUXL_DATA_FROM_EXECUTABLE
  "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}"
  "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/openms4/nuxl")
configure_file(cmake/NuXLDataPath.h.in generated/NuXLDataPath.h @ONLY)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/share/openms4/nuxl/nuxl_presets.json")
file(COPY share/openms4/nuxl/ DESTINATION "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_DATADIR}/openms4/nuxl")
install(DIRECTORY share/openms4/nuxl/ DESTINATION "${CMAKE_INSTALL_DATADIR}/openms4/nuxl")

# A private static target shares the production objects with the preserved tests.
# No algorithm headers, library, or CMake developer package are installed.
add_library(nuxl_backend STATIC
  source/NuXLAnnotateAndLocate.cpp
  source/NuXLDeisotoper.cpp
  source/NuXLFDR.cpp
  source/NuXLFragmentAdductDefinition.cpp
  source/NuXLFragmentAnnotationHelper.cpp
  source/NuXLFragmentIonGenerator.cpp
  source/NuXLModificationsGenerator.cpp
  source/NuXLParameterParsing.cpp
  source/NuXLPresets.cpp)
target_compile_features(nuxl_backend PUBLIC cxx_std_23)
target_include_directories(nuxl_backend PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
  PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(nuxl_backend PUBLIC OpenMS::Core PRIVATE nlohmann_json::nlohmann_json)

add_executable(OpenNuXL src/OpenNuXL.cpp)
target_link_libraries(OpenNuXL PRIVATE nuxl_backend OpenMS::CLI Boost::regex)
openms4_install_rpath(OpenNuXL)
install(TARGETS OpenNuXL RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
set(manifest "# name\tcategory\tversion\texecutable\nOpenNuXL\t${OpenNuXL_CATEGORY}\t${PROJECT_VERSION}\t${CMAKE_INSTALL_BINDIR}/OpenNuXL${CMAKE_EXECUTABLE_SUFFIX}\n")
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/share/openms4/tools)
file(WRITE ${CMAKE_BINARY_DIR}/share/openms4/tools/nuxl.tools.tsv "${manifest}")
install(FILES ${CMAKE_BINARY_DIR}/share/openms4/tools/nuxl.tools.tsv DESTINATION share/openms4/tools)

if(BUILD_TESTING)
  foreach(kind ini ctd)
    add_test(NAME OpenNuXL_write_${kind} COMMAND ${Python3_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckToolMetadata.py $<TARGET_FILE:OpenNuXL>
      OpenNuXL ${PROJECT_VERSION} "${OpenNuXL_CATEGORY}" ${kind}
      ${CMAKE_CURRENT_BINARY_DIR}/metadata/OpenNuXL.${kind})
    set_tests_properties(OpenNuXL_write_${kind} PROPERTIES
      ENVIRONMENT "OPENMS_TOOL_PREFIX_PATH=${CMAKE_BINARY_DIR};OPENMS_DISABLE_UPDATE_CHECK=ON")
  endforeach()
  foreach(test NuXLFragmentAdductDefinition NuXLModificationsGenerator NuXLParameterParsing NuXLPresets)
    add_executable(${test}_test tests/${test}_test.cpp ${OpenMS_TEST_SUPPORT_SOURCE})
    target_link_libraries(${test}_test PRIVATE nuxl_backend OpenMS::TestFramework)
    openms4_install_rpath(${test}_test)
    add_test(NAME ${test}_test COMMAND ${test}_test)
  endforeach()
  add_test(NAME NuXL_package_source_contract COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_package.py)
endif()

option(OPENMS4_REGRESSION_TESTS "Enable numerical tests using installed OpenMSTestData" OFF)
if(BUILD_TESTING AND OPENMS4_REGRESSION_TESTS)
  openms4_find_package(OpenMSTestData)
  # Check scientific identity/localization on every platform. The full suite's
  # score/byte comparisons retain their existing platform qualifications.
  add_test(NAME NuXL_search_fixture COMMAND OpenNuXL -test -threads 2
    -precursor:mass_tolerance 10.0
    -in ${OpenMSTestData_TOPP_DIR}/OpenNuXL_1_input.mzML
    -database ${OpenMSTestData_TOPP_DIR}/OpenNuXL_1_input.fasta
    -out ${CMAKE_CURRENT_BINARY_DIR}/NuXL_fixture.idXML
    -out_tsv ${CMAKE_CURRENT_BINARY_DIR}/NuXL_fixture.tsv
    -NuXL:decoys false)
  add_test(NAME NuXL_search_localized_adducts COMMAND ${CMAKE_COMMAND}
    -DINPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/NuXL_fixture.idXML
    -P ${OpenMSTestData_TOPP_DIR}/OpenNuXL_adduct_in_sequence_test.cmake)
  set_tests_properties(NuXL_search_localized_adducts PROPERTIES DEPENDS NuXL_search_fixture)
  set_tests_properties(NuXL_search_fixture PROPERTIES
    ENVIRONMENT "OPENMS_TOOL_PREFIX_PATH=${CMAKE_BINARY_DIR};OPENMS_DISABLE_UPDATE_CHECK=ON")
  add_test(NAME NuXL_custom_preset_search COMMAND OpenNuXL -test -threads 1
    -precursor:mass_tolerance 10.0
    -in ${OpenMSTestData_TOPP_DIR}/OpenNuXL_1_input.mzML
    -database ${OpenMSTestData_TOPP_DIR}/OpenNuXL_1_input.fasta
    -out ${CMAKE_CURRENT_BINARY_DIR}/NuXL_custom_fixture.idXML
    -NuXL:decoys false
    -NuXL:presets_file ${CMAKE_CURRENT_SOURCE_DIR}/tests/data/custom-presets.json
    -NuXL:presets custom-protocol)
  add_test(NAME NuXL_custom_preset_localized_adducts COMMAND ${CMAKE_COMMAND}
    -DINPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/NuXL_custom_fixture.idXML
    -P ${OpenMSTestData_TOPP_DIR}/OpenNuXL_adduct_in_sequence_test.cmake)
  set_tests_properties(NuXL_custom_preset_localized_adducts PROPERTIES DEPENDS NuXL_custom_preset_search)
  add_test(NAME NuXL_custom_preset_unknown_name COMMAND ${CMAKE_COMMAND}
    "-DTOOL_EXECUTABLE=$<TARGET_FILE:OpenNuXL>"
    "-DTOOL_ARGUMENTS=-test;-threads;1;-in;${OpenMSTestData_TOPP_DIR}/OpenNuXL_1_input.mzML;-database;${OpenMSTestData_TOPP_DIR}/OpenNuXL_1_input.fasta;-out;${CMAKE_CURRENT_BINARY_DIR}/NuXL_invalid_preset.idXML;-NuXL:presets_file;${CMAKE_CURRENT_SOURCE_DIR}/tests/data/custom-presets.json;-NuXL:presets;RNA-MISSING-PRESET"
    -DEXPECTED_EXIT_CODE=6
    "-DEXPECTED_DIAGNOSTICS=Error: unknown preset 'RNA-MISSING-PRESET'"
    -P ${OpenMSTestData_REGRESSION_SOURCE_DIR}/cmake/ExpectToolFailure.cmake)
  add_test(NAME NuXL_custom_preset_missing_marker_ions COMMAND ${CMAKE_COMMAND}
    "-DTOOL_EXECUTABLE=$<TARGET_FILE:OpenNuXL>"
    "-DTOOL_ARGUMENTS=-test;-threads;1;-in;${OpenMSTestData_TOPP_DIR}/OpenNuXL_1_input.mzML;-database;${OpenMSTestData_TOPP_DIR}/OpenNuXL_1_input.fasta;-out;${CMAKE_CURRENT_BINARY_DIR}/NuXL_invalid_markers.idXML;-NuXL:presets_file;${CMAKE_CURRENT_SOURCE_DIR}/tests/data/invalid-marker-presets.json;-NuXL:presets;neutral-key"
    -DEXPECTED_EXIT_CODE=6
    "-DEXPECTED_DIAGNOSTICS=Preset 'neutral-key' requires marker_ions set to 'RNA' or 'DNA'"
    -P ${OpenMSTestData_REGRESSION_SOURCE_DIR}/cmake/ExpectToolFailure.cmake)
  set_tests_properties(NuXL_custom_preset_search NuXL_custom_preset_unknown_name NuXL_custom_preset_missing_marker_ions PROPERTIES
    ENVIRONMENT "OPENMS_TOOL_PREFIX_PATH=${CMAKE_BINARY_DIR};OPENMS_DISABLE_UPDATE_CHECK=ON")
endif()
