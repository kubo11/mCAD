#include "point.hh"

std::unique_ptr<mge::VertexArray<GeometryVertex>> Point::s_vertex_array =
    nullptr;

void Point::init_draw_data() {
  if (!s_vertex_array) {
    s_vertex_array = std::make_unique<mge::VertexArray<GeometryVertex>>(
        std::vector<GeometryVertex>{}, GeometryVertex::get_vertex_attributes(),
        std::vector<unsigned int>{1u, 2u});
  }
}
void Point::draw() const {
  m_shader.use();
  m_shader.set_uniform("world",
                       mge::translate(m_position) * mge::scale(s_scale));

  s_vertex_array->bind();

  glDrawElements(GL_TRIANGLES, s_triangles, GL_UNSIGNED_INT, 0);
}