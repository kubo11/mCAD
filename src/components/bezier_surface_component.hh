#ifndef MCAD_GEOMETRY_BEZIER_SURFACE_COMPONENT
#define MCAD_GEOMETRY_BEZIER_SURFACE_COMPONENT

#include "mge.hh"

#include "../vertices/geometry_vertex.hh"

enum class BezierSurfaceWrapping { none, u, v };

struct BezierSurfaceComponent {
  using SurfacePointsVector = std::vector<std::vector<glm::vec3>>;
  using SurfacePatchesVector = std::vector<std::vector<std::vector<mge::EntityId>>>;

  BezierSurfaceComponent(unsigned int patch_count_u, unsigned int patch_count_v, BezierSurfaceWrapping wrapping,
                         mge::Entity& self, mge::Entity& grid);
  virtual ~BezierSurfaceComponent();

  bool get_grid_status() const;
  void set_grid_status(bool status);
  mge::Entity& get_grid() { return m_grid; }

  BezierSurfaceWrapping get_wrapping() const { return m_wrapping; }
  unsigned int get_patch_count_u() const { return m_patch_count_u; }
  unsigned int get_patch_count_v() const { return m_patch_count_v; }
  unsigned int get_point_count_u() const { return m_point_count_u; }
  unsigned int get_point_count_v() const { return m_point_count_v; }
  const std::vector<std::vector<std::pair<unsigned int, std::reference_wrapper<mge::Entity>>>>& get_points() const { return m_points; }

  virtual SurfacePatchesVector get_patches() const = 0;

  unsigned int get_line_count() const;
  void set_line_count(unsigned int line_count);

  virtual void update_surface(mge::Entity& entity) = 0;
  void update_surface_by_self(mge::Entity& entity);
  void update_position();

  void swap_points(mge::Entity& old_point, mge::Entity& new_point);

  void update_points_status(mge::Canvas& canvas);
  void show_all_points();

  virtual std::vector<GeometryVertex> generate_geometry() const;
  std::vector<unsigned int> generate_surface_topology() const;
  std::vector<unsigned int> generate_grid_topology() const;
  SurfacePointsVector generate_boor_points(const glm::vec3& pos, float size_u, float size_v) const;
  SurfacePointsVector generate_bezier_points(const SurfacePointsVector& boor_points) const;

 protected:
  std::vector<std::vector<std::pair<unsigned int, std::reference_wrapper<mge::Entity>>>> m_points;
  mge::Entity& m_grid;
  mge::Entity& m_self;
  unsigned int m_line_count;
  unsigned int m_patch_count_u;
  unsigned int m_patch_count_v;
  unsigned int m_point_count_u;
  unsigned int m_point_count_v;
  BezierSurfaceWrapping m_wrapping;
  bool m_block_updates;
  unsigned int m_blocked_updates_count;

  template <class T>
  void update_renderables(const std::vector<T>& surface_vertices, const std::vector<T>& grid_vertices,
                          const std::vector<unsigned int>& surface_indices,
                          const std::vector<unsigned int>& grid_indices) {
    if (m_block_updates) return;
    m_self.patch<mge::RenderableComponent<T>>([this, &surface_vertices, &surface_indices](auto& renderable) {
      auto& vertex_buffer = renderable.get_vertex_array().get_vertex_buffer();
      vertex_buffer.bind();
      vertex_buffer.copy(surface_vertices);
      vertex_buffer.unbind();
      auto& element_buffer = renderable.get_vertex_array().get_element_buffer();
      element_buffer.bind();
      element_buffer.copy(surface_indices);
      element_buffer.unbind();
    });
    m_grid.patch<mge::RenderableComponent<T>>([this, &grid_vertices, &grid_indices](auto& renderable) {
      auto& vertex_buffer = renderable.get_vertex_array().get_vertex_buffer();
      vertex_buffer.bind();
      vertex_buffer.copy(grid_vertices);
      vertex_buffer.unbind();
      auto& element_buffer = renderable.get_vertex_array().get_element_buffer();
      element_buffer.bind();
      element_buffer.copy(grid_indices);
      element_buffer.unbind();
    });
  }

  template <class T>
  void update_renderables(const std::vector<T>& vertices, const std::vector<unsigned int>& surface_indices,
                          const std::vector<unsigned int>& grid_indices) {
    update_renderables<T>(vertices, vertices, surface_indices, grid_indices);
  }

  std::pair<unsigned int, unsigned int> get_bezier_point_counts() const;
  std::vector<unsigned int> generate_grid_topology_without_wrapping() const;
  std::vector<unsigned int> generate_grid_topology_u_wrapping() const;
  std::vector<unsigned int> generate_grid_topology_v_wrapping() const;
  SurfacePointsVector generate_boor_points_without_wrapping(const glm::vec3& pos, float size_u, float size_v) const;
  SurfacePointsVector generate_boor_points_u_wrapping(const glm::vec3& pos, float size_u, float size_v) const;
  SurfacePointsVector generate_boor_points_v_wrapping(const glm::vec3& pos, float size_u, float size_v) const;
  SurfacePointsVector generate_intermediate_points(const SurfacePointsVector& boor_points) const;
  SurfacePointsVector generate_intermediate_points_without_wrapping(const SurfacePointsVector& boor_points) const;
  SurfacePointsVector generate_intermediate_points_u_wrapping(const SurfacePointsVector& boor_points) const;
  SurfacePointsVector generate_intermediate_points_v_wrapping(const SurfacePointsVector& boor_points) const;
  SurfacePointsVector generate_bezier_points_without_wrapping(const SurfacePointsVector& boor_points) const;
  SurfacePointsVector generate_bezier_points_u_wrapping(const SurfacePointsVector& boor_points) const;
  SurfacePointsVector generate_bezier_points_v_wrapping(const SurfacePointsVector& boor_points) const;
};

inline float normalize_parameter(float a, bool wrap) {
  if (wrap) {
  a = std::fmod(a, 1.0f);
  if (a < 0.0f) {
    a += 1.0f;
  }
  } else {
    a = std::clamp(a, 0.0f, 1.0f);
  }
  return a;
}

inline float binom3(int i) {
    if (i == 0 || i == 3)
        return 1;
    return 3;
}

inline float B2(int i, float t) {
    if (i < 0 || i > 2) {
        return 0;
    }
    if (i == 1) {
        return 2 * t * (1 - t);
    }
    return pow(t, i) * pow(1 - t, 2 - i);
}

inline float B3(int i, float t) {
    if (i < 0 || i > 3) {
        return 0;
    }
    return binom3(i) * pow(t, i) * pow(1 - t, 3 - i);
}

inline glm::vec3 c0_pos(glm::vec2 uv, glm::vec3 (&patch)[16]) {
    float u = uv.x, v = uv.y;

    glm::vec3 result(0,0,0);
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            result += B3(i, u) * B3(j, v) * patch[j * 4 + i];
    return result;
}

inline glm::vec3 c0_grad_u(glm::vec2 uv, glm::vec3 (&patch)[16]) {
    float u = uv.x, v = uv.y;

    glm::vec3 result(0,0,0);
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            result += 3 * (B2(i-1, u) - B2(i, u)) * B3(j, v) * patch[j * 4 + i];
    return result;
    // TODO Test
}

inline glm::vec3 c0_grad_v(glm::vec2 uv, glm::vec3 (&patch)[16]) {
    float u = uv.x, v = uv.y;

    glm::vec3 result(0,0,0);
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            result += B3(i, u) * 3 * (B2(j-1, v) - B2(j, v)) * patch[j * 4 + i];
    return result;
}

inline float deBoorCoefficient(int i, float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    switch(i) {
        case -1:
            return (-t3 + 3 * t2 - 3 * t + 1) / 6;
        case 0:
            return (3 * t3 - 6 * t2 + 4) / 6;
        case 1:
            return (-3 * t3 + 3 * t2 + 3 * t + 1) / 6;
        case 2:
            default:
                return (t3) / 6;
    }
}

inline float deBoorDerivative(int i, float t) {
    float t2der = 2 * t;
    float t3der = 3 * t * t;
    switch(i) {
        case -1:
            return (-t3der + 3 * t2der - 3 /* * t der */) / 6;
        case 0:
            return (3 * t3der - 6 * t2der) / 6;
        case 1:
            return (-3 * t3der + 3 * t2der + 3 /* t der */) / 6;
        case 2:
            default:
                return (t3der) / 6;
    }
}

inline glm::vec3 c2_pos(glm::vec2 uv, glm::vec3 (&patch)[16]) {
    float u = uv.x, v = uv.y;

    glm::vec3 result(0,0,0);
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            result += deBoorCoefficient(i - 1, u) * deBoorCoefficient(j - 1, v) * patch[j * 4 + i];
    return result;
}

inline glm::vec3 c2_grad_u(glm::vec2 uv, glm::vec3 (&patch)[16]) {
    float u = uv.x, v = uv.y;

    glm::vec3 result(0,0,0);
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            result += deBoorDerivative(i - 1, u) * deBoorCoefficient(j - 1, v) * patch[j * 4 + i];
    return result;
}

inline glm::vec3 c2_grad_v(glm::vec2 uv, glm::vec3 (&patch)[16]) {
    float u = uv.x, v = uv.y;

    glm::vec3 result(0,0,0);
    for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            result += deBoorCoefficient(i - 1, u) * deBoorDerivative(j - 1, v) * patch[j * 4 + i];
    return result;
}

#endif  // MCAD_GEOMETRY_BEZIER_SURFACE_COMPONENT