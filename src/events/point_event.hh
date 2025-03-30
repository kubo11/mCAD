#ifndef MCAD_EVENTS_POINT_EVENT
#define MCAD_EVENTS_POINT_EVENT

#include "mge.hh"

enum class PointEvents { Add, Collapse };

class AddPointEvent : public mge::Event<PointEvents> {
 public:
  AddPointEvent() : mge::Event<PointEvents>(PointEvents::Add, "AddPointEvent") {}

  mge::OptionalEntity point;
};

class CollapsePointsEvent : public mge::Event<PointEvents> {
  public:
  CollapsePointsEvent(const std::vector<mge::EntityId>& ids) : mge::Event<PointEvents>(PointEvents::Collapse, "CollapsePointEvent"), ids(ids) {}

  std::vector<mge::EntityId> ids;
};

#endif  // MCAD_EVENTS_POINT_EVENT