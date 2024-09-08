#ifndef MCAD_BEZIER_SURFACE_C2_SERIALIZER
#define MCAD_BEZIER_SURFACE_C2_SERIALIZER

#include "mge.hh"

#include "../../components/bezier_surface_c2_component.hh"
#include "nlohmann/json.hpp"

using ordered_json = nlohmann::ordered_json;
using PointIdMap = std::unordered_map<unsigned int, mge::EntityId>;
using ReversePointIdMap = std::unordered_map<mge::EntityId, unsigned int>;

class BezierSurfaceC2Serializer {
 public:
  static ordered_json serialize(const mge::Entity& entity, unsigned int id, const ReversePointIdMap& point_id_map);
  static void deserialize(const ordered_json& data, PointIdMap& point_id_map);
};

#endif  // MCAD_BEZIER_SURFACE_C2_SERIALIZER