#include "torus.hh"

void Torus::set_inner_radius(float inner_radius) {
  if (inner_radius > 0.0f && inner_radius < m_outer_radius) {
    m_inner_radius = inner_radius;
    m_vertex_array->update_vertices(std::move(generate_geometry()));
  }
}

void Torus::set_outer_radius(float outer_radius) {
  if (outer_radius > m_inner_radius) {
    m_outer_radius = outer_radius;
    m_vertex_array->update_vertices(std::move(generate_geometry()));
  }
}

void Torus::set_horizontal_density(unsigned int horizontal_density) {
  if (horizontal_density >= 3) {
    m_horizontal_density = horizontal_density;
    m_vertex_array->update_vertices(std::move(generate_geometry()));
    auto indices = generate_wireframe_topology();
    m_indices = indices.size();
    m_vertex_array->update_indices(std::move(indices));
  }
}

void Torus::set_vertical_density(unsigned int vertical_density) {
  if (vertical_density >= 3) {
    m_vertical_density = vertical_density;
    m_vertex_array->update_vertices(std::move(generate_geometry()));
    auto indices = generate_wireframe_topology();
    m_indices = indices.size();
    m_vertex_array->update_indices(std::move(indices));
  }
}

std::vector<GeometryVertex> Torus::generate_geometry() {
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

std::vector<unsigned int> Torus::generate_wireframe_topology() {
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

std::vector<unsigned int> Torus::generate_surface_topology() {
  std::vector<unsigned int> indices(m_vertical_density * m_horizontal_density *
                                    6);

  for (int j = 1; j <= m_vertical_density; ++j) {
    for (int i = 1; i <= m_horizontal_density; ++i) {
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
