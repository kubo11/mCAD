#ifndef MCAD_EVENTS_BEZIER_CURVE_C2_EVENT
#define MCAD_EVENTS_BEZIER_CURVE_C2_EVENT

#include "mge.hh"

#include "../components/bezier_curve_component.hh"

enum class BezierCurveC2Events { Add, UpdatePolygonState, UpdateBase, AddPoint, DeletePoint };

class AddBezierCurveC2Event : public mge::Event<BezierCurveC2Events> {
 public:
  AddBezierCurveC2Event(std::vector<mge::EntityId> control_points)
      : mge::Event<BezierCurveC2Events>(BezierCurveC2Events::Add, "AddBezierCurveC2Event"),
        control_points{std::move(control_points)} {}

  std::vector<mge::EntityId> control_points;
};

class BezierCurveC2UpdatePolygonStateEvent : public mge::Event<BezierCurveC2Events> {
 public:
  BezierCurveC2UpdatePolygonStateEvent(mge::EntityId id, bool state)
      : mge::Event<BezierCurveC2Events>(BezierCurveC2Events::UpdatePolygonState,
                                        "BezierCurveC2UpdatePolygonStateEvent"),
        id(id),
        state(state) {}

  mge::EntityId id;
  bool state;
};

class BezierCurveC2UpdateBaseEvent : public mge::Event<BezierCurveC2Events> {
 public:
  BezierCurveC2UpdateBaseEvent(mge::EntityId id, BezierCurveBase base)
      : mge::Event<BezierCurveC2Events>(BezierCurveC2Events::UpdateBase, "BezierCurveC2UpdateBaseEvent"),
        id(id),
        base(base) {}

  mge::EntityId id;
  BezierCurveBase base;
};

class BezierCurveC2AddPointEvent : public mge::Event<BezierCurveC2Events> {
 public:
  BezierCurveC2AddPointEvent(mge::EntityId bezier_id, mge::EntityId point_id)
      : mge::Event<BezierCurveC2Events>(BezierCurveC2Events::AddPoint, "BezierCurveC2AddPointEvent"),
        bezier_id(bezier_id),
        point_id(point_id) {}

  mge::EntityId bezier_id;
  mge::EntityId point_id;
};

class BezierCurveC2DeletePointEvent : public mge::Event<BezierCurveC2Events> {
 public:
  BezierCurveC2DeletePointEvent(mge::EntityId bezier_id, mge::EntityId point_id)
      : mge::Event<BezierCurveC2Events>(BezierCurveC2Events::DeletePoint, "BezierCurveC2DeletePointEvent"),
        bezier_id(bezier_id),
        point_id(point_id) {}

  mge::EntityId bezier_id;
  mge::EntityId point_id;
};

#endif  // MCAD_EVENTS_BEZIER_CURVE_C2_EVENT
