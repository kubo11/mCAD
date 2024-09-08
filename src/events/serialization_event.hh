#ifndef MCAD_EVENTS_DESERIALIZATION_EVENT
#define MCAD_EVENTS_DESERIALIZATION_EVENT

#include "mge.hh"

#include "../components/bezier_surface_component.hh"

enum class SerializationEvents {
  DeserializePoints,
  DeserializeTorus,
  DeserializeBezierCurveC0,
  DeserializeBezierCurveC2,
  DeserializeBezierCurveC2Interp,
  DeserializeBezierSurfaceC0,
  DeserializeBezierSurfaceC2,
  AnnounceDeserializedPoints,
  SerializeScene,
  DeserializeScene
};

class DeserializePointsEvent : public mge::Event<SerializationEvents> {
 public:
  DeserializePointsEvent(const std::vector<std::pair<std::string, glm::vec3>>& data)
      : mge::Event<SerializationEvents>(SerializationEvents::DeserializePoints, "DeserializePointsEvent"), data(data) {}
  std::vector<std::pair<std::string, glm::vec3>> data;
  std::vector<mge::EntityId> ids;
};

class DeserializeTorusEvent : public mge::Event<SerializationEvents> {
 public:
  DeserializeTorusEvent(const std::string& name, float inner_radius, float outer_radius, unsigned int inner_density,
                        unsigned int outer_density, const glm::vec3& position, const glm::quat& rotation,
                        const glm::vec3& scale)
      : mge::Event<SerializationEvents>(SerializationEvents::DeserializeTorus, "DeserializeTorusEvent"),
        name(name),
        inner_density(inner_density),
        outer_density(outer_density),
        inner_radius(inner_radius),
        outer_radius(outer_radius),
        position(position),
        rotation(rotation),
        scale(scale) {}
  std::string name;
  float inner_radius;
  float outer_radius;
  unsigned int inner_density;
  unsigned int outer_density;
  glm::vec3 position;
  glm::quat rotation;
  glm::vec3 scale;
};

class DeserializeBezierCurveC0Event : public mge::Event<SerializationEvents> {
 public:
  DeserializeBezierCurveC0Event(const std::string& name, const std::vector<mge::EntityId>& points)
      : mge::Event<SerializationEvents>(SerializationEvents::DeserializeBezierCurveC0, "DeserializeBezierCurveC0Event"),
        name(name),
        points(points) {}
  std::string name;
  std::vector<mge::EntityId> points;
};

class DeserializeBezierCurveC2Event : public mge::Event<SerializationEvents> {
 public:
  DeserializeBezierCurveC2Event(const std::string& name, const std::vector<mge::EntityId>& points)
      : mge::Event<SerializationEvents>(SerializationEvents::DeserializeBezierCurveC2, "DeserializeBezierCurveC2Event"),
        name(name),
        points(points) {}
  std::string name;
  std::vector<mge::EntityId> points;
};

class DeserializeBezierCurveC2InterpEvent : public mge::Event<SerializationEvents> {
 public:
  DeserializeBezierCurveC2InterpEvent(const std::string& name, const std::vector<mge::EntityId>& points)
      : mge::Event<SerializationEvents>(SerializationEvents::DeserializeBezierCurveC2Interp,
                                        "DeserializeBezierCurveC2InterpEvent"),
        name(name),
        points(points) {}
  std::string name;
  std::vector<mge::EntityId> points;
};

class DeserializeBezierSurfaceC0Event : public mge::Event<SerializationEvents> {
 public:
  DeserializeBezierSurfaceC0Event(const std::string& name, const std::vector<std::vector<mge::EntityId>>& points,
                                  unsigned int patch_count_u, unsigned int patch_count_v, unsigned int line_count,
                                  BezierSurfaceWrapping wrapping)
      : mge::Event<SerializationEvents>(SerializationEvents::DeserializeBezierSurfaceC0,
                                        "DeserializeBezierSurfaceC0Event"),
        name(name),
        points(points),
        patch_count_u(patch_count_u),
        patch_count_v(patch_count_v),
        line_count(line_count),
        wrapping(wrapping) {}
  std::string name;
  std::vector<std::vector<mge::EntityId>> points;
  unsigned int patch_count_u;
  unsigned int patch_count_v;
  unsigned int line_count;
  BezierSurfaceWrapping wrapping;
};

class DeserializeBezierSurfaceC2Event : public mge::Event<SerializationEvents> {
 public:
  DeserializeBezierSurfaceC2Event(const std::string& name, const std::vector<std::vector<mge::EntityId>>& points,
                                  unsigned int patch_count_u, unsigned int patch_count_v, unsigned int line_count,
                                  BezierSurfaceWrapping wrapping)
      : mge::Event<SerializationEvents>(SerializationEvents::DeserializeBezierSurfaceC2,
                                        "DeserializeBezierSurfaceC2Event"),
        name(name),
        points(points),
        patch_count_u(patch_count_u),
        patch_count_v(patch_count_v),
        line_count(line_count),
        wrapping(wrapping) {}
  std::string name;
  std::vector<std::vector<mge::EntityId>> points;
  unsigned int patch_count_u;
  unsigned int patch_count_v;
  unsigned int line_count;
  BezierSurfaceWrapping wrapping;
};

class AnnounceDeserializedPointsEvent : public mge::Event<SerializationEvents> {
 public:
  AnnounceDeserializedPointsEvent(const std::vector<std::pair<mge::EntityId, std::string>>& data)
      : mge::Event<SerializationEvents>(SerializationEvents::AnnounceDeserializedPoints,
                                        "DeserializeBezierSurfaceC2Event"),
        data(data) {}

  std::vector<std::pair<mge::EntityId, std::string>> data;
};

class SerializeSceneEvent : public mge::Event<SerializationEvents> {
 public:
  SerializeSceneEvent(const fs::path& path)
      : mge::Event<SerializationEvents>(SerializationEvents::SerializeScene, "DeserializeBezierSurfaceC2Event"),
        path(path) {}

  fs::path path;
};

class DeserializeSceneEvent : public mge::Event<SerializationEvents> {
 public:
  DeserializeSceneEvent(const fs::path& path)
      : mge::Event<SerializationEvents>(SerializationEvents::DeserializeScene, "DeserializeSceneEvent"), path(path) {}

  fs::path path;
};

#endif  // MCAD_EVENTS_DESERIALIZATION_EVENT