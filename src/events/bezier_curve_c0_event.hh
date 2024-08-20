#ifndef MCAD_EVENTS_BEZIER_CURVE_C0_EVENT
#define MCAD_EVENTS_BEZIER_CURVE_C0_EVENT

#include "mge.hh"

enum class BezierCurveC0Events { Add, UpdatePolygonState, AddPoint, DeletePoint };

class AddBezierCurveC0Event : public mge::Event<BezierCurveC0Events> {
 public:
  AddBezierCurveC0Event(std::vector<mge::EntityId> control_points)
      : mge::Event<BezierCurveC0Events>(BezierCurveC0Events::Add, "AddBezierCurveC0Event"),
        control_points{std::move(control_points)} {}

  std::vector<mge::EntityId> control_points;
};

class BezierCurveC0UpdatePolygonStateEvent : public mge::Event<BezierCurveC0Events> {
 public:
  BezierCurveC0UpdatePolygonStateEvent(mge::EntityId id, bool state)
      : mge::Event<BezierCurveC0Events>(BezierCurveC0Events::UpdatePolygonState,
                                        "BezierCurveC0UpdatePolygonStateEvent"),
        id(id),
        state(state) {}

  mge::EntityId id;
  bool state;
};

class BezierCurveC0AddPointEvent : public mge::Event<BezierCurveC0Events> {
 public:
  BezierCurveC0AddPointEvent(mge::EntityId bezier_id, mge::EntityId point_id)
      : mge::Event<BezierCurveC0Events>(BezierCurveC0Events::AddPoint, "BezierCurveC0AddPointEvent"),
        bezier_id(bezier_id),
        point_id(point_id) {}

  mge::EntityId bezier_id;
  mge::EntityId point_id;
};

class BezierCurveC0DeletePointEvent : public mge::Event<BezierCurveC0Events> {
 public:
  BezierCurveC0DeletePointEvent(mge::EntityId bezier_id, mge::EntityId point_id)
      : mge::Event<BezierCurveC0Events>(BezierCurveC0Events::DeletePoint, "BezierCurveC0DeletePointEvent"),
        bezier_id(bezier_id),
        point_id(point_id) {}

  mge::EntityId bezier_id;
  mge::EntityId point_id;
};

#endif  // MCAD_EVENTS_BEZIER_CURVE_C0_EVENT
