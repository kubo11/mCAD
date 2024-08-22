#ifndef MCAD_EVENTS_BEZIER_C2_CURVE_EVENT_H
#define MCAD_EVENTS_BEZIER_C2_CURVE_EVENT_H

#include "mge.hh"

#include "../components/bezier_c2_curve_component.hh"

enum class BezierC2CurveEvents { Add, UpdatePolygonState, UpdateBase, AddPoint, DeletePoint };

class AddBezierC2CurveEvent : public mge::Event<BezierC2CurveEvents> {
 public:
  AddBezierC2CurveEvent(std::vector<mge::EntityId> control_points)
      : mge::Event<BezierC2CurveEvents>(BezierC2CurveEvents::Add, "AddBezierC2CurveEvent"),
        control_points{std::move(control_points)} {}

  std::vector<mge::EntityId> control_points;
};

class BezierC2CurveUpdatePolygonStateEvent : public mge::Event<BezierC2CurveEvents> {
 public:
  BezierC2CurveUpdatePolygonStateEvent(mge::EntityId id, bool state)
      : mge::Event<BezierC2CurveEvents>(BezierC2CurveEvents::UpdatePolygonState,
                                        "BezierC2CurveUpdatePolygonStateEvent"),
        id(id),
        state(state) {}

  mge::EntityId id;
  bool state;
};

class BezierC2CurveUpdateBaseEvent : public mge::Event<BezierC2CurveEvents> {
 public:
  BezierC2CurveUpdateBaseEvent(mge::EntityId id, BezierCurveBase base)
      : mge::Event<BezierC2CurveEvents>(BezierC2CurveEvents::UpdateBase, "BezierC2CurveUpdateBaseEvent"),
        id(id),
        base(base) {}

  mge::EntityId id;
  BezierCurveBase base;
};

class BezierC2CurveAddPointEvent : public mge::Event<BezierC2CurveEvents> {
 public:
  BezierC2CurveAddPointEvent(mge::EntityId bezier_id, mge::EntityId point_id)
      : mge::Event<BezierC2CurveEvents>(BezierC2CurveEvents::AddPoint, "BezierC2CurveAddPointEvent"),
        bezier_id(bezier_id),
        point_id(point_id) {}

  mge::EntityId bezier_id;
  mge::EntityId point_id;
};

class BezierC2CurveDeletePointEvent : public mge::Event<BezierC2CurveEvents> {
 public:
  BezierC2CurveDeletePointEvent(mge::EntityId bezier_id, mge::EntityId point_id)
      : mge::Event<BezierC2CurveEvents>(BezierC2CurveEvents::DeletePoint, "BezierC2CurveDeletePointEvent"),
        bezier_id(bezier_id),
        point_id(point_id) {}

  mge::EntityId bezier_id;
  mge::EntityId point_id;
};

#endif  // MCAD_EVENTS_BEZIER_C2_CURVE_EVENT_H
