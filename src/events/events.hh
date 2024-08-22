#ifndef MCAD_EVENTS
#define MCAD_EVENTS

#include "bezier_c0_curve_event.hh"
#include "bezier_c2_curve_event.hh"
#include "cursor_event.hh"
#include "input_state_event.hh"
#include "point_event.hh"
#include "select_event.hh"
#include "torus_event.hh"
#include "transform_event.hh"

DeclareClientEventManager(BezierC0CurveEvents, BezierC2CurveEvents, CursorEvents, InputStateEvents, PointEvents,
                          SelectionEvents, TorusEvents, TransformEvents);

#endif  // MCAD_EVENTS