#ifndef MCAD_EVENTS_CURSOR_EVENT_HH
#define MCAD_EVENTS_CURSOR_EVENT_HH

#include "mge.hh"

class CursorMoveEvent : public mge::Event {
 public:
  CursorMoveEvent(glm::vec2 position) : m_position(position) {}
  virtual inline const std::string name() const override {
    return "CursorMoveEvent";
  }
  inline glm::vec2 get_position() const { return m_position; }

 protected:
  glm::vec2 m_position;
};

#endif  // MCAD_EVENTS_CURSOR_EVENT_HH