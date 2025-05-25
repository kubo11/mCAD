#ifndef MCAD_INTERSECTION_EVENT
#define MCAD_INTERSECTION_EVENT

#include "mge.hh"

enum class IntersectionEvents { FindStartingPoint, Find };

class FindIntersectionStartingPointEvent : public mge::Event<IntersectionEvents> {
 public:
  FindIntersectionStartingPointEvent(mge::Entity& s1, mge::Entity& s2, std::optional<glm::vec3> beg = {})
    : mge::Event<IntersectionEvents>(IntersectionEvents::FindStartingPoint, "FindIntersectionStartingPointEvent"), s1(s1), s2(s2), beg(beg) {}
 
  std::pair<glm::vec2, glm::vec2> starting_point;
  mge::Entity& s1;
  mge::Entity& s2;
  std::optional<glm::vec3> beg;
};

class FindIntersectionEvent : public mge::Event<IntersectionEvents> {
 public:
 FindIntersectionEvent(mge::Entity& s1, mge::Entity& s2, std::pair<glm::vec2, glm::vec2> starting_point, float newton_factor, float max_dist, bool rough)
  : mge::Event<IntersectionEvents>(IntersectionEvents::Find, "FindIntersectionEvent"), s1(s1), s2(s2), starting_point(starting_point), newton_factor(newton_factor), max_dist(max_dist), rough(rough) {}
  
  std::pair<glm::vec2, glm::vec2> starting_point;
  mge::Entity& s1;
  mge::Entity& s2;
  float newton_factor;
  float max_dist;
  bool rough;
};

#endif // MCAD_INTERSECTION_EVENT