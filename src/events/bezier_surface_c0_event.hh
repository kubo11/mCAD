#ifndef MCAD_EVENTS_BEZIER_SURFACE_C0_EVENT
#define MCAD_EVENTS_BEZIER_SURFACE_C0_EVENT

#include "mge.hh"

#include "../components/bezier_surface_component.hh"

enum class BezierSurfaceC0Events { AddFlat, AddWrapped, UpdateGridState, UpdateLineCount };

class AddFlatBezierSurfaceC0Event : public mge::Event<BezierSurfaceC0Events> {
 public:
  AddFlatBezierSurfaceC0Event(float patch_count_u, float patch_count_v, float size_u, float size_v)
      : mge::Event<BezierSurfaceC0Events>(BezierSurfaceC0Events::AddFlat, "AddFlatBezierSurfaceC0Event"),
        patch_count_u(patch_count_u),
        patch_count_v(patch_count_v),
        size_u(size_u),
        size_v(size_v) {}

  float patch_count_u;
  float patch_count_v;
  float size_u;
  float size_v;
};

class AddWrappedBezierSurfaceC0Event : public mge::Event<BezierSurfaceC0Events> {
 public:
  AddWrappedBezierSurfaceC0Event(float patch_count_u, float patch_count_v, float height, float radius,
                                 BezierSurfaceWrapping wrapping)
      : mge::Event<BezierSurfaceC0Events>(BezierSurfaceC0Events::AddWrapped, "AddWrappedBezierSurfaceC0Event"),
        patch_count_u(patch_count_u),
        patch_count_v(patch_count_v),
        height(height),
        radius(radius),
        wrapping(wrapping) {}

  float patch_count_u;
  float patch_count_v;
  float height;
  float radius;
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
