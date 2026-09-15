#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Boost::ut_module" for configuration ""
set_property(TARGET Boost::ut_module APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(Boost::ut_module PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libut_module.a"
  )

list(APPEND _cmake_import_check_targets Boost::ut_module )
list(APPEND _cmake_import_check_files_for_Boost::ut_module "${_IMPORT_PREFIX}/lib/libut_module.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
