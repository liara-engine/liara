// <windows.h>, included without NOMINMAX as most Windows code does, defines `min` and `max` as function-like macros,
// and every later `std::min(` or `std::numeric_limits<T>::max(` then expands them. This file gives FixedStep.h the same
// macros, after the standard headers it includes (as <windows.h> usually comes after them too), so that a framework
// header that breaks under them breaks here, on every platform, rather than in a Windows game.

// NOLINTBEGIN(misc-include-cleaner): included before the macros, as FixedStep.h's own includes, so that the macros
// reach FixedStep.h alone and not the standard library's headers.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
// NOLINTEND(misc-include-cleaner)

// NOLINTBEGIN(cppcoreguidelines-macro-usage, readability-identifier-naming): the very macros <windows.h> defines.
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
// NOLINTEND(cppcoreguidelines-macro-usage, readability-identifier-naming)

#include <liara/framework/FixedStep.h>

#undef min
#undef max

#include <doctest/doctest.h>

// NOLINTBEGIN(readability-identifier-naming)
TEST_CASE("FixedStep - compiles and runs after <windows.h>'s min and max macros") {
    Liara::Framework::FixedStep step(60);
    const uint64_t ticks = step.Advance(250'000'000).m_Ticks;
    CHECK(ticks == 15);
}

// NOLINTEND(readability-identifier-naming)
