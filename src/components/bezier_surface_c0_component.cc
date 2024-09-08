#include "bezier_surface_c0_component.hh"

#include "../events/events.hh"

unsigned int BezierSurfaceC0Component::s_new_id = 1;

BezierSurfaceC0Component::BezierSurfaceC0Component(unsigned int patch_count_u, unsigned int patch_count_v, float size_u,
                                                   float size_v, BezierSurfaceWrapping wrapping, mge::Entity& self,
                                                   mge::Entity& grid)
    : BezierSurfaceComponent(patch_count_u, patch_count_v, wrapping, self, grid) {
  m_self.register_on_update<mge::TransformComponent, BezierSurfaceC0Component>(
      &BezierSurfaceC0Component::update_surface_by_self, this);
  auto [point_count_u, point_count_v] = get_bezier_point_counts();
  m_point_count_u = point_count_u;
  m_point_count_v = point_count_v;
  create_points(size_u, size_v);
  update_position();
}

BezierSurfaceC0Component::BezierSurfaceC0Component(std::vector<mge::EntityVector>& points, unsigned int patch_count_u,
                                                   unsigned int patch_count_v, BezierSurfaceWrapping wrapping,
                                                   unsigned int line_count, mge::Entity& self, mge::Entity& grid)
    : BezierSurfaceComponent(patch_count_u, patch_count_v, wrapping, self, grid) {
  m_line_count = line_count;
  m_self.register_on_update<mge::TransformComponent, BezierSurfaceC0Component>(
      &BezierSurfaceC0Component::update_surface_by_self, this);
  auto [point_count_u, point_count_v] = get_bezier_point_counts();
  m_point_count_u = point_count_u;
  m_point_count_v = point_count_v;
  m_points.resize(points.size());
  for (int v = 0; v < points.size(); ++v) {
    m_points[v].reserve(points[v].size());
    for (int u = 0; u < points[v].size(); ++u) {
      m_self.add_child(points[v][u]);
      m_points[v].push_back({points[v][u].get().register_on_update<mge::TransformComponent>(
                                 &BezierSurfaceC0Component::update_surface, this),
                             points[v][u]});
    }
  }
  update_position();
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

void BezierSurfaceC0Component::update_surface(mge::Entity& entity) {
  if (m_blocked_updates_count > 0) {
    --m_blocked_updates_count;
    return;
  }
  update_renderables(generate_geometry(), generate_surface_topology(), generate_grid_topology());
  update_position();
}

BezierSurfaceC0Component::SurfacePatchesVector BezierSurfaceC0Component::get_patches() const {
  SurfacePatchesVector patches;
  patches.resize(m_patch_count_v);
  for (int patch_v = 0; patch_v < m_patch_count_v; ++patch_v) {
    patches[patch_v].resize(m_patch_count_u);
    for (int patch_u = 0; patch_u < m_patch_count_u; ++patch_u) {
      patches[patch_v][patch_u].reserve(16);
      for (int v = 3 * patch_v; v < 3 * patch_v + 4; ++v) {
        for (int u = 3 * patch_u; u < 3 * patch_u + 4; ++u) {
          if (m_wrapping == BezierSurfaceWrapping::u && u == m_point_count_u)
            patches[patch_v][patch_u].push_back(m_points[v][0].second.get().get_id());
          else if (m_wrapping == BezierSurfaceWrapping::v && v == m_point_count_v)
            patches[patch_v][patch_u].push_back(m_points[0][u].second.get().get_id());
          else
            patches[patch_v][patch_u].push_back(m_points[v][u].second.get().get_id());
        }
      }
    }
  }
  return patches;
}