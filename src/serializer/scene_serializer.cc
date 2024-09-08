#include "scene_serializer.hh"

#include <fstream>

#include "../events/events.hh"

void SceneSerializer::serialize(const mge::Scene& scene, const fs::path& path) {
  m_reverse_point_id_map.clear();
  m_new_id = 1u;

  ordered_json points_json = ordered_json::array();
  serialize_points(points_json, scene);

  ordered_json geometry_json = ordered_json::array();
  serialize_tori(geometry_json, scene);
  serialize_bezier_curves_c0(geometry_json, scene);
  serialize_bezier_curves_c2(geometry_json, scene);
  serialize_bezier_curves_c2_interp(geometry_json, scene);
  serialize_bezier_surfaces_c0(geometry_json, scene);
  serialize_bezier_surfaces_c2(geometry_json, scene);

  ordered_json scene_json;
  scene_json["geometry"] = geometry_json;
  scene_json["points"] = points_json;

  std::ofstream file(path);
  if (!file) {
    MGE_ERROR("Unable to save scene at: {}", fs::absolute(path).c_str());
    return;
  }
  file << std::setw(4) << scene_json << std::endl;
  file.close();

  MGE_INFO("Saved scene at: {}", fs::absolute(path).c_str());
}

void SceneSerializer::deserialize(mge::Scene& scene, const fs::path& path) {
  m_point_id_map.clear();

  if (!fs::exists(path)) {
    MGE_ERROR("Unable to access file: {}", fs::absolute(path).c_str());
    return;
  }
  std::ifstream file(path);
  if (!file) {
    MGE_ERROR("Unable to load scene from: {}", fs::absolute(path).c_str());
    return;
  }
  ordered_json scene_json = ordered_json::parse(file);
  file.close();

  deserialize_points(scene_json["points"], scene);
  deserialize_geometry(scene_json["geometry"], scene);
  MGE_INFO("Loaded scene from: {}", fs::absolute(path).c_str());
}

void SceneSerializer::serialize_points(ordered_json& json, const mge::Scene& scene) {
  scene.foreach<>(entt::get<PointComponent, mge::TransformComponent, mge::TagComponent>, entt::exclude<>,
                  [&](auto& entity) {
                    auto id = entity.get_id();
                    m_reverse_point_id_map.insert({id, m_new_id});
                    json.push_back(PointSerializer::serialize(entity, m_new_id++));
                  });
}

void SceneSerializer::serialize_tori(ordered_json& json, const mge::Scene& scene) {
  scene.foreach<>(entt::get<TorusComponent, mge::TransformComponent, mge::TagComponent>, entt::exclude<>,
                  [&](auto& entity) { json.push_back(TorusSerializer::serialize(entity, m_new_id++)); });
}

void SceneSerializer::serialize_bezier_curves_c0(ordered_json& json, const mge::Scene& scene) {
  scene.foreach<>(entt::get<BezierCurveC0Component, mge::TagComponent>, entt::exclude<>, [&](auto& entity) {
    json.push_back(BezierCurveC0Serializer::serialize(entity, m_new_id++, m_reverse_point_id_map));
  });
}

void SceneSerializer::serialize_bezier_curves_c2(ordered_json& json, const mge::Scene& scene) {
  scene.foreach<>(entt::get<BezierCurveC2Component, mge::TagComponent>, entt::exclude<>, [&](auto& entity) {
    json.push_back(BezierCurveC2Serializer::serialize(entity, m_new_id++, m_reverse_point_id_map));
  });
}

void SceneSerializer::serialize_bezier_curves_c2_interp(ordered_json& json, const mge::Scene& scene) {
  scene.foreach<>(entt::get<BezierCurveC2InterpComponent, mge::TagComponent>, entt::exclude<>, [&](auto& entity) {
    json.push_back(BezierCurveC2InterpSerializer::serialize(entity, m_new_id++, m_reverse_point_id_map));
  });
}

void SceneSerializer::serialize_bezier_surfaces_c0(ordered_json& json, const mge::Scene& scene) {
  scene.foreach<>(entt::get<BezierSurfaceC0Component, mge::TagComponent>, entt::exclude<>, [&](auto& entity) {
    json.push_back(BezierSurfaceC0Serializer::serialize(entity, m_new_id++, m_reverse_point_id_map));
    auto& bezier = entity.template get_component<BezierSurfaceC0Component>();
    m_new_id += (bezier.get_patch_count_u() * bezier.get_patch_count_v());
  });
}

void SceneSerializer::serialize_bezier_surfaces_c2(ordered_json& json, const mge::Scene& scene) {
  scene.foreach<>(entt::get<BezierSurfaceC2Component, mge::TagComponent>, entt::exclude<>, [&](auto& entity) {
    json.push_back(BezierSurfaceC2Serializer::serialize(entity, m_new_id++, m_reverse_point_id_map));
    auto& bezier = entity.template get_component<BezierSurfaceC2Component>();
    m_new_id += (bezier.get_patch_count_u() * bezier.get_patch_count_v());
  });
}

void SceneSerializer::deserialize_points(const ordered_json& data, const mge::Scene& scene) {
  std::vector<std::pair<std::string, glm::vec3>> points;
  std::vector<unsigned int> ids;
  for (auto& point_json : data) {
    auto point_data = PointSerializer::deserialize(point_json);
    points.push_back({std::get<0>(point_data), std::get<1>(point_data)});
    ids.push_back(std::get<2>(point_data));
  }
  DeserializePointsEvent event(points);
  SendEvent(event);

  for (int i = 0; i < points.size(); ++i) {
    m_point_id_map.insert({ids[i], event.ids[i]});
  }
}

void SceneSerializer::deserialize_geometry(const ordered_json& data, const mge::Scene& scene) {
  for (auto& object_data : data) {
    if (object_data["objectType"].get<std::string>() == "torus") {
      TorusSerializer::deserialize(object_data);
    } else if (object_data["objectType"].get<std::string>() == "bezierC0") {
      BezierCurveC0Serializer::deserialize(object_data, m_point_id_map);
    } else if (object_data["objectType"].get<std::string>() == "bezierC2") {
      BezierCurveC2Serializer::deserialize(object_data, m_point_id_map);
    } else if (object_data["objectType"].get<std::string>() == "interpolatedC2") {
      BezierCurveC2InterpSerializer::deserialize(object_data, m_point_id_map);
    } else if (object_data["objectType"].get<std::string>() == "bezierSurfaceC0") {
      BezierSurfaceC0Serializer::deserialize(object_data, m_point_id_map);
    } else if (object_data["objectType"].get<std::string>() == "bezierSurfaceC2") {
      BezierSurfaceC2Serializer::deserialize(object_data, m_point_id_map);
    }
  }
}