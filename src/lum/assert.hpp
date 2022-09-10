#pragma once

#include <stdexcept>

namespace lum {

template <class L, class R> void assert_eq(L &&left, R &&right) {
  if (left == right)
    return;
  throw std::runtime_error{"left doesn't equal right"};
}

} // namespace lum
