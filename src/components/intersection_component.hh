#ifndef MCAD_INTERSECTION_COMPONENT
#define MCAD_INTERSECTION_COMPONENT

#include "mge.hh"

struct IntersectionComponent {
 public:
  IntersectionComponent(mge::Entity& intersectable, const std::vector<glm::vec2>& points);
  IntersectionComponent(mge::Entity& intersectable1, mge::Entity& intersectable2, const std::vector<glm::vec2>& points1, const std::vector<glm::vec2>& points2);
 private:
  mge::Entity& m_intersectable1;
  mge::OptionalEntity m_intersectable2;
  std::vector<glm::vec3> m_points;
};

#endif // MCAD_INTERSECTION_COMPONENT