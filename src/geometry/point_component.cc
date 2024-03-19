#include "point_component.hh"

unsigned int PointComponent::s_new_id = 1;
float PointComponent::s_scale = 0.05f;

std::vector<GeometryVertex> PointComponent::generate_geometry() {
  return {{{-1.0, -1.0, 1.0}}, {{1.0, -1.0, 1.0}},   {{1.0, 1.0, 1.0}},
          {{-1.0, 1.0, 1.0}},  {{-1.0, -1.0, -1.0}}, {{1.0, -1.0, -1.0}},
          {{1.0, 1.0, -1.0}},  {{-1.0, 1.0, -1.0}}};
}

std::vector<unsigned int> PointComponent::generate_topology() {
  return {0, 1, 2, 2, 3, 0, 1, 5, 6, 6, 2, 1, 7, 6, 5, 5, 4, 7,
          4, 0, 3, 3, 7, 4, 4, 5, 1, 1, 0, 4, 3, 2, 6, 6, 7, 3};
}
