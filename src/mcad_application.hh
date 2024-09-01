#ifndef MCAD_APPLICATION_HH
#define MCAD_APPLICATION_HH

#include "mge.hh"

#include "events/events.hh"

class MCadApplication : public mge::Application {
 public:
  MCadApplication();

 protected:
  std::shared_ptr<EventManager> m_event_manager = nullptr;

 private:
  bool on_anaglyph_update_state(AnaglyphUpdateStateEvent& event);
  bool on_anaglyph_update_eye_distance(AnaglyphUpdateEyeDistanceEvent& event);
  bool on_anaglyph_update_projective_plane_distance(AnaglyphUpdateProjectivePlaneDistanceEvent& event);
  bool on_anaglyph_update_screen_distance(AnaglyphUpdateScreenDistanceEvent& event);
};

#endif  // MCAD_APPLICATION_HH