#include "torus_component.hh"

unsigned int TorusComponent::s_new_id = 1;

std::vector<GeometryVertex> TorusComponent::generate_geometry() {
  std::vector<GeometryVertex> vertices(m_vertical_density *
                                       m_horizontal_density);

  float ds = 2 * glm::pi<float>() / m_vertical_density;
  float dt = 2 * glm::pi<float>() / m_horizontal_density;
  for (int j = 0; j < m_vertical_density; ++j) {
    for (int i = 0; i < m_horizontal_density; ++i) {
      float s = j * ds;
      float t = i * dt;
      float c = m_inner_radius * std::cos(t) + m_outer_radius;

      vertices[j * m_horizontal_density + i] = GeometryVertex(glm::vec3{
          std::sin(s) * c, m_inner_radius * std::sin(t), std::cos(s) * c});
    }
  }

  return vertices;
}

template <>
std::vector<unsigned int>
TorusComponent::generate_topology<mge::RenderMode::WIREFRAME>() {
  std::vector<unsigned int> indices(m_vertical_density * m_horizontal_density *
                                    4);

  for (int j = 0; j < m_vertical_density; ++j) {
    for (int i = 0; i < m_horizontal_density; ++i) {
      auto a = m_horizontal_density * j + i;
      auto b = m_horizontal_density * j + (i + 1) % m_horizontal_density;
      auto d = m_horizontal_density * ((j + 1) % m_vertical_density) + i;

      // edges
      auto face_idx = j * m_horizontal_density + i;
      indices[4 * face_idx + 0] = a;
      indices[4 * face_idx + 1] = b;
      indices[4 * face_idx + 2] = a;
      indices[4 * face_idx + 3] = d;
    }
  }

  return indices;
}

template <>
std::vector<unsigned int>
TorusComponent::generate_topology<mge::RenderMode::SURFACE>() {
  std::vector<unsigned int> indices(m_vertical_density * m_horizontal_density *
                                    6);

  for (int j = 0; j < m_vertical_density; ++j) {
    for (int i = 0; i < m_horizontal_density; ++i) {
      auto a = m_horizontal_density * j + i;
      auto b = m_horizontal_density * j + (i + 1) % m_horizontal_density;
      auto c = m_horizontal_density * ((j + 1) % m_vertical_density) +
               (i + 1) % m_horizontal_density;
      auto d = m_horizontal_density * ((j + 1) % m_vertical_density) + i;

      // faces
      auto face_idx = j * m_horizontal_density + i;
      indices[6 * face_idx + 0] = a;
      indices[6 * face_idx + 1] = b;
      indices[6 * face_idx + 2] = d;
      indices[6 * face_idx + 3] = b;
      indices[6 * face_idx + 4] = c;
      indices[6 * face_idx + 5] = d;
    }
  }

  return indices;
}
