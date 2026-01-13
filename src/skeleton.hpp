#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <Eigen/Geometry>
#include <iostream>
#include <nlohmann/json.hpp>
#include <rerun.hpp>
#include <vector>

using json = nlohmann::json;
using namespace std;

struct Ellipsoid3D {
  Eigen::Vector3d semi_axes;     // a, b, c
  Eigen::Quaterniond quaternion; // orientation
  Ellipsoid3D(
      const Eigen::Matrix3d &covariance,
      double k = 1.0 // scaling factor (1 = 1σ, sqrt(chi2) for confidence)
  ) {
    // Ensure symmetry (important for numerical robustness)
    Eigen::Matrix3d C = 0.5 * (covariance + covariance.transpose());

    // Eigen-decomposition for symmetric matrix
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(C);
    if (solver.info() != Eigen::Success) {
      throw std::runtime_error("Eigen decomposition failed");
    }

    // Eigenvalues (ascending order) and eigenvectors
    Eigen::Vector3d eigenvalues = solver.eigenvalues();
    Eigen::Matrix3d eigenvectors = solver.eigenvectors();

    // Semi-axes lengths
    semi_axes = k * eigenvalues.cwiseSqrt();

    // Rotation matrix from eigenvectors
    Eigen::Matrix3d R = eigenvectors;

    // Enforce right-handed coordinate system (determinant +1)
    if (R.determinant() < 0.0) {
      R.col(0) *= -1.0;
    }

    // Convert rotation matrix to quaternion
    quaternion = Eigen::Quaterniond(R);
    quaternion.normalize();
  }

  Ellipsoid3D(vector<double> cov_array, double k = 1.0) {
    if (cov_array.size() != 6) {
      throw std::invalid_argument(
          "Covariance array must have exactly 6 elements.");
    }
    Eigen::Matrix3d covariance;
    covariance << cov_array[0], cov_array[3], cov_array[4], cov_array[3],
        cov_array[1], cov_array[5], cov_array[4], cov_array[5], cov_array[2];
    *this = Ellipsoid3D(covariance, k);
  }

  Ellipsoid3D(json &ary, double k = 1.0) {
    if (!ary.is_array() || ary.size() != 6) {
      throw std::invalid_argument(
          "Covariance JSON must be an array of exactly 6 elements.");
    }
    vector<double> cov_array = ary.get<vector<double>>();
    *this = Ellipsoid3D(cov_array, k);
  }
};

class Skeleton {
  static inline const array<string, 18> ValidNodes = {
      "ANKL", "ANKR", "EARR", "EARL", "ELBL", "ELBR", "EYEL", "EYER", "HIPL",
      "HIPR", "KNEL", "KNER", "NEC_", "NOS_", "SHOL", "SHOR", "WRIL", "WRIR"};

public:
  Skeleton(rerun::RecordingStream *rec) : _rec(rec) {}
  ~Skeleton() {}

  void log(const json &data) {
    log_nodes(data);
    log_edges(data);
  }

  void log_nodes(const json &data) {
    unsigned static long frame_no = 0;
    unsigned int logged_nodes = 0;
    _rec->log("skeleton", rerun::ViewCoordinates::RIGHT_HAND_Y_UP);
    for (const auto &[key, value] : data.items()) {
      if (!value.is_object() ||
          find(ValidNodes.begin(), ValidNodes.end(), key) == ValidNodes.end()) {
        continue; // Skip invalid nodes
      }
      try {
        _rec->log("skeleton/nodes/" + key,
                  rerun::Points3D(rerun::Position3D(value["crd"]))
                      .with_radii({_radius})
                      .with_labels({key}));
        auto e = Ellipsoid3D(value["unc"]);
        _rec->log("skeleton/uncertainty/" + key,
                  rerun::Ellipsoids3D::from_centers_and_half_sizes(
                      rerun::Collection{rerun::Position3D(value["crd"])},
                      rerun::Collection{rerun::HalfSize3D(
                          e.semi_axes.x(), e.semi_axes.y(), e.semi_axes.z())})
                      .with_quaternions(
                          rerun::Collection{rerun::components::RotationQuat(
                              rerun::Quaternion::from_wxyz(
                                  e.quaternion.w(), e.quaternion.x(),
                                  e.quaternion.y(), e.quaternion.z()))}));
        logged_nodes++;
      } catch (const json::exception &e) {
        _rec->log("logs/skeleton_errors",
                  rerun::TextLog("Node " + key + " incomplete or invalid")
                      .with_level(rerun::TextLogLevel::Info));
      } catch (const std::exception &e) {
        _rec->log("logs/skeleton_errors",
                  rerun::TextLog("Error logging skeleton node " + key + ": " +
                                 string(e.what()))
                      .with_level(rerun::TextLogLevel::Error));
        cerr << "Standard exception logging skeleton node " << key << ": "
             << e.what() << endl;
      } catch (...) {
        cerr << "Unknown error logging skeleton node " << key << endl;
      }
    }
    _rec->log("nodes_no", rerun::Scalars(logged_nodes));
    _rec->log("frame_no", rerun::Scalars(frame_no));
    frame_no++;
  }

  void log_edges(const json &data) {
    vector<rerun::Vec3D> legs;
    vector<rerun::Vec3D> arms;
    vector<rerun::Vec3D> torso;
    vector<rerun::Vec3D> head;
    vector<rerun::Vec3D> neck;

    auto addIfValid = [](const json &data, const string &key,
                         vector<rerun::Vec3D> &vec) {
      if (!data[key].is_null() && data[key]["crd"].is_array() &&
          data[key]["crd"][0].is_number()) {
        vec.push_back(rerun::Vec3D(data[key]["crd"]));
      }
    };

    addIfValid(data, "ANKL", legs);
    addIfValid(data, "KNEL", legs);
    addIfValid(data, "HIPL", legs);
    addIfValid(data, "HIPR", legs);
    addIfValid(data, "KNER", legs);
    addIfValid(data, "ANKR", legs);

    addIfValid(data, "WRIL", arms);
    addIfValid(data, "ELBL", arms);
    addIfValid(data, "SHOL", arms);
    addIfValid(data, "NEC_", arms);
    addIfValid(data, "SHOR", arms);
    addIfValid(data, "ELBR", arms);
    addIfValid(data, "WRIR", arms);

    addIfValid(data, "HIPL", torso);
    addIfValid(data, "SHOL", torso);
    addIfValid(data, "NEC_", torso);
    addIfValid(data, "SHOR", torso);
    addIfValid(data, "HIPR", torso);

    addIfValid(data, "EARL", head);
    addIfValid(data, "EYEL", head);
    addIfValid(data, "NOS_", head);
    addIfValid(data, "EYER", head);
    addIfValid(data, "EARR", head);

    addIfValid(data, "NEC_", neck);
    addIfValid(data, "NOS_", neck);

    rerun::Collection<rerun::Vec3D> legs_col(legs);
    rerun::Collection<rerun::Vec3D> arms_col(arms);
    rerun::Collection<rerun::Vec3D> torso_col(torso);
    rerun::Collection<rerun::Vec3D> head_col(head);
    rerun::Collection<rerun::Vec3D> neck_col(neck);
    _rec->log("skeleton/legs", rerun::LineStrips3D({legs_col}));
    _rec->log("skeleton/arms", rerun::LineStrips3D({arms_col}));
    _rec->log("skeleton/torso", rerun::LineStrips3D({torso_col}));
    _rec->log("skeleton/head", rerun::LineStrips3D({head_col, neck_col}));
  }

  void set_radius(float radius) { _radius = radius; }

private:
  rerun::RecordingStream *_rec;
  float _radius = 1.0f;

}; // class Skeleton
