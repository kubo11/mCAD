#include "torus_serializer.hh"

#include "../events/events.hh"

ordered_json TorusSerializer::serialize(const mge::Entity& entity, unsigned int id) {
  auto tag = entity.get_component<mge::TagComponent>().get_tag();
  auto& transform = entity.get_component<mge::TransformComponent>();
  auto& torus = entity.get_component<TorusComponent>();

  ordered_json torus_json{};
  torus_json["objectType"] = "torus";
  torus_json["id"] = id;
  torus_json["name"] = tag;
  torus_json["smallRadius"] = torus.get_inner_radius();
  torus_json["largeRadius"] = torus.get_outer_radius();
  torus_json["position"]["x"] = transform.get_position().x;
  torus_json["position"]["y"] = transform.get_position().y;
  torus_json["position"]["z"] = transform.get_position().z;
  torus_json["rotation"]["x"] = transform.get_rotation_euler().x;
  torus_json["rotation"]["y"] = transform.get_rotation_euler().y;
  torus_json["rotation"]["z"] = transform.get_rotation_euler().z;
  torus_json["scale"]["x"] = transform.get_scale().x;
  torus_json["scale"]["y"] = transform.get_scale().y;
  torus_json["scale"]["z"] = transform.get_scale().z;
  torus_json["samples"]["x"] = torus.get_horizontal_density();
  torus_json["samples"]["y"] = torus.get_vertical_density();

  return torus_json;
}

void TorusSerializer::deserialize(const ordered_json& data) {
  float inner_radius = data["smallRadius"].get<float>();
  float outer_radius = data["largeRadius"].get<float>();
  unsigned int horizontal_density = data["samples"]["x"].get<unsigned int>();
  unsigned int vertical_density = data["samples"]["y"].get<unsigned int>();

  std::string tag = "";
  if (data.contains("name")) {
    tag = data["name"].get<std::string>();
  }

  auto position = glm::vec3{data["position"]["x"].get<float>(), data["position"]["y"].get<float>(),
                            data["position"]["z"].get<float>()};
  auto rotation = glm::quat{glm::vec3{data["rotation"]["x"].get<float>(), data["rotation"]["y"].get<float>(),
                                      data["rotation"]["z"].get<float>()}};
  auto scale =
      glm::vec3{data["scale"]["x"].get<float>(), data["scale"]["y"].get<float>(), data["scale"]["z"].get<float>()};

  DeserializeTorusEvent event(tag, inner_radius, outer_radius, horizontal_density, vertical_density, position, rotation,
                              scale);
  SendEvent(event);
}