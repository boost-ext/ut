module;

#if __has_include(<unistd.h>) and __has_include(<sys/wait.h>)
#include <sys/wait.h>
#include <unistd.h>
#endif

export module boost.ut;
export import std;

#define BOOST_UT_CXX_MODULES 1
// #undef BOOST_UT_DISABLE_MODULE // not necessary anymore

#include "ut.hpp"
