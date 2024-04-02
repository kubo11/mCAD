#ifndef MCAD_EVENTS_DELETE_EVENT_HH
#define MCAD_EVENTS_DELETE_EVENT_HH

#include "mge.hh"

class DeleteEntityByTagEvent : public mge::Event {
 public:
  DeleteEntityByTagEvent(const std::string& tag) : m_tag(tag) {}
  virtual inline const std::string name() const override {
    return "DeleteEntityByTagEvent";
  }
  inline const std::string& get_tag() const { return m_tag; }

 protected:
  const std::string& m_tag;
};

class DeletePositionEvent : public mge::Event {
 public:
  DeletePositionEvent(glm::vec2 position) : m_position(position) {}
  virtual inline const std::string name() const override {
    return "DeletePositionEvent";
  }
  inline glm::vec2 get_position() const { return m_position; }

 protected:
  glm::vec2 m_position;
};

#endif  // MCAD_EVENTS_DELETE_EVENT_HH