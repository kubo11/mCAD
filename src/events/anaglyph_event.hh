#ifndef MCAD_EVENTS_ANAGLYPH_EVENT
#define MCAD_EVENTS_ANAGLYPH_EVENT

#include "mge.hh"

enum class AnaglyphEvents { UpdateState, UpdateEyeDistance, UpdateProjectivePlaneDistance, UpdateScreenDistance };

class AnaglyphUpdateStateEvent : public mge::Event<AnaglyphEvents> {
 public:
  AnaglyphUpdateStateEvent(bool state)
      : mge::Event<AnaglyphEvents>(AnaglyphEvents::UpdateState, "AnaglyphUpdateStateEvent"), state(state) {}

  bool state;
};

class AnaglyphUpdateEyeDistanceEvent : public mge::Event<AnaglyphEvents> {
 public:
  AnaglyphUpdateEyeDistanceEvent(float distance)
      : mge::Event<AnaglyphEvents>(AnaglyphEvents::UpdateEyeDistance, "AnaglyphUpdateEyeDistanceEvent"),
        distance(distance) {}

  float distance;
};

class AnaglyphUpdateProjectivePlaneDistanceEvent : public mge::Event<AnaglyphEvents> {
 public:
  AnaglyphUpdateProjectivePlaneDistanceEvent(float distance)
      : mge::Event<AnaglyphEvents>(AnaglyphEvents::UpdateProjectivePlaneDistance,
                                   "AnaglyphUpdateProjectivePlaneDistanceEvent"),
        distance(distance) {}

  float distance;
};

class AnaglyphUpdateScreenDistanceEvent : public mge::Event<AnaglyphEvents> {
 public:
  AnaglyphUpdateScreenDistanceEvent(float distance)
      : mge::Event<AnaglyphEvents>(AnaglyphEvents::UpdateScreenDistance, "AnaglyphUpdateScreenDistanceEvent"),
        distance(distance) {}

  float distance;
};

#endif  // MCAD_EVENTS_ANAGLYPH_EVENT