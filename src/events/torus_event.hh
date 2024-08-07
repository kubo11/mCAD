#ifndef MCAD_EVENTS_TORUS_EVENT
#define MCAD_EVENTS_TORUS_EVENT

#include "mge.hh"

enum class TorusEvents { Add, RadiusUpdated, GridDensityUpdated };

class AddTorusEvent : public mge::Event<TorusEvents> {
 public:
  AddTorusEvent(float inner_radius, float outer_radius, unsigned int inner_density, unsigned int outer_density)
      : mge::Event<TorusEvents>(TorusEvents::Add, "AddTorusEvent"),
        inner_radius(inner_radius),
        outer_radius(outer_radius),
        inner_density(inner_density),
        outer_density(outer_density) {}

  float inner_radius;
  float outer_radius;
  unsigned int inner_density;
  unsigned int outer_density;
};

class TorusRadiusUpdatedEvent : public mge::Event<TorusEvents> {
 public:
  TorusRadiusUpdatedEvent(mge::EntityId id, float inner_radius, float outer_radius)
      : mge::Event<TorusEvents>(TorusEvents::RadiusUpdated, "TorusRadiusUpdatedEvent"),
        id(id),
        inner_radius(inner_radius),
        outer_radius(outer_radius) {}

  mge::EntityId id;
  float inner_radius;
  float outer_radius;
};

class TorusGridDensityUpdatedEvent : public mge::Event<TorusEvents> {
 public:
  TorusGridDensityUpdatedEvent(mge::EntityId id, unsigned int inner_density, unsigned int outer_density)
      : mge::Event<TorusEvents>(TorusEvents::GridDensityUpdated, "TorusGridDensityUpdatedEvent"),
        id(id),
        inner_density(inner_density),
        outer_density(outer_density) {}

  mge::EntityId id;
  unsigned int inner_density;
  unsigned int outer_density;
};

#endif  // MCAD_EVENTS_TORUS_EVENT