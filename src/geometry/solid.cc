#include "solid.hh"

const std::unordered_map<Solid::DrawMode, fs::path> Solid::s_shader_paths = {
    {Solid::DrawMode::SURFACE,
     fs::current_path() / "src" / "shaders" / "solid" / "surface"},
    {Solid::DrawMode::WIREFRAME,
     fs::current_path() / "src" / "shaders" / "solid" / "wireframe"}};

void Solid::init_draw_data() {
  auto indices = generate_wireframe_topology();
  m_indices = indices.size();
  m_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
      std::move(generate_geometry()), GeometryVertex::get_vertex_attributes(),
      std::move(indices));

  //   m_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
  //       std::vector<GeometryVertex>{GeometryVertex{glm::vec3{0.1f, 0.3f,
  //       0.0f}},
  //                                   GeometryVertex{glm::vec3{0.9f, 0.8f,
  //                                   0.0f}}},
  //       GeometryVertex::get_vertex_attributes(), std::vector<unsigned int>{0,
  //       1});
}

void Solid::draw() const {
  m_shader.use();
  mge::glCheckError();
  auto model = mge::translate(m_position) * mge::rotate(m_rotation) *
               mge::scale(m_scale);
  m_shader.set_uniform("model", model);
  m_shader.set_uniform("color", m_color);
  mge::glCheckError();
  m_vertex_array->bind();
  mge::glCheckError();
  if (m_draw_mode == DrawMode::SURFACE) {
    glDrawElements(GL_TRIANGLES, m_indices, GL_UNSIGNED_INT, 0);
  } else if (m_draw_mode == DrawMode::WIREFRAME) {
    glDrawElements(GL_LINES, m_indices, GL_UNSIGNED_INT, 0);
    mge::glCheckError();
  }
}

void Solid::rotate(const glm::vec3& angle) {
  m_rotation += angle;
  for (int i = 0; i < 3; ++i) {
    while (m_rotation[i] >= 2 * glm::pi<float>()) {
      m_rotation[i] -= 2 * glm::pi<float>();
    }

    while (m_rotation[i] <= 2 * glm::pi<float>()) {
      m_rotation[i] += 2 * glm::pi<float>();
    }
  }
}
