#ifndef MCAD_TORUS_SERIALIZER
#define MCAD_TORUS_SERIALIZER

#include "mge.hh"

#include "../components/torus_component.hh"
#include "nlohmann/json.hpp"

using ordered_json = nlohmann::ordered_json;

class TorusSerializer {
 public:
  static ordered_json serialize(const mge::Entity& entity, unsigned int id);
  static void deserialize(const ordered_json& data);
};

#endif  // MCAD_TORUS_SERIALIZER