// Exercise the public plugin API with the real SDK disabled by RERUN=off.
#define main rerunner_example_main
#include "../src/rerunner.cpp"
#undef main

void require(bool condition, const char *message) {
  if (!condition) throw std::runtime_error(message);
}
int main() {
  RerunnerPlugin plugin;
  const json matrix = {{1,0,0,1}, {0,1,0,2}, {0,0,1,3}, {0,0,0,1}};
  for (bool parallel : {false, true}) {
    plugin.set_params({{"parallelize", parallel},
                       {"hmat_keypaths", {"/robot/pose", "robot.pose", "/robot/arm/pose"}}});
    require(plugin.load_data({{"pose", matrix}}, "robot") == return_type::retry,
            "frame-only message should log");
    require(plugin.load_data({{"pose", matrix}}, "robot/arm") == return_type::retry,
            "slash-containing topic should log");
    require(plugin.load_data({{"pose", matrix}}, "other") == return_type::success,
            "unrelated topic should be skipped");
    require(plugin.load_data(json::object(), "robot") == return_type::success,
            "missing field should be skipped");
    require(plugin.load_data({{"pose", nullptr}}, "robot") == return_type::error,
            "present null should return error");
    require(plugin.error().find("/robot/pose") != std::string::npos,
            "error should identify the matrix keypath");
    auto paths = json::parse(plugin.info().at("Reference frame keypaths"));
    require(paths.size() == 2, "equivalent paths should be deduplicated");
    plugin.set_params({{"parallelize", parallel}, {"hmat_keypaths", {"robot.new_pose"}}});
    require(plugin.load_data({{"pose", matrix}}, "robot") == return_type::success,
            "reconfiguration should clear the topic cache");
    require(plugin.load_data({{"new_pose", matrix}}, "robot") == return_type::retry,
            "legacy dot paths should log");
  }
  plugin.set_params(json::object());
  require(plugin.load_data({{"pose", matrix}}, "robot") == return_type::success,
          "reference frames should be opt-in");
  for (const json &params : {json{{"h_scale", 0}}, json{{"hmat_keypaths", "bad"}},
                            json{{"hmat_keypaths", {""}}}, json{{"hmat_keypaths", {42}}}}) {
    bool rejected = false;
    try { plugin.set_params(params); }
    catch (const std::invalid_argument &) { rejected = true; }
    require(rejected, "invalid configuration should be rejected");
  }
  std::cout << "Reference frame plugin integration tests passed\n";
}
