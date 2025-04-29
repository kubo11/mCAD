#ifndef MCAD_INTERSECTION_EVENT
#define MCAD_INTERSECTION_EVENT

#include "mge.hh"

enum class IntersectionEvents { Add, FindStartingPoint, Find };

class AddIntersectionEvent : public mge::Event<IntersectionEvents> {
 public:
 AddIntersectionEvent() : mge::Event<IntersectionEvents>(IntersectionEvents::Add, "AddIntersectionEvent") {}

  mge::OptionalEntity intersection;
};

class FindIntersectionStartingPointEvent : public mge::Event<IntersectionEvents> {
 public:
  FindIntersectionStartingPointEvent() : mge::Event<IntersectionEvents>(IntersectionEvents::FindStartingPoint, "FindIntersectionStartingPointEvent") {}
 
  std::optional<glm::vec3> starting_point;
};

class FindIntersectionEvent : public mge::Event<IntersectionEvents> {
 public:
 FindIntersectionEvent() : mge::Event<IntersectionEvents>(IntersectionEvents::Find, "FindIntersectionEvent") {}
  
  std::vector<glm::vec3> intersection_points;
};

#endif // MCAD_INTERSECTION_EVENT