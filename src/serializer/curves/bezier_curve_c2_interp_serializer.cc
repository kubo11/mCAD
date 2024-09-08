#include "bezier_curve_c2_interp_serializer.hh"

#include "../../events/events.hh"

ordered_json BezierCurveC2InterpSerializer::serialize(const mge::Entity& entity, unsigned int id,
                                                      const ReversePointIdMap& point_id_map) {
  auto tag = entity.get_component<mge::TagComponent>().get_tag();
  auto& bezier = entity.get_component<BezierCurveC2InterpComponent>();

  ordered_json json{};
  json["objectType"] = "interpolatedC2";
  json["id"] = id;
  json["name"] = tag;
  for (auto& [_, point] : bezier.get_control_points()) {
    ordered_json point_json;
    point_json["id"] = point_id_map.at(point.get().get_id());
    json["controlPoints"].push_back(point_json);
  }

  return json;
}

void BezierCurveC2InterpSerializer::deserialize(const ordered_json& data, PointIdMap& point_id_map) {
  std::vector<mge::EntityId> points;
  for (auto& point_data : data["controlPoints"]) {
    auto id = point_data["id"].get<int>();
    points.push_back(point_id_map.at(id));
  }

  std::string tag = "";
  if (data.contains("name")) {
    tag = data["name"].get<std::string>();
  }

  DeserializeBezierCurveC2InterpEvent event(tag, points);
  SendEvent(event);
}