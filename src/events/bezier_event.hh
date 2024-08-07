#ifndef MCAD_EVENTS_BEZIER_EVENT_H
#define MCAD_EVENTS_BEZIER_EVENT_H

#include "mge.hh"

enum class BezierEvents { Add, UpdateBerensteinPolygonState, AddControlPoint, DeleteControlPoint };

class AddBezierEvent : public mge::Event<BezierEvents> {
 public:
  AddBezierEvent(std::vector<mge::EntityId> control_points)
      : mge::Event<BezierEvents>(BezierEvents::Add, "AddBezierEvent"), control_points{std::move(control_points)} {}

  std::vector<mge::EntityId> control_points;
};

class BezierUpdateBerensteinPolygonStateEvent : public mge::Event<BezierEvents> {
 public:
  BezierUpdateBerensteinPolygonStateEvent(mge::EntityId id, bool state)
      : mge::Event<BezierEvents>(BezierEvents::UpdateBerensteinPolygonState, "BezierUpdateBerensteinPolygonStateEvent"),
        id(id),
        state(state) {}

  mge::EntityId id;
  bool state;
};

class BezierAddControlPointEvent : public mge::Event<BezierEvents> {
 public:
  BezierAddControlPointEvent(mge::EntityId bezier_id, mge::EntityId control_point_id)
      : mge::Event<BezierEvents>(BezierEvents::AddControlPoint, "BezierAddControlPointEvent"),
        bezier_id(bezier_id),
        control_point_id(control_point_id) {}

  mge::EntityId bezier_id;
  mge::EntityId control_point_id;
};

class BezierDeleteControlPointEvent : public mge::Event<BezierEvents> {
 public:
  BezierDeleteControlPointEvent(mge::EntityId bezier_id, mge::EntityId control_point_id)
      : mge::Event<BezierEvents>(BezierEvents::DeleteControlPoint, "BezierDeleteControlPointEvent"),
        bezier_id(bezier_id),
        control_point_id(control_point_id) {}

  mge::EntityId bezier_id;
  mge::EntityId control_point_id;
};

#endif  // MCAD_EVENTS_BEZIER_EVENT_H
