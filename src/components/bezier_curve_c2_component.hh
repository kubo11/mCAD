#ifndef MCAD_GEOMETRY_BEZIER_CURVE_C2_COMPONENT
#define MCAD_GEOMETRY_BEZIER_CURVE_C2_COMPONENT

#include "mge.hh"

#include "../vertices/point_vertex.hh"
#include "bezier_curve_component.hh"

struct BezierCurveC2Component : public BezierCurveComponent {
  BezierCurveC2Component(const mge::EntityVector& points, mge::Entity& self, mge::Entity& polygon);
  virtual ~BezierCurveC2Component() override;

  static std::string get_new_name() { return "BezierCurveC2 " + std::to_string(s_new_id++); }

  virtual void add_point(mge::Entity& point) override;
  virtual void remove_point(mge::Entity& control_point) override;

  std::vector<GeometryVertex> generate_geometry() const;
  std::vector<GeometryVertex> generate_polygon_geometry() const;

  void update_by_control_point(mge::Entity& entity);
  void update_by_bernstein_point(mge::Entity& entity);

  virtual void set_base(BezierCurveBase base) override;

  virtual void update_curve(mge::Entity& entity) override;
  virtual void update_curve_by_self(mge::Entity& entity) override;

 private:
  static unsigned int s_new_id;
  std::vector<std::pair<unsigned int, std::reference_wrapper<mge::Entity>>> m_bernstein_points;

  void create_bernstein_points();
  void update_control_points(mge::Entity& bernstein_point);
  void update_bernstein_points();
  void enable_point(mge::Entity& point);
  void disable_point(mge::Entity& point);
  void show_point(mge::Entity& point);
  void hide_point(mge::Entity& point);
};

#endif  // MCAD_GEOMETRY_BEZIER_CURVE_C2_COMPONENT