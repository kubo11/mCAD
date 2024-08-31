#include "bezier_surface_component.hh"
#include "selectible_component.hh"

BezierSurfaceComponent::BezierSurfaceComponent(unsigned int patch_count_u, unsigned int patch_count_v,
                                               BezierSurfaceWrapping wrapping, mge::Entity& self, mge::Entity& grid)
    : m_patch_count_u(patch_count_u),
      m_patch_count_v(patch_count_v),
      m_wrapping(wrapping),
      m_self(self),
      m_grid(grid),
      m_line_count(4) {}

BezierSurfaceComponent::~BezierSurfaceComponent() {
  mge::DeleteEntityEvent event(m_grid.get_id());
  SendEngineEvent(event);
  for (auto& row : m_points) {
    for (auto& [_, point] : row) {
      m_self.remove_child(point);
      mge::DeleteEntityEvent event(point.get().get_id());
      SendEngineEvent(event);
    }
  }
}

void BezierSurfaceComponent::set_grid_status(bool status) {
  m_grid.patch<mge::RenderableComponent<GeometryVertex>>([&status](auto& renderable) {
    if (status) {
      renderable.enable();
    } else {
      renderable.disable();
    }
  });
}

bool BezierSurfaceComponent::get_grid_status() const {
  return m_grid.get_component<mge::RenderableComponent<GeometryVertex>>().is_enabled();
}

unsigned int BezierSurfaceComponent::get_line_count() const { return m_line_count; }

void BezierSurfaceComponent::set_line_count(unsigned int line_count) { m_line_count = line_count; }

void BezierSurfaceComponent::update_position() {
  glm::vec3 center = {};
  for (auto& row : m_points) {
    for (auto& [_, point] : row) {
      center += point.get().get_component<mge::TransformComponent>().get_position();
    }
  }
  m_block_updates = true;
  m_self.patch<mge::TransformComponent>([&center, count = m_point_count_u * m_point_count_v](auto& transform) {
    transform.set_position(center / static_cast<float>(count));
  });
  m_block_updates = false;
}

void BezierSurfaceComponent::update_surface_by_self(mge::Entity& entity) {
  if (m_block_updates) return;
  m_blocked_updates_count = m_point_count_u * m_point_count_v - 1;
}

std::vector<GeometryVertex> BezierSurfaceComponent::generate_geometry() const {
  std::vector<GeometryVertex> vertices;
  auto [point_count_u, point_count_v] = get_bezier_point_counts();
  vertices.reserve(point_count_u * point_count_v);
  for (auto& row : m_points) {
    for (auto& point : row) {
      vertices.push_back(point.second.get().get_component<mge::TransformComponent>().get_position());
    }
  }
  return vertices;
}

std::vector<unsigned int> BezierSurfaceComponent::generate_surface_topology() const {
  auto [point_count_u, point_count_v] = get_bezier_point_counts();

  std::vector<unsigned int> indices;
  indices.reserve(m_patch_count_u * m_patch_count_v * 16);
  for (int patch_v = 0; patch_v < m_patch_count_v; ++patch_v) {
    for (int patch_u = 0; patch_u < m_patch_count_u; ++patch_u) {
      for (int v = 3 * patch_v; v < 3 * patch_v + 4; ++v) {
        for (int u = 3 * patch_u; u < 3 * patch_u + 4; ++u) {
          indices.push_back(v % point_count_v * point_count_u + u % point_count_u);
        }
      }
    }
  }
  return indices;
}

std::vector<unsigned int> BezierSurfaceComponent::generate_grid_topology() const {
  switch (m_wrapping) {
    case BezierSurfaceWrapping::none:
      return generate_grid_topology_without_wrapping();

    case BezierSurfaceWrapping::u:
      return generate_grid_topology_u_wrapping();

    case BezierSurfaceWrapping::v:
      return generate_grid_topology_v_wrapping();

    default:
      return {};
  }
}

BezierSurfaceComponent::SurfacePointsVector BezierSurfaceComponent::generate_boor_points(const glm::vec3& pos,
                                                                                         float size_u,
                                                                                         float size_v) const {
  switch (m_wrapping) {
    case BezierSurfaceWrapping::none:
      return generate_boor_points_without_wrapping(pos, size_u, size_v);

    case BezierSurfaceWrapping::u:
      return generate_boor_points_u_wrapping(pos, size_u, size_v);

    case BezierSurfaceWrapping::v:
      return generate_boor_points_v_wrapping(pos, size_u, size_v);

    default:
      return {};
  }
}

std::pair<unsigned int, unsigned int> BezierSurfaceComponent::get_bezier_point_counts() const {
  unsigned int point_count_u;
  unsigned int point_count_v;
  switch (m_wrapping) {
    case BezierSurfaceWrapping::none:
      point_count_u = 3 * m_patch_count_u + 1;
      point_count_v = 3 * m_patch_count_v + 1;
      break;

    case BezierSurfaceWrapping::u:
      point_count_u = 3 * m_patch_count_u;
      point_count_v = 3 * m_patch_count_v + 1;
      break;

    case BezierSurfaceWrapping::v:
      point_count_u = 3 * m_patch_count_u + 1;
      point_count_v = 3 * m_patch_count_v;
      break;
  }

  return {point_count_u, point_count_v};
}

BezierSurfaceComponent::SurfacePointsVector BezierSurfaceComponent::generate_bezier_points(
    const SurfacePointsVector& boor_points) const {
  switch (m_wrapping) {
    case BezierSurfaceWrapping::none:
      return generate_bezier_points_without_wrapping(boor_points);

    case BezierSurfaceWrapping::u:
      return generate_bezier_points_u_wrapping(boor_points);

    case BezierSurfaceWrapping::v:
      return generate_bezier_points_v_wrapping(boor_points);

    default:
      return {};
  }
}

std::vector<unsigned int> BezierSurfaceComponent::generate_grid_topology_without_wrapping() const {
  std::vector<unsigned int> indices;
  indices.reserve(2 * m_point_count_v * (m_point_count_u - 1) + 2 * m_point_count_u * (m_point_count_v - 1));
  for (int v = 0; v < m_point_count_v; ++v) {
    for (int u = 0; u < m_point_count_u - 1; ++u) {
      indices.push_back(v * m_point_count_u + u);
      indices.push_back(v * m_point_count_u + u + 1);
    }
  }
  for (int u = 0; u < m_point_count_u; ++u) {
    for (int v = 0; v < m_point_count_v - 1; ++v) {
      indices.push_back(v * m_point_count_u + u);
      indices.push_back((v + 1) * m_point_count_u + u);
    }
  }
  return indices;
}

std::vector<unsigned int> BezierSurfaceComponent::generate_grid_topology_u_wrapping() const {
  std::vector<unsigned int> indices;
  indices.reserve(2 * m_point_count_v * m_point_count_u + 2 * m_point_count_u * (m_point_count_v - 1));
  for (int v = 0; v < m_point_count_v; ++v) {
    for (int u = 0; u < m_point_count_u; ++u) {
      indices.push_back(v * m_point_count_u + u);
      indices.push_back(v * m_point_count_u + (u + 1) % m_point_count_u);
    }
  }
  for (int u = 0; u < m_point_count_u; ++u) {
    for (int v = 0; v < m_point_count_v - 1; ++v) {
      indices.push_back(v * m_point_count_u + u);
      indices.push_back((v + 1) * m_point_count_u + u);
    }
  }
  return indices;
}

std::vector<unsigned int> BezierSurfaceComponent::generate_grid_topology_v_wrapping() const {
  std::vector<unsigned int> indices;
  indices.reserve(2 * m_point_count_v * (m_point_count_u - 1) + 2 * m_point_count_u * m_point_count_v);
  for (int v = 0; v < m_point_count_v; ++v) {
    for (int u = 0; u < m_point_count_u - 1; ++u) {
      indices.push_back(v * m_point_count_u + u);
      indices.push_back(v * m_point_count_u + u + 1);
    }
  }
  for (int u = 0; u < m_point_count_u; ++u) {
    for (int v = 0; v < m_point_count_v; ++v) {
      indices.push_back(v * m_point_count_u + u);
      indices.push_back((v + 1) % m_point_count_v * m_point_count_u + u);
    }
  }
  return indices;
}

BezierSurfaceComponent::SurfacePointsVector BezierSurfaceComponent::generate_boor_points_without_wrapping(
    const glm::vec3& pos, float size_u, float size_v) const {
  unsigned int boor_point_count_u = m_patch_count_u + 3;
  unsigned int boor_point_count_v = m_patch_count_v + 3;
  float dU = size_u / m_patch_count_u;
  float dV = size_v / m_patch_count_v;
  glm::vec3 first_point = pos + glm::vec3{-size_u / 2.0f, 0.0f, -size_v / 2.0f};

  SurfacePointsVector points;
  points.resize(boor_point_count_v);
  for (int v = 0; v < boor_point_count_v; ++v) {
    points[v].resize(boor_point_count_u);
  }

  for (int v = 0; v < boor_point_count_v; ++v) {
    points[v][0] = first_point;
    points[v][1] = first_point + glm::vec3{dU / 3.0f, 0.0f, 0.0f};
    for (int u = 2; u < boor_point_count_u - 2; ++u) {
      points[v][u] = first_point + glm::vec3{static_cast<float>(u - 1) * dU, 0.0f, 0.0f};
    }
    points[v][boor_point_count_u - 2] = first_point + glm::vec3{(m_patch_count_u - 1.0 / 3.0f) * dU, 0.0f, 0.0f};
    points[v][boor_point_count_u - 1] = first_point + glm::vec3{m_patch_count_u * dU, 0.0f, 0.0f};
  }
  for (int u = 0; u < boor_point_count_u; ++u) {
    points[1][u] = points[1][u] + glm::vec3{0.0f, 0.0f, dV / 3.0f};
    for (int v = 2; v < boor_point_count_v - 2; ++v) {
      points[v][u] = points[v][u] + glm::vec3{0.0f, 0.0f, static_cast<float>(v - 1) * dV};
    }
    points[boor_point_count_v - 2][u] =
        points[boor_point_count_v - 2][u] + glm::vec3{0.0f, 0.0f, (m_patch_count_v - 1.0 / 3.0f) * dV};
    points[boor_point_count_v - 1][u] = points[boor_point_count_v - 1][u] + glm::vec3{0.0f, 0.0f, m_patch_count_v * dV};
  }
  return points;
}

BezierSurfaceComponent::SurfacePointsVector BezierSurfaceComponent::generate_boor_points_u_wrapping(
    const glm::vec3& pos, float size_u, float size_v) const {
  unsigned int boor_point_count_u = m_patch_count_u;
  unsigned int boor_point_count_v = m_patch_count_v + 3;
  float dAlpha = 2.0f * glm::pi<float>() / m_patch_count_u;
  float dV = size_v / m_patch_count_v;
  float sin = std::sin(dAlpha / 2);
  float R = 3.0f * size_u / (3.0f - 2.0f * sin * sin) / 2.0f;
  glm::vec3 first_point = pos + glm::vec3{0.0f, 0.0f, -size_v / 2};

  SurfacePointsVector points;
  points.resize(boor_point_count_v);
  for (int v = 0; v < boor_point_count_v; ++v) {
    points[v].resize(boor_point_count_u);
  }

  for (int v = 0; v < boor_point_count_v; ++v) {
    for (int u = 0; u < boor_point_count_u; ++u) {
      points[v][u] = first_point + glm::vec3{R * std::cos(u * dAlpha), R * std::sin(u * dAlpha), 0.0f};
    }
  }
  for (int u = 0; u < boor_point_count_u; ++u) {
    points[1][u] = points[1][u] + glm::vec3{0.0f, 0.0f, dV / 3.0f};
    for (int v = 2; v < boor_point_count_v - 2; ++v) {
      points[v][u] = points[v][u] + glm::vec3{0.0f, 0.0f, static_cast<float>(v - 1) * dV};
    }
    points[boor_point_count_v - 2][u] =
        points[boor_point_count_v - 2][u] + glm::vec3{0.0f, 0.0f, (m_patch_count_v - 1.0 / 3.0f) * dV};
    points[boor_point_count_v - 1][u] = points[boor_point_count_v - 1][u] + glm::vec3{0.0f, 0.0f, m_patch_count_v * dV};
  }
  return points;
}

BezierSurfaceComponent::SurfacePointsVector BezierSurfaceComponent::generate_boor_points_v_wrapping(
    const glm::vec3& pos, float size_u, float size_v) const {
  unsigned int boor_point_count_u = m_patch_count_u + 3;
  unsigned int boor_point_count_v = m_patch_count_v;
  float dU = size_u / m_patch_count_u;
  float dAlpha = 2.0f * glm::pi<float>() / m_patch_count_v;
  float sin = std::sin(dAlpha / 2);
  float R = 3.0f * size_v / (3.0f - 2.0f * sin * sin) / 2.0f;
  glm::vec3 first_point = pos + glm::vec3{-size_u / 2.0f, 0.0f, 0.0f};

  SurfacePointsVector points;
  points.resize(boor_point_count_v);
  for (int v = 0; v < boor_point_count_v; ++v) {
    points[v].resize(boor_point_count_u);
  }

  for (int v = 0; v < boor_point_count_v; ++v) {
    points[v][0] = first_point;
    points[v][1] = first_point + glm::vec3{dU / 3.0f, 0.0f, 0.0f};
    for (int u = 2; u < boor_point_count_u - 2; ++u) {
      points[v][u] = first_point + glm::vec3{static_cast<float>(u - 1) * dU, 0.0f, 0.0f};
    }
    points[v][boor_point_count_u - 2] = first_point + glm::vec3{(m_patch_count_u - 1.0 / 3.0f) * dU, 0.0f, 0.0f};
    points[v][boor_point_count_u - 1] = first_point + glm::vec3{m_patch_count_u * dU, 0.0f, 0.0f};
  }
  for (int u = 0; u < boor_point_count_u; ++u) {
    for (int v = 0; v < boor_point_count_v; ++v) {
      points[v][u] = points[v][u] + glm::vec3{0.0f, R * std::sin(v * dAlpha), R * std::cos(v * dAlpha)};
    }
  }
  return points;
}

BezierSurfaceComponent::SurfacePointsVector BezierSurfaceComponent::generate_intermediate_points(
    const SurfacePointsVector& boor_points) const {
  switch (m_wrapping) {
    case BezierSurfaceWrapping::none:
      return generate_intermediate_points_without_wrapping(boor_points);

    case BezierSurfaceWrapping::u:
      return generate_intermediate_points_u_wrapping(boor_points);

    case BezierSurfaceWrapping::v:
      return generate_intermediate_points_v_wrapping(boor_points);

    default:
      return {};
  }
}

BezierSurfaceComponent::SurfacePointsVector BezierSurfaceComponent::generate_intermediate_points_without_wrapping(
    const SurfacePointsVector& boor_points) const {
  SurfacePointsVector intermediate_points;
  intermediate_points.resize(boor_points.size());
  for (int v = 0; v < boor_points.size(); ++v) {
    intermediate_points[v].reserve(3 * m_patch_count_u + 1);
    std::vector<glm::vec3> e;
    e.resize(m_patch_count_u + 1);
    std::vector<glm::vec3> f;
    f.resize(m_patch_count_u);
    std::vector<glm::vec3> g;
    g.resize(m_patch_count_u);

    f[0] = boor_points[v][1];
    if (m_patch_count_u > 1) {
      g[0] = (boor_points[v][1] + boor_points[v][2]) / 2.0f;
      for (int patch_u = 1; patch_u < m_patch_count_u - 1; ++patch_u) {
        f[patch_u] = (2.0f * boor_points[v][patch_u + 1] + boor_points[v][patch_u + 2]) / 3.0f;
        g[patch_u] = (boor_points[v][patch_u + 1] + 2.0f * boor_points[v][patch_u + 2]) / 3.0f;
      }
      f[m_patch_count_u - 1] = (boor_points[v][m_patch_count_u] + boor_points[v][m_patch_count_u + 1]) / 2.0f;
    }
    g[m_patch_count_u - 1] = boor_points[v][m_patch_count_u + 1];

    e[0] = boor_points[v][0];
    for (int patch_u = 1; patch_u < m_patch_count_u; ++patch_u) {
      e[patch_u] = (g[patch_u - 1] + f[patch_u]) / 2.0f;
    }
    e[m_patch_count_u] = boor_points[v][m_patch_count_u + 2];

    for (int patch_u = 0; patch_u < m_patch_count_u; ++patch_u) {
      intermediate_points[v].push_back(e[patch_u]);
      intermediate_points[v].push_back(f[patch_u]);
      intermediate_points[v].push_back(g[patch_u]);
    }
    intermediate_points[v].push_back(e[m_patch_count_u]);
  }
  return intermediate_points;
}

BezierSurfaceComponent::SurfacePointsVector BezierSurfaceComponent::generate_intermediate_points_u_wrapping(
    const SurfacePointsVector& boor_points) const {
  SurfacePointsVector intermediate_points;
  intermediate_points.resize(boor_points.size());
  for (int v = 0; v < boor_points.size(); ++v) {
    intermediate_points[v].reserve(3 * m_patch_count_u);
    std::vector<glm::vec3> e;
    e.resize(m_patch_count_u);
    std::vector<glm::vec3> f;
    f.resize(m_patch_count_u);
    std::vector<glm::vec3> g;
    g.resize(m_patch_count_u);

    f[0] = (2.0f * boor_points[v][0] + boor_points[v][1]) / 3.0f;
    g[0] = (boor_points[v][0] + 2.0f * boor_points[v][1]) / 3.0f;
    for (int patch_u = 1; patch_u < m_patch_count_u; ++patch_u) {
      f[patch_u] = (2.0f * boor_points[v][patch_u] + boor_points[v][(patch_u + 1) % m_patch_count_u]) / 3.0f;
      g[patch_u] = (boor_points[v][patch_u] + 2.0f * boor_points[v][(patch_u + 1) % m_patch_count_u]) / 3.0f;
      e[patch_u] = (g[patch_u - 1] + f[patch_u]) / 2.0f;
    }
    e[0] = (g[m_patch_count_u - 1] + f[0]) / 2.0f;

    for (int patch_u = 0; patch_u < m_patch_count_u; ++patch_u) {
      intermediate_points[v].push_back(e[patch_u]);
      intermediate_points[v].push_back(f[patch_u]);
      intermediate_points[v].push_back(g[patch_u]);
    }
  }
  return intermediate_points;
}

BezierSurfaceComponent::SurfacePointsVector BezierSurfaceComponent::generate_intermediate_points_v_wrapping(
    const SurfacePointsVector& boor_points) const {
  return generate_intermediate_points_without_wrapping(boor_points);
}

BezierSurfaceComponent::SurfacePointsVector BezierSurfaceComponent::generate_bezier_points_without_wrapping(
    const SurfacePointsVector& boor_points) const {
  auto [point_count_u, point_count_v] = get_bezier_point_counts();
  SurfacePointsVector bezier_points;
  bezier_points.resize(point_count_v);
  for (int v = 0; v < bezier_points.size(); ++v) {
    bezier_points[v].resize(point_count_u);
  }

  SurfacePointsVector intermediate_points = generate_intermediate_points(boor_points);
  for (int u = 0; u < intermediate_points[0].size(); ++u) {
    std::vector<glm::vec3> e;
    e.resize(m_patch_count_v + 1);
    std::vector<glm::vec3> f;
    f.resize(m_patch_count_v);
    std::vector<glm::vec3> g;
    g.resize(m_patch_count_v);

    f[0] = intermediate_points[1][u];
    if (m_patch_count_v > 1) {
      g[0] = (intermediate_points[1][u] + intermediate_points[2][u]) / 2.0f;
      for (int patch_v = 1; patch_v < m_patch_count_v - 1; ++patch_v) {
        f[patch_v] = (2.0f * intermediate_points[patch_v + 1][u] + intermediate_points[patch_v + 2][u]) / 3.0f;
        g[patch_v] = (intermediate_points[patch_v + 1][u] + 2.0f * intermediate_points[patch_v + 2][u]) / 3.0f;
      }
      f[m_patch_count_v - 1] =
          (intermediate_points[m_patch_count_v][u] + intermediate_points[m_patch_count_v + 1][u]) / 2.0f;
    }
    g[m_patch_count_v - 1] = intermediate_points[m_patch_count_v + 1][u];

    e[0] = intermediate_points[0][u];
    for (int patch_v = 1; patch_v < m_patch_count_v; ++patch_v) {
      e[patch_v] = (g[patch_v - 1] + f[patch_v]) / 2.0f;
    }
    e[m_patch_count_v] = intermediate_points[m_patch_count_v + 2][u];

    for (int patch_v = 0; patch_v < m_patch_count_v; ++patch_v) {
      bezier_points[3 * patch_v][u] = e[patch_v];
      bezier_points[3 * patch_v + 1][u] = f[patch_v];
      bezier_points[3 * patch_v + 2][u] = g[patch_v];
    }
    bezier_points[3 * m_patch_count_v][u] = e[m_patch_count_v];
  }
  return bezier_points;
}

BezierSurfaceComponent::SurfacePointsVector BezierSurfaceComponent::generate_bezier_points_u_wrapping(
    const SurfacePointsVector& boor_points) const {
  return generate_bezier_points_without_wrapping(boor_points);
}

BezierSurfaceComponent::SurfacePointsVector BezierSurfaceComponent::generate_bezier_points_v_wrapping(
    const SurfacePointsVector& boor_points) const {
  auto [point_count_u, point_count_v] = get_bezier_point_counts();
  SurfacePointsVector bezier_points;
  bezier_points.resize(point_count_v);
  for (int v = 0; v < bezier_points.size(); ++v) {
    bezier_points[v].resize(point_count_u);
  }

  SurfacePointsVector intermediate_points = generate_intermediate_points(boor_points);
  for (int u = 0; u < intermediate_points[0].size(); ++u) {
    std::vector<glm::vec3> e;
    e.resize(m_patch_count_v);
    std::vector<glm::vec3> f;
    f.resize(m_patch_count_v);
    std::vector<glm::vec3> g;
    g.resize(m_patch_count_v);

    if (m_patch_count_v > 1) {
      f[0] = (2.0f * intermediate_points[0][u] + intermediate_points[1][u]) / 3.0f;
      g[0] = (intermediate_points[0][u] + 2.0f * intermediate_points[1][u]) / 3.0f;
    } else {
      f[0] = 2.0f * intermediate_points[0][u] / 3.0f;
      g[0] = intermediate_points[0][u] / 3.0f;
    }

    for (int patch_v = 1; patch_v < m_patch_count_v; ++patch_v) {
      f[patch_v] =
          (2.0f * intermediate_points[patch_v][u] + intermediate_points[(patch_v + 1) % m_patch_count_v][u]) / 3.0f;
      g[patch_v] =
          (intermediate_points[patch_v][u] + 2.0f * intermediate_points[(patch_v + 1) % m_patch_count_v][u]) / 3.0f;
      e[patch_v] = (g[patch_v - 1] + f[patch_v]) / 2.0f;
    }
    e[0] = (g[m_patch_count_v - 1] + f[0]) / 2.0f;

    for (int patch_v = 0; patch_v < m_patch_count_v; ++patch_v) {
      bezier_points[3 * patch_v][u] = e[patch_v];
      bezier_points[3 * patch_v + 1][u] = f[patch_v];
      bezier_points[3 * patch_v + 2][u] = g[patch_v];
    }
  }
  return bezier_points;
}