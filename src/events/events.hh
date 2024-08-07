#ifndef MCAD_EVENTS
#define MCAD_EVENTS

#include "bezier_event.hh"
#include "cursor_event.hh"
#include "input_state_event.hh"
#include "point_event.hh"
#include "select_event.hh"
#include "torus_event.hh"
#include "transform_event.hh"

DeclareClientEventManager(BezierEvents, CursorEvents, InputStateEvents, PointEvents, SelectionEvents, TorusEvents,
                          TransformEvents);

#endif  // MCAD_EVENTS