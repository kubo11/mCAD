#ifndef MCAD_GEOMETRY_POINT_HH
#define MCAD_GEOMETRY_POINT_HH

#include "mge.hh"

#include "geometry_vertex.hh"

class Point : mge::Drawable {
 public:
  Point(glm::vec3 position)
      : m_position(position),
        m_shader(mge::ShaderSystem::acquire(fs::current_path() / "src" /
                                            "shaders" / "solid" / "surface")) {}
  ~Point() = default;

  virtual void init_draw_data() override;
  virtual void draw() const override;

  inline const glm::vec3& get_position() const { return m_position; }
  inline const glm::vec3& get_color() const { return m_color; }

 private:
  glm::vec3 m_position = {0.0f, 0.0f, 0.0f};
  glm::vec3 m_color = {0.0f, 0.0f, 0.0f};
  static constexpr glm::vec3 s_scale = {1.0f, 1.0f, 1.0f};
  static constexpr unsigned int s_triangles = 6;
  mge::Shader& m_shader;
  static std::unique_ptr<mge::VertexArray<GeometryVertex>> s_vertex_array;
};

#endif  // MCAD_GEOMETRY_POINT_HH