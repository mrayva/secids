#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "secids::secids_isin64_cli" for configuration ""
set_property(TARGET secids::secids_isin64_cli APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(secids::secids_isin64_cli PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/bin/secids_isin64_cli"
  )

list(APPEND _cmake_import_check_targets secids::secids_isin64_cli )
list(APPEND _cmake_import_check_files_for_secids::secids_isin64_cli "${_IMPORT_PREFIX}/bin/secids_isin64_cli" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
