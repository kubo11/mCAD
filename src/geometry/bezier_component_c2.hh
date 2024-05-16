#ifndef MCAD_GEOMETRY_BEZIER_COMPONENT_C2_HH
#define MCAD_GEOMETRY_BEZIER_COMPONENT_C2_HH

#include "mge.hh"

#include "geometry_vertex.hh"

struct BezierComponentC2 {
  enum class Base { Berenstein = 0, DeBoor = 1 };
  BezierComponentC2(const mge::EntityVector& control_points,
                    mge::Entity& polygon, Base base, mge::Entity& entity,
                    mge::Scene& scene);
  static std::string get_new_name() {
    return "BezierC2 " + std::to_string(s_new_id++);
  }

  void on_construct(mge::Entity& entity);
  void on_update(mge::Entity& entity);

  void set_polygon_status(bool status);
  void set_base(Base base);
  void add_control_point(mge::Entity& control_point);
  void remove_control_point(mge::Entity& control_point);
  bool get_polygon_status() const;

 private:
  static unsigned int s_new_id;
  static fs::path s_poly_shader_path;
  static fs::path s_bezier_shader_path;
  std::unordered_map<Base, mge::EntityVector> m_control_points;
  mge::Entity& m_polygon;
  Base m_base;
  mge::Entity& m_entity;
  mge::Scene& m_scene;

  std::vector<GeometryVertex> generate_geometry();
  std::vector<GeometryVertex> generate_polygon_geometry();
};

#endif  // MCAD_GEOMETRY_BEZIER_COMPONENT_C2_HH