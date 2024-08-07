#include "bezier_component.hh"
#include "selectible_component.hh"

unsigned int BezierComponent::s_new_id = 1;

BezierComponent::BezierComponent(const mge::EntityVector& control_points, mge::Entity& berenstein_polygon)
    : m_control_points(control_points), m_berenstein_polygon(berenstein_polygon) {}

BezierComponent::~BezierComponent() { m_berenstein_polygon.destroy(); }

void BezierComponent::set_berenstein_polygon_status(bool status) {
  m_berenstein_polygon.patch<mge::RenderableComponent<GeometryVertex>>([&status](auto& renderable) {
    if (status) {
      renderable.enable();
    } else {
      renderable.disable();
    }
  });
}

std::vector<GeometryVertex> BezierComponent::generate_geometry() const {
  std::vector<GeometryVertex> points;
  if (m_control_points.empty()) {
    return points;
  }
  int size = m_control_points.size() <= 4 ? 4 : (m_control_points.size() + 1) / 3 * 4;
  points.reserve(size);
  for (int i = 0; i < m_control_points.size(); ++i) {
    if (i > 0 && i % 3 == 0) {
      points.emplace_back(m_control_points[i].get().get_component<mge::TransformComponent>().get_position());
    }
    points.emplace_back(m_control_points[i].get().get_component<mge::TransformComponent>().get_position());
  }
  for (int i = points.size(); i < size; ++i) {
    points.emplace_back(glm::vec3{std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(),
                                  std::numeric_limits<float>::quiet_NaN()});
  }

  return points;
}

std::vector<GeometryVertex> BezierComponent::generate_polygon_geometry() const {
  std::vector<GeometryVertex> points(m_control_points.size());
  for (int i = 0; i < m_control_points.size(); ++i) {
    points[i] = {m_control_points[i].get().get_component<mge::TransformComponent>().get_position()};
  }
  return points;
}

void BezierComponent::add_control_point(mge::Entity& control_point) { m_control_points.emplace_back(control_point); }

void BezierComponent::remove_control_point(mge::Entity& control_point) {
  m_control_points.erase(std::remove(m_control_points.begin(), m_control_points.end(), control_point),
                         m_control_points.end());
}

bool BezierComponent::get_bezier_polygon_status() const {
  return m_berenstein_polygon.get_component<mge::RenderableComponent<GeometryVertex>>().is_enabled();
}
