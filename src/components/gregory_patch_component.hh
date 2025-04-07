#ifndef MCAD_GREGORY_PATCH_COMPONENT
#define MCAD_GREGORY_PATCH_COMPONENT

#include "mge.hh"

#include "../vertices/geometry_vertex.hh"
#include "../geometry/gregory_patch_builder.hh"

struct GregoryPatchComponent {
 public:
  GregoryPatchComponent(const GregoryPatchData& data, mge::Entity& gregory_entity, mge::Entity& vectors_entity);
  ~GregoryPatchComponent();

  static std::string get_new_name() { return "GregoryPatch " + std::to_string(s_new_id++); }

  bool get_vectors_status() const;
  void set_vectors_status(bool status);
  mge::Entity& get_vectors() { return m_vectors; }

  unsigned int get_line_count() const;
  void set_line_count(unsigned int line_count);

  void update_patch(mge::Entity& entity);
  void update_position();

  std::vector<GeometryVertex> generate_patch_geometry() const;
  std::vector<GeometryVertex> generate_vectors_geometry() const;

 private:
  static unsigned int s_new_id;
  std::vector<std::pair<unsigned int, std::reference_wrapper<mge::Entity>>> m_points;
  mge::Entity& m_vectors;
  mge::Entity& m_self;
  GregoryPatchData m_data;
  unsigned int m_line_count = 3;

  template <class T>
  void update_renderables(const std::vector<T>& surface_vertices, const std::vector<T>& vectors_vertices) {
    m_self.patch<mge::RenderableComponent<T>>([this, &surface_vertices](auto& renderable) {
      auto& vertex_buffer = renderable.get_vertex_array().get_vertex_buffer();
      vertex_buffer.bind();
      vertex_buffer.copy(surface_vertices);
      vertex_buffer.unbind();
    });
    m_vectors.patch<mge::RenderableComponent<T>>([this, &vectors_vertices](auto& renderable) {
      auto& vertex_buffer = renderable.get_vertex_array().get_vertex_buffer();
      vertex_buffer.bind();
      vertex_buffer.copy(vectors_vertices);
      vertex_buffer.unbind();
    });
  }
};

#endif // MCAD_GREGORY_PATCH_COMPONENT