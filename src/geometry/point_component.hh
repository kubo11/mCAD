#ifndef MCAD_GEOMETRY_POINT_COMPONENT_HH
#define MCAD_GEOMETRY_POINT_COMPONENT_HH

#include "mge.hh"

#include "geometry_vertex.hh"

struct PointComponent {
  static float s_scale;

  PointComponent(int a) : a(a) {}
  static std::string get_new_name() {
    return "Point " + std::to_string(s_new_id++);
  }

  std::vector<GeometryVertex> generate_geometry();
  std::vector<unsigned int> generate_topology();

 private:
  static unsigned int s_new_id;
  int a;
};

#endif  // MCAD_GEOMETRY_POINT_COMPONENT_HH