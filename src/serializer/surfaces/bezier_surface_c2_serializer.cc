#include "bezier_surface_c2_serializer.hh"

#include "../../events/events.hh"

ordered_json BezierSurfaceC2Serializer::serialize(const mge::Entity& entity, unsigned int id,
                                                  const ReversePointIdMap& point_id_map) {
  auto tag = entity.get_component<mge::TagComponent>().get_tag();
  auto& bezier = entity.get_component<BezierSurfaceC2Component>();

  ordered_json json{};
  json["objectType"] = "bezierSurfaceC2";
  json["name"] = tag;
  json["id"] = id++;
  auto patches = bezier.get_patches();

  for (int v = 0; v < patches.size(); ++v) {
    for (int u = 0; u < patches[v].size(); ++u) {
      ordered_json patch_json;
      patch_json["objectType"] = "bezierPatchC2";
      patch_json["id"] = id++;
      patch_json["name"] = "";
      for (auto point_id : patches[v][u]) {
        ordered_json point_json;
        point_json["id"] = point_id_map.at(point_id);
        patch_json["controlPoints"].push_back(point_json);
      }
      patch_json["samples"]["x"] = bezier.get_line_count();
      patch_json["samples"]["y"] = bezier.get_line_count();
      json["patches"].push_back(patch_json);
    }
  }

  json["parameterWrapped"]["u"] = bezier.get_wrapping() == BezierSurfaceWrapping::u;
  json["parameterWrapped"]["v"] = bezier.get_wrapping() == BezierSurfaceWrapping::v;

  json["size"]["x"] = bezier.get_patch_count_u();
  json["size"]["y"] = bezier.get_patch_count_v();

  return json;
}

void BezierSurfaceC2Serializer::deserialize(const ordered_json& data, PointIdMap& point_id_map) {
  auto patches_u = data["size"]["x"].get<unsigned int>();
  auto patches_v = data["size"]["y"].get<unsigned int>();
  unsigned int points_u = patches_u + 3;
  unsigned int points_v = patches_v + 3;

  float u_wrapping = data["parameterWrapped"]["u"].get<bool>();
  float v_wrapping = data["parameterWrapped"]["v"].get<bool>();
  BezierSurfaceWrapping wrapping = BezierSurfaceWrapping::none;
  if (u_wrapping) {
    wrapping = BezierSurfaceWrapping::u;
    points_u -= 3;
  } else if (v_wrapping) {
    wrapping = BezierSurfaceWrapping::v;
    points_v -= 3;
  }

  std::vector<std::vector<mge::EntityId>> points;
  points.resize(points_v);
  for (auto& row : points) {
    row.resize(points_u);
  }

  unsigned int patch_u = 0;
  unsigned int patch_v = 0;
  unsigned int line_count = 0;
  for (auto& patch_json : data["patches"]) {
    unsigned int dU = 0;
    unsigned int dV = 0;
    line_count = std::max(line_count, patch_json["samples"]["x"].get<unsigned int>());
    line_count = std::max(line_count, patch_json["samples"]["y"].get<unsigned int>());
    for (auto& point_json : patch_json["controlPoints"]) {
      unsigned int u = (patch_u + dU) % points_u;
      unsigned int v = (patch_v + dV) % points_v;

      if (!(u_wrapping && u == points_u) && !(v_wrapping && v == points_v))
        points[v][u] = point_id_map.at(point_json["id"].get<unsigned int>());

      ++dU;
      if (dU == 4) {
        dU = 0;
        ++dV;
      }
    }

    ++patch_u;
    if (patch_u == patches_u) {
      patch_u = 0;
      ++patch_v;
    }
  }

  line_count = line_count + std::min(patches_u, patches_v) + 1;

  std::string tag = "";
  if (data.contains("name")) {
    tag = data["name"].get<std::string>();
  }

  DeserializeBezierSurfaceC2Event event(tag, points, patches_u, patches_v, line_count, wrapping);
  SendEvent(event);
}