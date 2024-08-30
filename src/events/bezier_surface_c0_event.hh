#ifndef MCAD_EVENTS_BEZIER_SURFACE_C0_EVENT
#define MCAD_EVENTS_BEZIER_SURFACE_C0_EVENT

#include "mge.hh"

#include "../components/bezier_surface_component.hh"

enum class BezierSurfaceC0Events { Add, UpdateGridState, UpdateLineCount };

class AddBezierSurfaceC0Event : public mge::Event<BezierSurfaceC0Events> {
 public:
  AddBezierSurfaceC0Event(float patch_count_u, float patch_count_v, float size_u, float size_v,
                          BezierSurfaceWrapping wrapping)
      : mge::Event<BezierSurfaceC0Events>(BezierSurfaceC0Events::Add, "AddBezierSurfaceC0Event"),
        patch_count_u(patch_count_u),
        patch_count_v(patch_count_v),
        size_u(size_u),
        size_v(size_v),
        wrapping(wrapping) {}

  float patch_count_u;
  float patch_count_v;
  float size_u;
  float size_v;
  BezierSurfaceWrapping wrapping;
};

class BezierSurfaceC0UpdateGridStateEvent : public mge::Event<BezierSurfaceC0Events> {
 public:
  BezierSurfaceC0UpdateGridStateEvent(mge::EntityId id, bool state)
      : mge::Event<BezierSurfaceC0Events>(BezierSurfaceC0Events::UpdateGridState,
                                          "BezierSurfaceC0UpdateGridStateEvent"),
        id(id),
        state(state) {}

  mge::EntityId id;
  bool state;
};

class BezierSurfaceC0UpdateLineCountEvent : public mge::Event<BezierSurfaceC0Events> {
 public:
  BezierSurfaceC0UpdateLineCountEvent(mge::EntityId id, unsigned int line_count)
      : mge::Event<BezierSurfaceC0Events>(BezierSurfaceC0Events::UpdateLineCount,
                                          "BezierSurfaceC0UpdateLineCountEvent"),
        id(id),
        line_count(line_count) {}

  mge::EntityId id;
  unsigned int line_count;
};

#endif  // MCAD_EVENTS_BEZIER_SURFACE_C0_EVENT
