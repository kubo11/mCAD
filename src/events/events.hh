#ifndef MCAD_EVENTS
#define MCAD_EVENTS

#include "bezier_curve_c0_event.hh"
#include "bezier_curve_c2_event.hh"
#include "cursor_event.hh"
#include "input_state_event.hh"
#include "point_event.hh"
#include "select_event.hh"
#include "torus_event.hh"
#include "transform_event.hh"

DeclareClientEventManager(BezierCurveC0Events, BezierCurveC2Events, CursorEvents, InputStateEvents, PointEvents,
                          SelectionEvents, TorusEvents, TransformEvents);

#endif  // MCAD_EVENTS