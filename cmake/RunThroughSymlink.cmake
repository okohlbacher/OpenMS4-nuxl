# Run the tool through a symlink in a separate directory, the way a Homebrew cask
# or a distribution package exposes it, and require that its presets still load.
cmake_minimum_required(VERSION 3.24)

if(NOT DEFINED TOOL OR NOT DEFINED WORK_DIR)
  message(FATAL_ERROR "TOOL and WORK_DIR are required")
endif()

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}/bin")
get_filename_component(tool_name "${TOOL}" NAME)
file(CREATE_LINK "${TOOL}" "${WORK_DIR}/bin/${tool_name}" SYMBOLIC)

execute_process(COMMAND "${WORK_DIR}/bin/${tool_name}" -write_ini "${WORK_DIR}/out.ini"
                RESULT_VARIABLE code OUTPUT_VARIABLE output ERROR_VARIABLE output)
if(NOT code EQUAL 0)
  message(FATAL_ERROR "${tool_name} failed when invoked through a symlink (${code}):\n${output}")
endif()
if(NOT EXISTS "${WORK_DIR}/out.ini")
  message(FATAL_ERROR "${tool_name} wrote no INI file through the symlink")
endif()
