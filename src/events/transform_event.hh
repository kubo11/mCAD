#ifndef MCAD_EVENTS_TRANSFORM_EVENT
#define MCAD_EVENTS_TRANSFORM_EVENT

#include "mge.hh"

enum class TransformEvents { TranslateToCursor, Translate, RelativeScale, Scale, RelativeRotate, Rotate };

class TranslateToCursorEvent : public mge::Event<TransformEvents> {
 public:
  TranslateToCursorEvent(const std::vector<mge::EntityId>& ids)
      : mge::Event<TransformEvents>(TransformEvents::TranslateToCursor, "TranslateToCursorEvent"), ids(ids) {}

  std::vector<mge::EntityId> ids;
};

class TranslateEvent : public mge::Event<TransformEvents> {
 public:
  TranslateEvent(mge::EntityId id, glm::vec3 destination)
      : mge::Event<TransformEvents>(TransformEvents::Translate, "TranslateEvent"), id(id), destination(destination) {}

  mge::EntityId id;
  glm::vec3 destination;
};

class RelativeScaleEvent : public mge::Event<TransformEvents> {
 public:
  RelativeScaleEvent(const std::vector<mge::EntityId>& ids, glm::vec2 scaling_begin, glm::vec2 scaling_end)
      : mge::Event<TransformEvents>(TransformEvents::RelativeScale, "RelativeScaleEvent"),
        ids(ids),
        scaling_begin(scaling_begin),
        scaling_end(scaling_end) {}

  std::vector<mge::EntityId> ids;
  glm::vec2 scaling_begin;
  glm::vec2 scaling_end;
};

class ScaleEvent : public mge::Event<TransformEvents> {
 public:
  ScaleEvent(mge::EntityId id, glm::vec3 scale)
      : mge::Event<TransformEvents>(TransformEvents::Scale, "ScaleEvent"), id(id), scale(scale) {}

  mge::EntityId id;
  glm::vec3 scale;
};

class RelativeRotateEvent : public mge::Event<TransformEvents> {
 public:
  RelativeRotateEvent(const std::vector<mge::EntityId>& ids, glm::vec2 rotation_begin, glm::vec2 rotation_end,
                      glm::vec3 axis)
      : mge::Event<TransformEvents>(TransformEvents::RelativeRotate, "RelativeRotateEvent"),
        ids(ids),
        rotation_begin(rotation_begin),
        rotation_end(rotation_end),
        axis(axis) {}

  std::vector<mge::EntityId> ids;
  glm::vec2 rotation_begin;
  glm::vec2 rotation_end;
  glm::vec3 axis;
};

class RotateEvent : public mge::Event<TransformEvents> {
 public:
  RotateEvent(mge::EntityId id, glm::quat rotation)
      : mge::Event<TransformEvents>(TransformEvents::Rotate, "RotateEvent"), id(id), rotation(rotation) {}

  mge::EntityId id;
  glm::quat rotation;
};

#endif  // MCAD_EVENTS_TRANSFORM_EVENT