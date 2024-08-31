#include "bezier_surface_c2_component.hh"

#include "../events/events.hh"

unsigned int BezierSurfaceC2Component::s_new_id = 1;

BezierSurfaceC2Component::BezierSurfaceC2Component(unsigned int patch_count_u, unsigned int patch_count_v, float size_u,
                                                   float size_v, BezierSurfaceWrapping wrapping, mge::Entity& self,
                                                   mge::Entity& grid)
    : BezierSurfaceComponent(patch_count_u, patch_count_v, wrapping, self, grid) {
  m_self.register_on_update<mge::TransformComponent, BezierSurfaceC2Component>(
      &BezierSurfaceC2Component::update_surface_by_self, this);
  switch (wrapping) {
    case BezierSurfaceWrapping::none:
      m_point_count_u = m_patch_count_u + 3;
      m_point_count_v = m_patch_count_u + 3;
      break;

    case BezierSurfaceWrapping::u:
      m_point_count_u = m_patch_count_u;
      m_point_count_v = m_patch_count_v + 3;
      break;

    case BezierSurfaceWrapping::v:
      m_point_count_u = m_patch_count_u + 3;
      m_point_count_v = m_patch_count_v;
      break;
  }
  create_points(size_u, size_v);
  create_bezier_points();
}

void BezierSurfaceC2Component::create_points(float size_u, float size_v) {
  auto pos = m_self.get_component<mge::TransformComponent>().get_position();
  std::vector<std::vector<glm::vec3>> boor_points = generate_boor_points(pos, size_u, size_v);
  m_points.resize(boor_points.size());
  for (int v = 0; v < boor_points.size(); ++v) {
    m_points[v].reserve(boor_points[v].size());
    for (int u = 0; u < boor_points[v].size(); ++u) {
      AddPointEvent event;
      SendEvent(event);
      auto& point = event.point.value().get();
      point.patch<mge::TransformComponent>([pos = boor_points[v][u]](auto& transform) { transform.set_position(pos); });
      m_self.add_child(point);
      m_points[v].push_back(
          {point.register_on_update<mge::TransformComponent>(&BezierSurfaceC2Component::update_surface, this), point});
    }
  }
}

void BezierSurfaceC2Component::create_bezier_points() {
  SurfacePointsVector boor_points(m_points.size());
  for (int v = 0; v < m_points.size(); ++v) {
    boor_points[v].reserve(m_points[v].size());
    for (int u = 0; u < m_points[v].size(); ++u) {
      boor_points[v].push_back(m_points[v][u].second.get().get_component<mge::TransformComponent>().get_position());
    }
  }
  m_bezier_points = BezierSurfaceComponent::generate_bezier_points(boor_points);
}

std::vector<GeometryVertex> BezierSurfaceC2Component::generate_geometry() const {
  std::vector<GeometryVertex> vertices;
  vertices.reserve(m_point_count_u * m_point_count_v);
  for (auto& row : m_bezier_points) {
    for (auto& point : row) {
      vertices.push_back(point);
    }
  }
  return vertices;
}

void BezierSurfaceC2Component::update_surface(mge::Entity& entity) {
  if (m_blocked_updates_count > 0) {
    --m_blocked_updates_count;
    return;
  }
  create_bezier_points();
  update_renderables(generate_geometry(), BezierSurfaceComponent::generate_geometry(), generate_surface_topology(),
                     generate_grid_topology());
  update_position();
}