#include "bezier_curve_c0_serializer.hh"

#include "../../events/events.hh"

ordered_json BezierCurveC0Serializer::serialize(const mge::Entity& entity, unsigned int id,
                                                const ReversePointIdMap& point_id_map) {
  auto tag = entity.get_component<mge::TagComponent>().get_tag();
  auto& bezier = entity.get_component<BezierCurveC0Component>();

  ordered_json bezier_json{};
  bezier_json["objectType"] = "bezierC0";
  bezier_json["id"] = id;
  bezier_json["name"] = tag;
  for (auto& [_, point] : bezier.get_control_points()) {
    ordered_json point_json;
    point_json["id"] = point_id_map.at(point.get().get_id());
    bezier_json["controlPoints"].push_back(point_json);
  }

  return bezier_json;
}

void BezierCurveC0Serializer::deserialize(const ordered_json& data, PointIdMap& point_id_map) {
  std::vector<mge::EntityId> points;
  for (auto& point_data : data["controlPoints"]) {
    auto id = point_data["id"].get<int>();
    points.push_back(point_id_map.at(id));
  }

  std::string tag = "";
  if (data.contains("name")) {
    tag = data["name"].get<std::string>();
  }

  DeserializeBezierCurveC0Event event(tag, points);
  SendEvent(event);
}