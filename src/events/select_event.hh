#ifndef MCAD_EVENTS_SELECT_EVENT_HH
#define MCAD_EVENTS_SELECT_EVENT_HH

#include "mge.hh"

class SelectEntityByTagEvent : public mge::Event {
 public:
  SelectEntityByTagEvent(const std::string& tag) : m_tag(tag) {}
  virtual inline const std::string name() const override {
    return "SelectEntityByTagEvent";
  }
  inline const std::string& get_tag() const { return m_tag; }

 protected:
  const std::string& m_tag;
};

class UnSelectEntityByTagEvent : public mge::Event {
 public:
  UnSelectEntityByTagEvent(const std::string& tag) : m_tag(tag) {}
  virtual inline const std::string name() const override {
    return "UnSelectEntityByTagEvent";
  }
  inline const std::string& get_tag() const { return m_tag; }

 protected:
  const std::string& m_tag;
};

class UnSelectAllEntitiesEvent : public mge::Event {
 public:
  UnSelectAllEntitiesEvent() {}
  virtual inline const std::string name() const override {
    return "UnSelectAllEntitiesEvent";
  }
};

class SelectEntityByPositionEvent : public mge::Event {
 public:
  SelectEntityByPositionEvent(glm::vec2 position) : m_position(position) {}
  virtual inline const std::string name() const override {
    return "SelectEntityByPositionEvent";
  }
  inline glm::vec2 get_position() const { return m_position; }

 private:
  glm::vec2 m_position;
};

class QueryEntityByTagEvent : public mge::Event {
 public:
  QueryEntityByTagEvent(const std::string& tag)
      : m_tag(tag), m_entity(std::nullopt) {}
  virtual inline const std::string name() const override {
    return "QueryEntityByTagEvent";
  }
  inline const std::string& get_tag() const { return m_tag; }
  inline void set_entity(mge::Entity& entity) { m_entity = entity; }
  inline mge::OptionalEntity& get_entity() { return m_entity; }

 private:
  std::string m_tag;
  mge::OptionalEntity m_entity;
};

class UpdateDisplayedEntityEvent : public mge::Event {
 public:
  UpdateDisplayedEntityEvent(const mge::OptionalEntity& entity)
      : m_entity(entity) {}
  virtual inline const std::string name() const override {
    return "UpdateDisplayedEntityEvent";
  }
  inline mge::OptionalEntity& get_entity() { return m_entity; }

 private:
  mge::OptionalEntity m_entity;
};

#endif  // MCAD_EVENTS_SELECT_EVENT_HH