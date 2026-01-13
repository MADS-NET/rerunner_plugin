#include <iostream>
#include <nlohmann/json.hpp>
#include <rerun.hpp>
#include <vector>

using json = nlohmann::json;
using namespace std;

class Skeleton {
public:
  static inline const array<string, 17> ValidNodes = {
      "ANKL", "ANKR", "EARR", "ELBL", "ELBR", "EYEL", "EYER", "HIPL", "HIPR",
      "KNEL", "KNER", "NEC_", "NOS_", "SHOL", "SHOR", "WRIL", "WRIR"};

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
                  rerun::Points3D(rerun::Position3D(value["crd"])).with_radii({_radius}));
        logged_nodes++;
      } catch (const json::exception &e) {
        // NOOP
      } catch (const std::exception &e) {
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

    auto addIfValid = [](const json &data, const string &key, vector<rerun::Vec3D> &vec) {
      if (!data[key].is_null() && data[key]["crd"].is_array() && data[key]["crd"][0].is_number()) {
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
  /* data */
}; // class Skeleton
