#ifndef MCAD_GEOMETRY_BEZIER_C0_CURVE_COMPONENT_HH
#define MCAD_GEOMETRY_BEZIER_C0_CURVE_COMPONENT_HH

#include "mge.hh"

#include "../vertices/geometry_vertex.hh"

struct BezierC0CurveComponent {
  BezierC0CurveComponent(const mge::EntityVector& points, mge::Entity& polygon);
  ~BezierC0CurveComponent();

  static std::string get_new_name() { return "Bezier C0 Curve" + std::to_string(s_new_id++); }

  void set_polygon_status(bool status);
  void add_point(mge::Entity& point);
  void remove_point(mge::Entity& point);
  bool get_polygon_status() const;

  mge::Entity& get_polygon() { return m_polygon; }

  std::vector<GeometryVertex> generate_geometry() const;
  std::vector<GeometryVertex> generate_polygon_geometry() const;

 private:
  static unsigned int s_new_id;
  mge::EntityVector m_control_points;
  mge::Entity& m_polygon;
};

#endif  // MCAD_GEOMETRY_BEZIER_C0_CURVE_COMPONENT_HH