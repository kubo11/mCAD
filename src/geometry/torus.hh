#ifndef CMAD_TORUS_HH
#define CMAD_TORUS_HH

#include "mge.hh"

#include "solid.hh"

class Torus : public Solid {
 public:
  Torus(float inner_radius, float outer_radius, const glm::vec3& position)
      : Solid(position) {
    m_inner_radius = inner_radius;
    m_outer_radius = outer_radius;
    m_indices = m_vertical_density * m_horizontal_density * 4;
  }
  Torus(float inner_radius, float outer_radius, const glm::vec3& position,
        const glm::vec3& scale, const glm::vec3& rotation)
      : Solid(position, scale, rotation) {
    m_inner_radius = inner_radius;
    m_outer_radius = outer_radius;
    m_indices = m_vertical_density * m_horizontal_density * 4;
  }
  Torus(float inner_radius, float outer_radius, const glm::vec3& position,
        const glm::vec3& scale, const glm::vec3& rotation,
        const glm::vec3& color)
      : Solid(position, rotation, scale, color) {
    m_inner_radius = inner_radius;
    m_outer_radius = outer_radius;
    m_indices = m_vertical_density * m_horizontal_density * 4;
  }
  virtual ~Torus() override {}

  inline float get_inner_radius() const { return m_inner_radius; }
  inline float get_outer_radius() const { return m_outer_radius; }
  inline unsigned int get_horizontal_density() const {
    return m_horizontal_density;
  }
  inline unsigned int get_vertical_density() const {
    return m_vertical_density;
  }
  void set_inner_radius(float inner_radius);
  void set_outer_radius(float outer_radius);
  void set_horizontal_density(unsigned int horizontal_density);
  void set_vertical_density(unsigned int vertical_density);

 protected:
  virtual std::vector<GeometryVertex> generate_geometry() override;
  virtual std::vector<unsigned int> generate_wireframe_topology() override;
  virtual std::vector<unsigned int> generate_surface_topology() override;

 private:
  float m_inner_radius;
  float m_outer_radius;
  unsigned int m_horizontal_density = 8;
  unsigned int m_vertical_density = 8;
};

#endif  // CMAD_TORUS_HH