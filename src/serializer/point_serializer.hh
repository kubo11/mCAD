#ifndef MCAD_POINT_SERIALIZER
#define MCAD_POINT_SERIALIZER

#include "mge.hh"

#include "../components/point_component.hh"
#include "nlohmann/json.hpp"

using ordered_json = nlohmann::ordered_json;

class PointSerializer {
 public:
  static ordered_json serialize(const mge::Entity& entity, unsigned int id);
  static std::tuple<std::string, glm::vec3, unsigned int> deserialize(const ordered_json& data);
};

#endif  // MCAD_POINT_SERIALIZER