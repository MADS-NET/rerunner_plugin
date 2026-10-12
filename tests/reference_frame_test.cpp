#include "reference_frame.hpp"
#include <iostream>
#include <functional>

using nlohmann::json;
void require(bool condition) {
  if (!condition) throw std::runtime_error("unexpected frame geometry");
}
void rejects(const std::function<void()> &call) {
  try { call(); } catch (const std::invalid_argument &) { return; }
  throw std::runtime_error("invalid input was accepted");
}
int main() {
  const json identity = {{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}};
  auto frame = reference_frame::from_matrix(identity, 1);
  require(frame.origin == std::array<float,3>{0,0,0});
  require(frame.axes[0] == std::array<float,3>{1,0,0});
  require(frame.axes[1] == std::array<float,3>{0,1,0});
  require(frame.axes[2] == std::array<float,3>{0,0,1});
  // 90-degree rotation about Z, translated by (10,20,30), axes length 2.
  json rotated = {{0,-1,0,10}, {1,0,0,20}, {0,0,1,30}, {0,0,0,1}};
  json flat = json::array();
  for (const auto &row : rotated) for (const auto &value : row) flat.push_back(value);
  frame = reference_frame::from_matrix(rotated, 2);
  auto flattened = reference_frame::from_matrix(flat, 2);
  require(frame.origin == std::array<float,3>{10,20,30});
  require(frame.axes[0] == std::array<float,3>{0,2,0});
  require(frame.axes[1] == std::array<float,3>{-2,0,0});
  require(frame.axes[2] == std::array<float,3>{0,0,2});
  require(frame.origin == flattened.origin && frame.axes == flattened.axes);
  for (const json &bad : {json(), json::array(), json{1,2,3},
       json{{1,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}}}) {
    rejects([&] { reference_frame::from_matrix(bad, 1); });
  }
  for (const json &value : {json("1"), json(true), json(nullptr),
       json(std::numeric_limits<double>::infinity()), json(1e300)}) {
    auto bad = identity; bad[0][0] = value;
    rejects([&] { reference_frame::from_matrix(bad, 1); });
  }
  auto bad = identity; bad[3][0] = 1;
  rejects([&] { reference_frame::from_matrix(bad, 1); });
  for (float scale : {0.f, -1.f, std::numeric_limits<float>::infinity(),
                      std::numeric_limits<float>::quiet_NaN()}) {
    rejects([&] { reference_frame::from_matrix(identity, scale); });
  }
  bad = identity; bad[0][0] = std::numeric_limits<float>::max();
  rejects([&] { reference_frame::from_matrix(bad, 2); });
  std::cout << "Reference frame geometry and validation tests passed\n";
}
