#ifndef MCAD_ELLIPSOID_HH
#define MCAD_ELLIPSOID_HH

#include "mge.hh"

class Ellipsoid : public mge::RaycastingTarget {
 public:
  Ellipsoid(float a, float b, float c, const mge::Material& material);

  virtual const glm::mat4& get_matrix() const override;
  virtual const glm::vec3 get_normal_vector(
      const glm::vec3& point) const override;

  inline const float get_a() const { return m_a; }
  inline const float get_b() const { return m_b; }
  inline const float get_c() const { return m_c; }

  inline void set_a(float a) {
    m_a = a;
    update_matrix();
  }
  inline void set_b(float b) {
    m_b = b;
    update_matrix();
  }
  inline void set_c(float c) {
    m_c = c;
    update_matrix();
  }

 private:
  float m_a;
  float m_b;
  float m_c;
  glm::mat4 m_matrix;

  void update_matrix();
};

#endif  // MCAD_ELLIPSOID_HH