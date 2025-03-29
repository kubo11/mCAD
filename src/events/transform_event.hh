#ifndef MCAD_EVENTS_TRANSFORM_EVENT
#define MCAD_EVENTS_TRANSFORM_EVENT

#include "mge.hh"

enum class TransformEvents { TranslateToCursor, RelativeTranslate, Translate, RelativeScale, Scale, RelativeRotate, Rotate };

class TranslateToCursorEvent : public mge::Event<TransformEvents> {
  public:
  TranslateToCursorEvent(const std::vector<mge::EntityId>& ids)
       : mge::Event<TransformEvents>(TransformEvents::TranslateToCursor, "TranslateToCursorEvent"), ids(ids) {}
 
   std::vector<mge::EntityId> ids;
 };

class RelativeTranslateEvent : public mge::Event<TransformEvents> {
 public:
 RelativeTranslateEvent(const std::vector<mge::EntityId>& ids, const glm::vec3& translation)
      : mge::Event<TransformEvents>(TransformEvents::RelativeTranslate, "RelativeTranslateEvent"), ids(ids), translation(translation) {}

  std::vector<mge::EntityId> ids;
  glm::vec3 translation;
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
  RelativeScaleEvent(const std::vector<mge::EntityId>& ids, const glm::vec3& scaling)
      : mge::Event<TransformEvents>(TransformEvents::RelativeScale, "RelativeScaleEvent"),
        ids(ids),
        scaling(scaling) {}

  std::vector<mge::EntityId> ids;
  glm::vec3 scaling;
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
  RelativeRotateEvent(const std::vector<mge::EntityId>& ids, const glm::quat& quat)
      : mge::Event<TransformEvents>(TransformEvents::RelativeRotate, "RelativeRotateEvent"),
        ids(ids),
        quat(quat) {}

  std::vector<mge::EntityId> ids;
  glm::quat quat;
};

class RotateEvent : public mge::Event<TransformEvents> {
 public:
  RotateEvent(mge::EntityId id, glm::quat rotation)
      : mge::Event<TransformEvents>(TransformEvents::Rotate, "RotateEvent"), id(id), rotation(rotation) {}

  mge::EntityId id;
  glm::quat rotation;
};

#endif  // MCAD_EVENTS_TRANSFORM_EVENT