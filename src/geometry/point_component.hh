#ifndef MCAD_GEOMETRY_POINT_COMPONENT_HH
#define MCAD_GEOMETRY_POINT_COMPONENT_HH

#include "mge.hh"

#include "geometry_vertex.hh"

struct PointComponent {
  static float s_scale;

  PointComponent() {}
  static std::string get_new_name() {
    return "Point " + std::to_string(s_new_id++);
  }

  static std::unique_ptr<mge::VertexArray<GeometryVertex>> get_vertex_array();
  static mge::Shader& get_shader();

 private:
  static unsigned int s_new_id;
  const int _ = 1;
};

#endif  // MCAD_GEOMETRY_POINT_COMPONENT_HH