option(BOOST_UT_ENABLE_RUN_AFTER_BUILD "Automatically run built artifacts. If disabled, the tests can be run with ctest instead" ON)

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  if(WIN32) # clang-cl
    set(BOOST_UT_COMPILE_OPTIONS
      -Wall
      -Wextra
      # FIXME -Werror
      -Wno-c++98-c++11-c++14-c++17-compat-pedantic
      -Wno-c++98-c++11-compat
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-c99-extensions
      -Wno-pedantic
    )
  else()
    set(BOOST_UT_COMPILE_OPTIONS -Wall -Wextra -Wpedantic -Werror)
  endif()
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  set(BOOST_UT_COMPILE_OPTIONS
    -Wall
    -Wextra
    # TODO: why not simpply -Wpedantic
    -Werror
    -Wcast-align
    #-Wcast-align=strict
    -Wcast-qual
    -Wdouble-promotion
    -Wduplicated-branches
    -Wduplicated-cond
    -Wlogical-op
    -Wmissing-declarations
    -Wmissing-include-dirs
    -Wnull-dereference
    -Wold-style-cast
    -Wpointer-arith
    -Wredundant-decls
    -Wsign-conversion
    -Wswitch-enum
    -Wtrampolines
    -Wunused-but-set-variable
    -Wunused-result
    -Wuseless-cast
    -Wzero-as-null-pointer-constant
    # FIXME
    -Wno-undef
    -Wno-missing-declarations
    -Wno-sign-conversion
    -Wno-float-equal
  )
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(BOOST_UT_COMPILE_OPTIONS /W4 /WX)
endif()

function(ut_add_custom_command_or_test)
  # Define the supported set of keywords
  set(prefix "PARSE")
  set(noValues "")
  set(singleValues TARGET)
  set(multiValues COMMAND)

  # Process the arguments passed in
  include(CMakeParseArguments)
  cmake_parse_arguments("${prefix}" "${noValues}" "${singleValues}" "${multiValues}" ${ARGN})
  target_link_libraries(${PARSE_TARGET} PRIVATE Boost::ut)
  target_compile_options(${PARSE_TARGET} PRIVATE ${BOOST_UT_COMPILE_OPTIONS})

  if(BOOST_UT_ENABLE_RUN_AFTER_BUILD)
    add_custom_command(TARGET ${PARSE_TARGET} COMMAND ${PARSE_COMMAND})
  else()
    add_test(NAME ${PARSE_TARGET} COMMAND ${PARSE_COMMAND})
  endif()
endfunction()
