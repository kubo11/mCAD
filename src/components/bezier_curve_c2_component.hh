#ifndef MCAD_GEOMETRY_BEZIER_CURVE_C2_COMPONENT
#define MCAD_GEOMETRY_BEZIER_CURVE_C2_COMPONENT

#include "mge.hh"

#include "../vertices/point_vertex.hh"
#include "bezier_curve_component.hh"

struct BezierCurveC2Component : public BezierCurveComponent {
  BezierCurveC2Component(const mge::EntityVector& points, mge::Entity& polygon);
  virtual ~BezierCurveC2Component() override;

  static std::string get_new_name() { return "BezierCurveC2" + std::to_string(s_new_id++); }

  virtual void add_point(mge::Entity& point) override;
  virtual void remove_point(mge::Entity& control_point) override;

  BezierCurveBase get_base() const;
  void set_base(BezierCurveBase base);

  virtual std::vector<GeometryVertex> generate_geometry() const override;
  virtual std::vector<GeometryVertex> generate_polygon_geometry() const override;

 private:
  static unsigned int s_new_id;
  mge::EntityVector m_bernstein_points;
  BezierCurveBase m_base;

  void create_bernstein_points();
  void update_control_points(mge::Entity& bernstein_point);
  void update_bernstein_points();
};

#endif  // MCAD_GEOMETRY_BEZIER_CURVE_C2_COMPONENT