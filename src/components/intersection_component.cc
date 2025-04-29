#include "intersection_component.hh"

#include "components/bezier_surface_c0_component.hh"
#include "components/bezier_surface_c2_component.hh"
#include "components/torus_component.hh"

unsigned int IntersectionComponent::s_new_id = 1;

glm::vec3 position(mge::Entity& e, glm::vec2 uv) {
  if (e.has_component<TorusComponent>()) {
    return e.get_component<mge::TransformComponent>().get_model_mat() * glm::vec4(e.get_component<TorusComponent>().get_uv_pos(uv), 1.0f);
  }
  else if (e.has_component<BezierSurfaceC0Component>()) {
    return e.get_component<BezierSurfaceC0Component>().get_uv_pos(uv);
  }
  else if (e.has_component<BezierSurfaceC2Component>()) {
    return e.get_component<BezierSurfaceC2Component>().get_uv_pos(uv);
  }
  return glm::vec3(0.0f);
}

IntersectionComponent::IntersectionComponent(mge::Entity& intersectable, const std::vector<glm::vec2>& points) : m_intersectable1{intersectable}, m_intersectable2{std::nullopt}, m_points{} {
  m_points.reserve(points.size());
  for (const auto& point : points) {
    m_points.push_back(position(intersectable, point));
  }
}

IntersectionComponent::IntersectionComponent(mge::Entity& intersectable1, mge::Entity& intersectable2, const std::vector<glm::vec2>& points1, const std::vector<glm::vec2>& points2)
 : m_intersectable1{intersectable1}, m_intersectable2{intersectable2}, m_points{} {
  m_points.reserve(points1.size());
  for (const auto& point : points1) {
    m_points.push_back(position(intersectable1, point));
  }
}