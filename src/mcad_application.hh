#ifndef MCAD_APPLICATION_HH
#define MCAD_APPLICATION_HH

#include "mge.hh"

#include "events/events.hh"
#include "serializer/scene_serializer.hh"

class MCadApplication : public mge::Application {
 public:
  MCadApplication();

 protected:
  std::shared_ptr<EventManager> m_event_manager = nullptr;
  SceneSerializer m_serializer;

 private:
  bool on_anaglyph_update_state(AnaglyphUpdateStateEvent& event);
  bool on_anaglyph_update_eye_distance(AnaglyphUpdateEyeDistanceEvent& event);
  bool on_anaglyph_update_projective_plane_distance(AnaglyphUpdateProjectivePlaneDistanceEvent& event);
  bool on_anaglyph_update_screen_distance(AnaglyphUpdateScreenDistanceEvent& event);

  bool on_deserialize_points(DeserializePointsEvent& event);
  bool on_deserialize_torus(DeserializeTorusEvent& event);
  bool on_deserialize_bezier_curve_c0(DeserializeBezierCurveC0Event& event);
  bool on_deserialize_bezier_curve_c2(DeserializeBezierCurveC2Event& event);
  bool on_deserialize_bezier_curve_c2_interp(DeserializeBezierCurveC2InterpEvent& event);
  bool on_deserialize_bezier_surface_c0(DeserializeBezierSurfaceC0Event& event);
  bool on_deserialize_bezier_surface_c2(DeserializeBezierSurfaceC2Event& event);
  bool on_serialize_scene(SerializeSceneEvent& event);
  bool on_deserialize_scene(DeserializeSceneEvent& event);
};

#endif  // MCAD_APPLICATION_HH