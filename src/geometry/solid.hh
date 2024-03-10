#ifndef CMAD_SOLID_HH
#define CMAD_SOLID_HH

#include "mge.hh"

#include "line.hh"
#include "point.hh"

class Solid : public mge::Drawable {
 public:
  enum class DrawMode { NONE, WIREFRAME, SURFACE };

  Solid(const glm::vec3& position, const glm::vec3& scale = {1.0f, 1.0f, 1.0f},
        const glm::vec3& rotation = {0.0f, 0.0f, 0.0f},
        const glm::vec3& color = {0.8f, 0.8f, 0.3f})
      : m_position(position),
        m_scale(scale),
        m_rotation(rotation),
        m_color(color),
        m_shader(mge::ShaderSystem::acquire(
            s_shader_paths.at(DrawMode::WIREFRAME))) {}
  virtual ~Solid() {}

  inline void show() { m_show = true; }
  inline void hide() { m_show = false; }
  inline void set_draw_mode(DrawMode mode) { m_draw_mode = mode; }

  virtual void init_draw_data() override;
  virtual void draw() const override;

  inline const glm::vec3& get_color() const { return m_color; }
  inline const glm::vec3& get_position() const { return m_position; }
  inline const glm::vec3& get_scale() const { return m_scale; }
  inline const glm::vec3& get_rotation() const { return m_rotation; }
  inline void set_color(const glm::vec3& color) { m_color = color; }
  inline void set_position(const glm::vec3& position) { m_position = position; }
  inline void set_scale(const glm::vec3& scale) { m_scale = scale; }
  inline void set_rotation(const glm::vec3& rotation) { m_rotation = rotation; }
  inline void move(const glm::vec3& offset) { m_position += offset; }
  void rotate(const glm::vec3& angle);

 protected:
  bool m_show = true;
  DrawMode m_draw_mode = DrawMode::WIREFRAME;
  glm::vec3 m_color = {0.8f, 0.8f, 0.3f};
  glm::vec3 m_position;
  glm::vec3 m_scale = {1.0f, 1.0f, 1.0f};
  glm::vec3 m_rotation = {0.0f, 0.0f, 0.0f};

  static const std::unordered_map<DrawMode, fs::path> s_shader_paths;

  unsigned int m_indices;

  std::unique_ptr<mge::VertexArray<GeometryVertex>> m_vertex_array;
  mge::Shader& m_shader;

  virtual std::vector<GeometryVertex> generate_geometry() = 0;
  virtual std::vector<unsigned int> generate_wireframe_topology() = 0;
  virtual std::vector<unsigned int> generate_surface_topology() = 0;
};

#endif  // CMAD_SOLID_HH