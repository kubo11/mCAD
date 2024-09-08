#ifndef MCAD_GEOMETRY_TORUS_COMPONENT_HH
#define MCAD_GEOMETRY_TORUS_COMPONENT_HH

#include "mge.hh"

#include "../vertices/geometry_vertex.hh"

struct TorusComponent {
  TorusComponent(float inner_radius = 4.0f, float outer_radius = 6.0f, unsigned int horizontal_density = 8,
                 unsigned int vertival_density = 8)
      : m_inner_radius(inner_radius),
        m_outer_radius(outer_radius),
        m_horizontal_density(horizontal_density),
        m_vertical_density(vertival_density) {}
  TorusComponent(TorusComponent&& other)
      : m_inner_radius(std::move(other.m_inner_radius)),
        m_outer_radius(std::move(other.m_outer_radius)),
        m_horizontal_density(std::move(other.m_horizontal_density)),
        m_vertical_density(std::move(other.m_vertical_density)) {}
  inline TorusComponent& operator=(TorusComponent&& other) {
    m_inner_radius = std::move(other.m_inner_radius);
    m_outer_radius = std::move(other.m_outer_radius);
    m_horizontal_density = std::move(other.m_horizontal_density);
    m_vertical_density = std::move(other.m_vertical_density);
    return *this;
  }

  void set_inner_radius(float inner_radius) { m_inner_radius = inner_radius; }
  void set_outer_radius(float outer_radius) { m_outer_radius = outer_radius; }
  void set_horizontal_density(unsigned int horizontal_density) {
    if (horizontal_density >= 3) {
      m_horizontal_density = horizontal_density;
    }
  }
  void set_vertical_density(unsigned int vertical_density) {
    if (vertical_density >= 3) {
      m_vertical_density = vertical_density;
    }
  }

  float get_inner_radius() const { return m_inner_radius; }
  float get_outer_radius() const { return m_outer_radius; }
  unsigned int get_horizontal_density() const { return m_horizontal_density; }
  unsigned int get_vertical_density() const { return m_vertical_density; }

  static std::string get_new_name() { return "Torus " + std::to_string(s_new_id++); }

  std::vector<GeometryVertex> generate_geometry() const;
  template <mge::RenderMode mode>
  std::vector<unsigned int> generate_topology() const;

 private:
  static unsigned int s_new_id;
  static const fs::path s_default_shader_path;

  float m_inner_radius;
  float m_outer_radius;
  unsigned int m_horizontal_density;
  unsigned int m_vertical_density;
};

#endif  // MCAD_GEOMETRY_TORUS_COMPONENT_HH