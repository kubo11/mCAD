#ifndef MCAD_GEOMETRY_BEZIER_CURVE_COMPONENT
#define MCAD_GEOMETRY_BEZIER_CURVE_COMPONENT

#include "mge.hh"

#include "../vertices/geometry_vertex.hh"

enum class BezierCurveBase { BSpline, Bernstein };

struct BezierCurveComponent {
  BezierCurveComponent(const mge::EntityVector& points, mge::Entity& polygon);
  virtual ~BezierCurveComponent();

  bool get_polygon_status() const;
  void set_polygon_status(bool status);
  mge::Entity& get_polygon() { return m_polygon; }

  virtual void add_point(mge::Entity& point);
  virtual void remove_point(mge::Entity& point);

  virtual std::vector<GeometryVertex> generate_geometry() const = 0;
  virtual std::vector<GeometryVertex> generate_polygon_geometry() const = 0;

 protected:
  mge::EntityVector m_control_points;
  mge::Entity& m_polygon;
};

#endif  // MCAD_GEOMETRY_BEZIER_C0_CURVE_COMPONENT