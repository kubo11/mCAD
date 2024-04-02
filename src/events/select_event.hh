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

class QuerySelectedEntityEvent : public mge::Event {
 public:
  QuerySelectedEntityEvent() : m_entity{} {}
  virtual inline const std::string name() const override {
    return "QuerySelectedEntityEvent";
  }
  inline std::optional<std::reference_wrapper<mge::Entity>> get_entity() const {
    return m_entity;
  }
  inline void set_entity(
      std::optional<std::reference_wrapper<mge::Entity>> entity) {
    m_entity = entity;
  }

 private:
  std::optional<std::reference_wrapper<mge::Entity>> m_entity;
};

#endif  // MCAD_EVENTS_SELECT_EVENT_HH