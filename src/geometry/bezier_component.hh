#ifndef MCAD_GEOMETRY_BEZIER_COMPONENT_HH
#define MCAD_GEOMETRY_BEZIER_COMPONENT_HH

#include "mge.hh"

#include "geometry_vertex.hh"

struct BezierComponent {
  BezierComponent(const mge::EntityVector& control_points,
                  mge::Entity& berenstein_polygon);
  static std::string get_new_name() {
    return "Bezier " + std::to_string(s_new_id++);
  }

  void on_construct(mge::Entity& entity);
  void on_update(mge::Entity& entity);

  void update_detail(unsigned int detail);
  void set_berenstein_polygon(bool status);
  void add_control_point(mge::Entity& control_point);
  void remove_control_point(mge::Entity& control_point);
  bool get_bezier_polygon_status() const;

 private:
  static unsigned int s_new_id;
  static fs::path s_poly_shader_path;
  static fs::path s_bezier_shader_path;
  mge::EntityVector m_control_points;
  mge::Entity& m_berenstein_polygon;
  unsigned int m_detail;

  std::vector<GeometryVertex> generate_geometry();
  std::vector<GeometryVertex> generate_polygon_geometry();
};

#endif  // MCAD_GEOMETRY_BEZIER_COMPONENT_HH