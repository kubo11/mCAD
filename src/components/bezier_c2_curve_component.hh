#ifndef MCAD_GEOMETRY_BEZIER_C2_CURVE_COMPONENT_HH
#define MCAD_GEOMETRY_BEZIER_C2_CURVE_COMPONENT_HH

#include "mge.hh"

#include "../vertices/geometry_vertex.hh"
#include "../vertices/point_vertex.hh"

enum class BezierCurveBase { BSpline, Bernstein };

struct BezierC2CurveComponent {
  BezierC2CurveComponent(const mge::EntityVector& points, mge::Entity& polygon);
  ~BezierC2CurveComponent();

  static std::string get_new_name() { return "Bezier C2 Curve" + std::to_string(s_new_id++); }

  void set_polygon_status(bool status);
  void add_point(mge::Entity& point);
  void remove_point(mge::Entity& control_point);
  bool get_polygon_status() const;

  mge::Entity& get_polygon() { return m_polygon; }

  BezierCurveBase get_base() const;
  void set_base(BezierCurveBase base);

  std::vector<GeometryVertex> generate_geometry() const;
  std::vector<GeometryVertex> generate_polygon_geometry() const;

 private:
  static unsigned int s_new_id;
  mge::EntityVector m_control_points;
  mge::EntityVector m_bernstein_points;
  mge::Entity& m_polygon;
  BezierCurveBase m_base;

  void create_bernstein_points();
  void update_control_points(mge::Entity& bernstein_point);
  void update_bernstein_points();
};

#endif  // MCAD_GEOMETRY_BEZIER_C2_CURVE_COMPONENT_HH