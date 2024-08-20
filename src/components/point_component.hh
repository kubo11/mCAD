#ifndef MCAD_GEOMETRY_POINT_COMPONENT_HH
#define MCAD_GEOMETRY_POINT_COMPONENT_HH

#include "mge.hh"

#include "../vertices/geometry_vertex.hh"

struct PointComponent {
  PointComponent() = default;
  PointComponent(PointComponent&&) = default;
  inline PointComponent& operator=(PointComponent&& other) { return *this; }
  static std::string get_new_name() { return "Point " + std::to_string(s_new_id++); }

 private:
  static unsigned int s_new_id;
  const int _ = 1;
};

#endif  // MCAD_GEOMETRY_POINT_COMPONENT_HH