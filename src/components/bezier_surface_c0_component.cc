#include "bezier_surface_c0_component.hh"

#include "../events/events.hh"

unsigned int BezierSurfaceC0Component::s_new_id = 1;

BezierSurfaceC0Component::BezierSurfaceC0Component(unsigned int patch_count_u, unsigned int patch_count_v, float size_u,
                                                   float size_v, BezierSurfaceWrapping wrapping, mge::Entity& self,
                                                   mge::Entity& grid)
    : BezierSurfaceComponent(patch_count_u, patch_count_v, wrapping, self, grid) {
  m_self.register_on_update<mge::TransformComponent, BezierSurfaceC0Component>(
      &BezierSurfaceC0Component::update_surface_by_self, this);
  create_points(size_u, size_v);
}

void BezierSurfaceC0Component::create_points(float size_u, float size_v) {
  auto pos = m_self.get_component<mge::TransformComponent>().get_position();
  std::vector<std::vector<glm::vec3>> boor_points = generate_boor_points(pos, size_u, size_v);
  std::vector<std::vector<glm::vec3>> bezier_points = generate_bezier_points(boor_points);
  m_points.resize(bezier_points.size());
  for (int v = 0; v < bezier_points.size(); ++v) {
    m_points[v].reserve(bezier_points[v].size());
    for (int u = 0; u < bezier_points[v].size(); ++u) {
      AddPointEvent event;
      SendEvent(event);
      auto& point = event.point.value().get();
      point.patch<mge::TransformComponent>(
          [pos = bezier_points[v][u]](auto& transform) { transform.set_position(pos); });
      m_self.add_child(point);
      m_points[v].push_back(
          {point.register_on_update<mge::TransformComponent>(&BezierSurfaceC0Component::update_surface, this), point});
    }
  }
}

std::vector<GeometryVertex> BezierSurfaceC0Component::generate_geometry() const {
  std::vector<GeometryVertex> vertices;
  vertices.reserve(m_point_count_u * m_point_count_v);
  for (auto& row : m_points) {
    for (auto& point : row) {
      vertices.push_back(point.second.get().get_component<mge::TransformComponent>().get_position());
    }
  }
  return vertices;
}

void BezierSurfaceC0Component::update_surface(mge::Entity& entity) {
  if (m_blocked_updates_count > 0) {
    --m_blocked_updates_count;
    return;
  }
  update_renderables(generate_geometry(), generate_surface_topology(), generate_grid_topology());
  update_position();
}