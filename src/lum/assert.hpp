#pragma once

#include "lum/log.hpp"
#include <sstream>
#include <stdexcept>

namespace lum {

template <class L, class R> void assert_eq(L &&left, R &&right) {
  if (left == right)
    return;
  std::stringstream ss;
  ss << log::range_or_plain(left) << " is not equal to "
     << log::range_or_plain(right);
  throw std::runtime_error{ss.str()};
}

} // namespace lum
