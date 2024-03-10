#include "ellipsoid.hh"

Ellipsoid::Ellipsoid(float a, float b, float c, const mge::Material& material)
    : RaycastingTarget(material) {
  m_a = a;
  m_b = b;
  m_c = c;

  update_matrix();
}

const glm::mat4& Ellipsoid::get_matrix() const { return m_matrix; }

const glm::vec3 Ellipsoid::get_normal_vector(const glm::vec3& point) const {
  return glm::normalize(glm::vec3(1.0f / (m_a * m_a) * point.x,
                                  1.0f / (m_b * m_b) * point.y,
                                  1.0f / (m_c * m_c) * point.z));
}

void Ellipsoid::update_matrix() {
  m_matrix = glm::diagonal4x4(glm::vec4{1.0f / (m_a * m_a), 1.0f / (m_b * m_b),
                                        1.0f / (m_c * m_c), -1.0f});
}
