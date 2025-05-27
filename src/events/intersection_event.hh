#ifndef MCAD_INTERSECTION_EVENT
#define MCAD_INTERSECTION_EVENT

#include "mge.hh"

enum class IntersectionEvents { FindStartingPoint, Find, ConvertToInterCurve };

class FindIntersectionStartingPointEvent : public mge::Event<IntersectionEvents> {
 public:
  FindIntersectionStartingPointEvent(mge::EntityId& s1, mge::EntityId& s2, bool cursor = true)
    : mge::Event<IntersectionEvents>(IntersectionEvents::FindStartingPoint, "FindIntersectionStartingPointEvent"), s1(s1), s2(s2), cursor(cursor) {}
 
  std::pair<glm::vec2, glm::vec2> starting_point;
  mge::EntityId& s1;
  mge::EntityId& s2;
  bool cursor;
};

class FindIntersectionEvent : public mge::Event<IntersectionEvents> {
 public:
 FindIntersectionEvent(mge::EntityId& s1, mge::EntityId& s2, std::pair<glm::vec2, glm::vec2> starting_point, float newton_factor, float max_dist, bool rough)
  : mge::Event<IntersectionEvents>(IntersectionEvents::Find, "FindIntersectionEvent"), s1(s1), s2(s2), starting_point(starting_point), newton_factor(newton_factor), max_dist(max_dist), rough(rough) {}
  
  std::pair<glm::vec2, glm::vec2> starting_point;
  mge::EntityId& s1;
  mge::EntityId& s2;
  float newton_factor;
  float max_dist;
  bool rough;
};

class ConvertIntersectionToInterpCurveEvent : public mge::Event<IntersectionEvents> {
 public:
 ConvertIntersectionToInterpCurveEvent(mge::EntityId intersection)
  : mge::Event<IntersectionEvents>(IntersectionEvents::ConvertToInterCurve, "ConvertIntersectionToInterpCurveEvent"), intersection(intersection) {}
  
  mge::EntityId intersection;
};

#endif // MCAD_INTERSECTION_EVENT