#ifndef MCAD_EVENTS_BEZIER_SURFACE_C2_EVENT
#define MCAD_EVENTS_BEZIER_SURFACE_C2_EVENT

#include "mge.hh"

#include "../components/bezier_surface_component.hh"

enum class BezierSurfaceC2Events { Add, UpdateGridState, UpdateLineCount };

class AddBezierSurfaceC2Event : public mge::Event<BezierSurfaceC2Events> {
 public:
  AddBezierSurfaceC2Event(float patch_count_u, float patch_count_v, float size_u, float size_v,
                          BezierSurfaceWrapping wrapping)
      : mge::Event<BezierSurfaceC2Events>(BezierSurfaceC2Events::Add, "AddBezierSurfaceC2Event"),
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

class BezierSurfaceC2UpdateGridStateEvent : public mge::Event<BezierSurfaceC2Events> {
 public:
  BezierSurfaceC2UpdateGridStateEvent(mge::EntityId id, bool state)
      : mge::Event<BezierSurfaceC2Events>(BezierSurfaceC2Events::UpdateGridState,
                                          "BezierSurfaceC2UpdateGridStateEvent"),
        id(id),
        state(state) {}

  mge::EntityId id;
  bool state;
};

class BezierSurfaceC2UpdateLineCountEvent : public mge::Event<BezierSurfaceC2Events> {
 public:
  BezierSurfaceC2UpdateLineCountEvent(mge::EntityId id, unsigned int line_count)
      : mge::Event<BezierSurfaceC2Events>(BezierSurfaceC2Events::UpdateLineCount,
                                          "BezierSurfaceC2UpdateLineCountEvent"),
        id(id),
        line_count(line_count) {}

  mge::EntityId id;
  unsigned int line_count;
};

#endif  // MCAD_EVENTS_BEZIER_SURFACE_C2_EVENT
