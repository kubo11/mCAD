#ifndef MCAD_EVENTS_POINT_EVENT
#define MCAD_EVENTS_POINT_EVENT

#include "mge.hh"

enum class PointEvents { Add };

class AddPointEvent : public mge::Event<PointEvents> {
 public:
  AddPointEvent() : mge::Event<PointEvents>(PointEvents::Add, "AddPointEvent") {}
};

#endif  // MCAD_EVENTS_POINT_EVENT