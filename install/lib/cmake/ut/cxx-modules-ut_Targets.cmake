# Load information for each installed configuration.
file(GLOB _cmake_cxx_module_includes "${CMAKE_CURRENT_LIST_DIR}/cxx-modules-ut_Targets-*.cmake")
foreach(_cmake_cxx_module_include IN LISTS _cmake_cxx_module_includes)
  include("${_cmake_cxx_module_include}")
endforeach()
unset(_cmake_cxx_module_include)
unset(_cmake_cxx_module_includes)
