#include "line.hh"

void Line::init_draw_data() {}
void Line::draw() const {
  m_shader.use();

  m_vertex_array->bind();

  glDrawElements(GL_LINE, 2, GL_UNSIGNED_INT, 0);
}