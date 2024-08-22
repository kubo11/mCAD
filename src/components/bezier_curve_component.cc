#include "bezier_curve_component.hh"
#include "selectible_component.hh"

BezierCurveComponent::BezierCurveComponent(const mge::EntityVector& points, mge::Entity& polygon)
    : m_control_points(points), m_polygon(polygon) {}

BezierCurveComponent::~BezierCurveComponent() { m_polygon.destroy(); }

void BezierCurveComponent::set_polygon_status(bool status) {
  m_polygon.patch<mge::RenderableComponent<GeometryVertex>>([&status](auto& renderable) {
    if (status) {
      renderable.enable();
    } else {
      renderable.disable();
    }
  });
}

bool BezierCurveComponent::get_polygon_status() const {
  return m_polygon.get_component<mge::RenderableComponent<GeometryVertex>>().is_enabled();
}

void BezierCurveComponent::add_point(mge::Entity& control_point) { m_control_points.emplace_back(control_point); }

void BezierCurveComponent::remove_point(mge::Entity& control_point) {
  mge::vector_remove(m_control_points, control_point);
}