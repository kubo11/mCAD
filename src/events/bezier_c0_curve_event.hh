#ifndef MCAD_EVENTS_BEZIER_C0_CURVE_EVENT_H
#define MCAD_EVENTS_BEZIER_C0_CURVE_EVENT_H

#include "mge.hh"

enum class BezierC0CurveEvents { Add, UpdatePolygonState, AddPoint, DeletePoint };

class AddBezierC0CurveEvent : public mge::Event<BezierC0CurveEvents> {
 public:
  AddBezierC0CurveEvent(std::vector<mge::EntityId> control_points)
      : mge::Event<BezierC0CurveEvents>(BezierC0CurveEvents::Add, "AddBezierC0CurveEvent"),
        control_points{std::move(control_points)} {}

  std::vector<mge::EntityId> control_points;
};

class BezierC0CurveUpdatePolygonStateEvent : public mge::Event<BezierC0CurveEvents> {
 public:
  BezierC0CurveUpdatePolygonStateEvent(mge::EntityId id, bool state)
      : mge::Event<BezierC0CurveEvents>(BezierC0CurveEvents::UpdatePolygonState,
                                        "BezierC0CurveUpdatePolygonStateEvent"),
        id(id),
        state(state) {}

  mge::EntityId id;
  bool state;
};

class BezierC0CurveAddPointEvent : public mge::Event<BezierC0CurveEvents> {
 public:
  BezierC0CurveAddPointEvent(mge::EntityId bezier_id, mge::EntityId point_id)
      : mge::Event<BezierC0CurveEvents>(BezierC0CurveEvents::AddPoint, "BezierC0CurveAddPointEvent"),
        bezier_id(bezier_id),
        point_id(point_id) {}

  mge::EntityId bezier_id;
  mge::EntityId point_id;
};

class BezierC0CurveDeletePointEvent : public mge::Event<BezierC0CurveEvents> {
 public:
  BezierC0CurveDeletePointEvent(mge::EntityId bezier_id, mge::EntityId point_id)
      : mge::Event<BezierC0CurveEvents>(BezierC0CurveEvents::DeletePoint, "BezierC0CurveDeletePointEvent"),
        bezier_id(bezier_id),
        point_id(point_id) {}

  mge::EntityId bezier_id;
  mge::EntityId point_id;
};

#endif  // MCAD_EVENTS_BEZIER_C0_CURVE_EVENT_H
