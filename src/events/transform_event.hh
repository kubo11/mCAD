#ifndef MCAD_EVENTS_TRANSFORM_EVENT_HH
#define MCAD_EVENTS_TRANSFORM_EVENT_HH

#include "mge.hh"

class MoveEvent : public mge::Event {
 public:
  MoveEvent() {}
  virtual inline const std::string name() const override { return "MoveEvent"; }
  inline const std::vector<std::reference_wrapper<mge::Entity>>& get_selected()
      const {
    return m_selected;
  }
  inline void set_selected(
      const std::vector<std::reference_wrapper<mge::Entity>>& selected) {
    m_selected = selected;
  }

 protected:
  std::vector<std::reference_wrapper<mge::Entity>> m_selected;
};

class ScaleEvent : public mge::Event {
 public:
  ScaleEvent(glm::vec2 beg, glm::vec2 end) : m_beg(beg), m_end(end) {}
  virtual inline const std::string name() const override {
    return "ScaleEvent";
  }
  inline glm::vec2 get_beg() const { return m_beg; }
  inline glm::vec2 get_end() const { return m_end; }
  inline const std::vector<std::reference_wrapper<mge::Entity>>& get_selected()
      const {
    return m_selected;
  }
  inline void set_selected(
      const std::vector<std::reference_wrapper<mge::Entity>>& selected) {
    m_selected = selected;
  }

 protected:
  glm::vec2 m_beg;
  glm::vec2 m_end;
  std::vector<std::reference_wrapper<mge::Entity>> m_selected;
};

class RotateEvent : public mge::Event {
 public:
  RotateEvent(glm::vec2 beg, glm::vec2 end, glm::vec3 axis)
      : m_beg(beg), m_end(end), m_axis(axis) {}
  virtual inline const std::string name() const override {
    return "RotateEvent";
  }
  inline glm::vec2 get_beg() const { return m_beg; }
  inline glm::vec2 get_end() const { return m_end; }
  inline glm::vec3 get_axis() const { return m_axis; }
  inline const std::vector<std::reference_wrapper<mge::Entity>>& get_selected()
      const {
    return m_selected;
  }
  inline void set_selected(
      const std::vector<std::reference_wrapper<mge::Entity>>& selected) {
    m_selected = selected;
  }

 protected:
  glm::vec2 m_beg;
  glm::vec2 m_end;
  glm::vec3 m_axis;
  std::vector<std::reference_wrapper<mge::Entity>> m_selected;
};

#endif  // MCAD_EVENTS_TRANSFORM_EVENT_HH