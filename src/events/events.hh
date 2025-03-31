#ifndef MCAD_EVENTS
#define MCAD_EVENTS

#include "anaglyph_event.hh"
#include "bezier_curve_c0_event.hh"
#include "bezier_curve_c2_event.hh"
#include "bezier_curve_c2_interp_event.hh"
#include "bezier_surface_c0_event.hh"
#include "bezier_surface_c2_event.hh"
#include "cursor_event.hh"
#include "gregory_patch.hh"
#include "point_event.hh"
#include "select_event.hh"
#include "serialization_event.hh"
#include "torus_event.hh"
#include "transform_event.hh"
#include "ui_event.hh"

DeclareClientEventManager(AnaglyphEvents, BezierCurveC0Events, BezierCurveC2Events, BezierCurveC2InterpEvents,
                          BezierSurfaceC0Events, BezierSurfaceC2Events, CursorEvents, GregoryPatchEvents,
                          SerializationEvents, PointEvents, SelectionEvents, TorusEvents, TransformEvents, UIEvents);

#endif  // MCAD_EVENTS