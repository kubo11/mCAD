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

#endif  // MCAD_GEOMETRY_BEZIER_SURFACE_COMPONENT