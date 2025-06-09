#include "torus_component.hh"

unsigned int TorusComponent::s_new_id = 1;

std::vector<PosUvVertex> TorusComponent::generate_geometry() const {
  std::vector<PosUvVertex> vertices((m_vertical_density+1) * (m_horizontal_density+1));

  float ds = 2 * glm::pi<float>() / m_vertical_density;
  float dt = 2 * glm::pi<float>() / m_horizontal_density;
  for (int j = 0; j < m_vertical_density+1; ++j) {
    for (int i = 0; i < m_horizontal_density+1; ++i) {
      float s = j * ds;
      float t = i * dt;
      float c = m_inner_radius * std::cos(t) + m_outer_radius;
      auto rot = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      auto pos = glm::vec3{std::sin(s) * c, m_inner_radius * std::sin(t), std::cos(s) * c};
      pos = rot * glm::vec4(pos, 1.0f);

      vertices[j * (m_horizontal_density+1) + i] =
          PosUvVertex(pos,
              glm::vec2{static_cast<float>(i)/static_cast<float>(m_horizontal_density),
                        static_cast<float>(j)/static_cast<float>(m_vertical_density)});
    }
  }

  return vertices;
}

template <>
std::vector<unsigned int> TorusComponent::generate_topology<mge::RenderMode::WIREFRAME>() const {
  std::vector<unsigned int> indices(m_vertical_density * m_horizontal_density * 4);

  for (int j = 0; j < m_vertical_density; ++j) {
    for (int i = 0; i < m_horizontal_density; ++i) {
      auto a = (m_horizontal_density+1) * j + i;
      auto b = (m_horizontal_density+1) * j + (i + 1) % (m_horizontal_density+1);
      auto d = (m_horizontal_density+1) * ((j + 1) % (m_vertical_density+1)) + i;

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
std::vector<unsigned int> TorusComponent::generate_topology<mge::RenderMode::SOLID>() const {
  std::vector<unsigned int> indices(m_vertical_density * m_horizontal_density * 6);

  for (int j = 0; j < m_vertical_density; ++j) {
    for (int i = 0; i < m_horizontal_density; ++i) {
      auto a = (m_horizontal_density+1) * j + i;
      auto b = (m_horizontal_density+1) * j + (i + 1) % (m_horizontal_density+1);
      auto c = (m_horizontal_density+1) * ((j + 1) % m_vertical_density) + (i + 1) % (m_horizontal_density+1);
      auto d = (m_horizontal_density+1) * ((j + 1) % m_vertical_density+1) + i;

      // faces
      auto face_idx = j * m_horizontal_density + i;

      indices[6 * face_idx + 0] = b;
      indices[6 * face_idx + 1] = a;
      indices[6 * face_idx + 2] = d;
      indices[6 * face_idx + 3] = b;
      indices[6 * face_idx + 4] = d;
      indices[6 * face_idx + 5] = c;
    }
  }

  return indices;
}

// local space
glm::vec3 TorusComponent::get_uv_pos(glm::vec2 uv) const {
  return glm::rotate(glm::mat4(1.0f), 2.0f * glm::pi<float>() * uv.y, glm::vec3{0.0f, 1.0f, 0.0f}) * glm::vec4(
    m_outer_radius + m_inner_radius * std::cos(uv.x * 2.0f * glm::pi<float>()),
    m_inner_radius * std::sin(uv.x * 2.0f * glm::pi<float>()),
    0.0f,
    1.0f
  );
}

// local space
std::pair<glm::vec3, glm::vec3> TorusComponent::get_uv_grad(glm::vec2 uv) const {
  auto cos2piu = std::cos(uv.x * 2.0f * glm::pi<float>()),
    cos2piv = std::cos(uv.y * 2.0f * glm::pi<float>()),
    sin2piu = std::sin(uv.x * 2.0f * glm::pi<float>()),
    sin2piv = std::sin(uv.y * 2.0f * glm::pi<float>());
  glm::vec4 gradu = {
    cos2piv * (-m_inner_radius) * sin2piu * 2.0f * glm::pi<float>(),
    m_inner_radius * cos2piu * 2.0f * glm::pi<float>(),
    -sin2piv * (-m_inner_radius) * sin2piu * 2.0f * glm::pi<float>(),
    0.0f
  };
  glm::vec4 gradv = {
    -sin2piv * 2.0f * glm::pi<float>() * (m_outer_radius + m_inner_radius * cos2piu),
    0.0f,
    -cos2piv * 2.0f * glm::pi<float>() * (m_outer_radius + m_inner_radius * cos2piu),
    0.0f
  };

  return {gradu, gradv};
}