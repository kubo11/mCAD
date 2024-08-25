#ifndef MCAD_EVENTS_BEZIER_CURVE_C2_INTERP_EVENT
#define MCAD_EVENTS_BEZIER_CURVE_C2_INTERP_EVENT

#include "mge.hh"

enum class BezierCurveC2InterpEvents { Add, UpdatePolygonState, AddPoint, DeletePoint };

class AddBezierCurveC2InterpEvent : public mge::Event<BezierCurveC2InterpEvents> {
 public:
  AddBezierCurveC2InterpEvent(std::vector<mge::EntityId> control_points)
      : mge::Event<BezierCurveC2InterpEvents>(BezierCurveC2InterpEvents::Add, "AddBezierCurveC2InterpEvent"),
        control_points{std::move(control_points)} {}

  std::vector<mge::EntityId> control_points;
};

class BezierCurveC2InterpUpdatePolygonStateEvent : public mge::Event<BezierCurveC2InterpEvents> {
 public:
  BezierCurveC2InterpUpdatePolygonStateEvent(mge::EntityId id, bool state)
      : mge::Event<BezierCurveC2InterpEvents>(BezierCurveC2InterpEvents::UpdatePolygonState,
                                              "BezierCurveC2InterpUpdatePolygonStateEvent"),
        id(id),
        state(state) {}

  mge::EntityId id;
  bool state;
};

class BezierCurveC2InterpAddPointEvent : public mge::Event<BezierCurveC2InterpEvents> {
 public:
  BezierCurveC2InterpAddPointEvent(mge::EntityId bezier_id, mge::EntityId point_id)
      : mge::Event<BezierCurveC2InterpEvents>(BezierCurveC2InterpEvents::AddPoint, "BezierCurveC2InterpAddPointEvent"),
        bezier_id(bezier_id),
        point_id(point_id) {}

  mge::EntityId bezier_id;
  mge::EntityId point_id;
};

class BezierCurveC2InterpDeletePointEvent : public mge::Event<BezierCurveC2InterpEvents> {
 public:
  BezierCurveC2InterpDeletePointEvent(mge::EntityId bezier_id, mge::EntityId point_id)
      : mge::Event<BezierCurveC2InterpEvents>(BezierCurveC2InterpEvents::DeletePoint,
                                              "BezierCurveC2InterpDeletePointEvent"),
        bezier_id(bezier_id),
        point_id(point_id) {}

  mge::EntityId bezier_id;
  mge::EntityId point_id;
};

#endif  // MCAD_EVENTS_BEZIER_CURVE_C2_INTERP_EVENT
