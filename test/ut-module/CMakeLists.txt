# not using ut_add_custom_command_or_test() to explicitly link against Boost:ut_module
add_executable(from-ut-module main.cpp)
target_compile_features(from-ut-module PRIVATE cxx_std_20)
target_link_libraries(from-ut-module PRIVATE Boost::ut_module)
set_target_properties(from-ut-module PROPERTIES CXX_SCAN_FOR_MODULES ON)

if(BOOST_UT_ENABLE_RUN_AFTER_BUILD)
  add_custom_command(
    TARGET from-ut-module
    POST_BUILD
    COMMAND from-ut-module
    COMMENT "Running from-ut-module"
  )
else()
  add_test(NAME from-ut-module COMMAND from-ut-module)
endif()