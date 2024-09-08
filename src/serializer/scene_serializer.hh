#ifndef MCAD_SCENE_SERIALIZER
#define MCAD_SCENE_SERIALIZER

#include "mge.hh"

#include "nlohmann/json.hpp"
using ordered_json = nlohmann::ordered_json;

#include "curves/bezier_curve_c0_serializer.hh"
#include "curves/bezier_curve_c2_interp_serializer.hh"
#include "curves/bezier_curve_c2_serializer.hh"
#include "point_serializer.hh"
#include "surfaces/bezier_surface_c0_serializer.hh"
#include "surfaces/bezier_surface_c2_serializer.hh"
#include "torus_serializer.hh"

class SceneSerializer {
 public:
  void serialize(const mge::Scene& scene, const fs::path& path);
  void deserialize(mge::Scene& scene, const fs::path& path);

 private:
  PointIdMap m_point_id_map;
  ReversePointIdMap m_reverse_point_id_map;
  unsigned int m_new_id;

  void serialize_points(ordered_json& json, const mge::Scene& scene);
  void serialize_tori(ordered_json& json, const mge::Scene& scene);
  void serialize_bezier_curves_c0(ordered_json& json, const mge::Scene& scene);
  void serialize_bezier_curves_c2(ordered_json& json, const mge::Scene& scene);
  void serialize_bezier_curves_c2_interp(ordered_json& json, const mge::Scene& scene);
  void serialize_bezier_surfaces_c0(ordered_json& json, const mge::Scene& scene);
  void serialize_bezier_surfaces_c2(ordered_json& json, const mge::Scene& scene);

  void deserialize_points(const ordered_json& json, const mge::Scene& scene);
  void deserialize_geometry(const ordered_json& json, const mge::Scene& scene);
};

#endif  // MCAD_SCENE_SERIALIZER