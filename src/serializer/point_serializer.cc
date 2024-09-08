#include "point_serializer.hh"

ordered_json PointSerializer::serialize(const mge::Entity& entity, unsigned int id) {
  auto tag = entity.get_component<mge::TagComponent>().get_tag();
  auto pos = entity.get_component<mge::TransformComponent>().get_position();

  ordered_json point_json{};
  point_json["id"] = id;
  point_json["name"] = tag;
  point_json["position"]["x"] = pos.x;
  point_json["position"]["y"] = pos.y;
  point_json["position"]["z"] = pos.z;

  return point_json;
}

std::tuple<std::string, glm::vec3, unsigned int> PointSerializer::deserialize(const ordered_json& data) {
  std::string tag = "";
  if (data.contains("name")) {
    tag = data["name"].get<std::string>();
  }

  auto position = glm::vec3{data["position"]["x"].get<float>(), data["position"]["y"].get<float>(),
                            data["position"]["z"].get<float>()};

  return {tag, position, data["id"].get<unsigned int>()};
}