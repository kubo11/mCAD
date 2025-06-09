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
  update_position();
}

BezierSurfaceC2Component::BezierSurfaceC2Component(std::vector<mge::EntityVector>& points, unsigned int patch_count_u,
                                                   unsigned int patch_count_v, BezierSurfaceWrapping wrapping,
                                                   unsigned int line_count, mge::Entity& self, mge::Entity& grid)
    : BezierSurfaceComponent(patch_count_u, patch_count_v, wrapping, self, grid) {
  m_line_count = line_count;
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
  m_points.resize(points.size());
  for (int v = 0; v < points.size(); ++v) {
    m_points[v].reserve(points[v].size());
    for (int u = 0; u < points[v].size(); ++u) {
      m_self.add_child(points[v][u]);
      m_points[v].push_back({points[v][u].get().register_on_update<mge::TransformComponent>(
                                 &BezierSurfaceC2Component::update_surface, this),
                             points[v][u]});
    }
  }
  update_position();
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

BezierSurfaceC2Component::SurfacePatchesVector BezierSurfaceC2Component::get_patches() const {
  SurfacePatchesVector patches;
  patches.resize(m_patch_count_v);
  for (int patch_v = 0; patch_v < m_patch_count_v; ++patch_v) {
    patches[patch_v].resize(m_patch_count_u);
    for (int patch_u = 0; patch_u < m_patch_count_u; ++patch_u) {
      patches[patch_v][patch_u].reserve(16);
      for (int v = patch_v; v < patch_v + 4; ++v) {
        for (int u = patch_u; u < patch_u + 4; ++u) {
          if (m_wrapping == BezierSurfaceWrapping::none)
            patches[patch_v][patch_u].push_back(m_points[v][u].second.get().get_id());
          else
            patches[patch_v][patch_u].push_back(
                m_points[v % m_point_count_v][u % m_point_count_u].second.get().get_id());
        }
      }
    }
  }
  return patches;
}

glm::vec3 BezierSurfaceC2Component::get_uv_pos(glm::vec2 uv) const {
  float u = normalize_parameter(uv.x, m_wrapping == BezierSurfaceWrapping::u);
  float v = normalize_parameter(uv.y, m_wrapping == BezierSurfaceWrapping::v);
  glm::ivec2 patch_pos = {
      std::min((int)m_patch_count_u - 1, (int)(u * m_patch_count_u)),
      std::min((int)m_patch_count_v - 1, (int)(v * m_patch_count_v))
  };
  int patch_id = patch_pos.x + patch_pos.y * m_patch_count_u;
  glm::vec3 patch[16];
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      patch[j * 4 + i] = m_points[(patch_pos.y+j)%m_point_count_v][(patch_pos.x+i)%m_point_count_u].second.get().get_component<mge::TransformComponent>().get_position();
    }
  }
  glm::vec2 patch_uv = { u * m_patch_count_u - patch_pos.x, v * m_patch_count_v - patch_pos.y };

  return c2_pos(patch_uv, patch);
}

std::pair<glm::vec3, glm::vec3> BezierSurfaceC2Component::get_uv_grad(glm::vec2 uv) const {
  float u = normalize_parameter(uv.x, m_wrapping == BezierSurfaceWrapping::u);
  float v = normalize_parameter(uv.y, m_wrapping == BezierSurfaceWrapping::v);
  glm::ivec2 patch_pos = {
      std::min((int)m_patch_count_u - 1, (int)(u * m_patch_count_u)),
      std::min((int)m_patch_count_v - 1, (int)(v * m_patch_count_v))
  };
  int patch_id = patch_pos.x + patch_pos.y * m_patch_count_u;
  glm::vec3 patch[16];
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      patch[j * 4 + i] = m_points[(patch_pos.y+j)%m_point_count_v][(patch_pos.x+i)%m_point_count_u].second.get().get_component<mge::TransformComponent>().get_position();
    }
  }
  glm::vec2 patch_uv = { u * m_patch_count_u - patch_pos.x, v * m_patch_count_v - patch_pos.y };

  return {
      c2_grad_u(patch_uv, patch),
      c2_grad_v(patch_uv, patch)
  };
}