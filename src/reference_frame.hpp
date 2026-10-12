#pragma once

#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <nlohmann/json.hpp>

// Reference-frame visualization adapted from alfatave/rerunner_plugin,
// commit 33f4660 (Mattia Tavernini).
namespace reference_frame {
struct Frame {
  std::array<float, 3> origin;
  std::array<std::array<float, 3>, 3> axes;
};

inline void validate_scale(float scale) {
  if (!std::isfinite(scale) || scale <= 0) {
    throw std::invalid_argument("h_scale must be finite and positive");
  }
}

inline Frame from_matrix(const nlohmann::json &matrix, float scale) {
  validate_scale(scale);
  std::array<double, 16> m{};
  const bool flat = matrix.is_array() && matrix.size() == 16;
  const bool rows = matrix.is_array() && matrix.size() == 4;
  if (!flat && !rows) {
    throw std::invalid_argument("expected a row-major flat[16] or 4x4 array");
  }
  for (size_t r = 0; r < 4; ++r) {
    if (rows && (!matrix.at(r).is_array() || matrix.at(r).size() != 4)) {
      throw std::invalid_argument("each matrix row must contain exactly four numbers");
    }
    for (size_t c = 0; c < 4; ++c) {
      const auto &value = flat ? matrix.at(r * 4 + c) : matrix.at(r).at(c);
      if (!value.is_number()) {
        throw std::invalid_argument("matrix entries must be finite numbers");
      }
      const double number = value.get<double>();
      if (!std::isfinite(number)) {
        throw std::invalid_argument("matrix entries must be finite numbers");
      }
      m[r * 4 + c] = number;
    }
  }
  if (std::abs(m[12]) > 1e-6 || std::abs(m[13]) > 1e-6 ||
      std::abs(m[14]) > 1e-6 || std::abs(m[15] - 1.0) > 1e-6) {
    throw std::invalid_argument("homogeneous matrix last row must be [0, 0, 0, 1]");
  }
  auto checked_float = [](double value) {
    if (!std::isfinite(value) || std::abs(value) > std::numeric_limits<float>::max()) {
      throw std::invalid_argument("frame coordinates exceed the finite float range");
    }
    return static_cast<float>(value);
  };
  Frame frame{};
  for (size_t r = 0; r < 3; ++r) {
    frame.origin[r] = checked_float(m[r * 4 + 3]);
    for (size_t c = 0; c < 3; ++c) {
      frame.axes[c][r] = checked_float(m[r * 4 + c] * scale);
    }
  }
  return frame;
}
} // namespace reference_frame
