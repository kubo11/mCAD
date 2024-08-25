#include "bezier_curve_component.hh"
#include "selectible_component.hh"

BezierCurveComponent::BezierCurveComponent(BezierCurveBase base, const mge::EntityVector& points, mge::Entity& self,
                                           mge::Entity& polygon)
    : m_base(base), m_self(self), m_polygon(polygon) {
  for (auto& point : points) {
    m_control_points.push_back({0, point});
  }
}

BezierCurveComponent::~BezierCurveComponent() {
  mge::DeleteEntityEvent event(m_polygon.get_id());
  SendEngineEvent(event);
}

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

void BezierCurveComponent::add_point(mge::Entity& point) {
  m_control_points.push_back(
      {point.register_on_update<mge::TransformComponent>(&BezierCurveComponent::update_curve, this), point});
  m_self.add_child(point);
}

void BezierCurveComponent::remove_point(mge::Entity& point) {
  unsigned int handle = 0;
  mge::vector_remove_if<std::pair<unsigned int, std::reference_wrapper<mge::Entity>>>(
      m_control_points, [&point, &handle](auto& data) {
        if (data.second.get() == point) {
          handle = data.first;
          return true;
        }
        return false;
      });
  point.unregister_on_update<mge::TransformComponent>(handle);
  m_self.remove_child(point);
}

void BezierCurveComponent::set_base(BezierCurveBase base) { m_base = base; }

BezierCurveBase BezierCurveComponent::get_base() const { return m_base; }