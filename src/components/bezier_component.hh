#ifndef MCAD_GEOMETRY_BEZIER_COMPONENT_HH
#define MCAD_GEOMETRY_BEZIER_COMPONENT_HH

#include "mge.hh"

#include "../vertices/geometry_vertex.hh"

struct BezierComponent {
  BezierComponent(const mge::EntityVector& control_points, mge::Entity& berenstein_polygon);
  ~BezierComponent();

  static std::string get_new_name() { return "Bezier " + std::to_string(s_new_id++); }

  void set_berenstein_polygon_status(bool status);
  void add_control_point(mge::Entity& control_point);
  void remove_control_point(mge::Entity& control_point);
  bool get_bezier_polygon_status() const;

  mge::Entity& get_berenstein_polygon() { return m_berenstein_polygon; }

  std::vector<GeometryVertex> generate_geometry() const;
  std::vector<GeometryVertex> generate_polygon_geometry() const;

 private:
  static unsigned int s_new_id;
  mge::EntityVector m_control_points;
  mge::Entity& m_berenstein_polygon;
};

#endif  // MCAD_GEOMETRY_BEZIER_COMPONENT_HH