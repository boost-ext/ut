module;

#if __has_include(<unistd.h>) and __has_include(<sys/wait.h>)
#include <sys/wait.h>
#include <unistd.h>
#endif

export module boost.ut;
export import std;

#define BOOST_UT_CXX_MODULES 1
#include "ut.hpp"

template class boost::ut::reporter_junit<boost::ut::printer>;
template void boost::ut::reporter_junit<boost::ut::printer>::on<bool>(boost::ut::events::log<bool>);
template void boost::ut::reporter_junit<boost::ut::printer>::on<bool>(boost::ut::events::assertion_pass<bool>);
template void boost::ut::reporter_junit<boost::ut::printer>::on<bool>(boost::ut::events::assertion_fail<bool>);
template auto boost::ut::detail::test::operator=<>(test_location<void (*)()> _test);
template auto boost::ut::expect<bool>(const bool&expr,const reflection::source_location&);
template void boost::ut::reporter_junit<>::on<boost::ut::detail::fatal_<bool>>(events::assertion_fail<boost::ut::detail::fatal_<bool>>);
template void boost::ut::reporter_junit<>::on<boost::ut::detail::fatal_<bool>>(events::assertion_pass<boost::ut::detail::fatal_<bool>>);
template void boost::ut::reporter_junit<>::on<boost::ut::detail::fatal_<bool>>(events::log<boost::ut::detail::fatal_<bool>>);
