#ifndef MCAD_EVENTS_SELECT_EVENT_HH
#define MCAD_EVENTS_SELECT_EVENT_HH

#include "mge.hh"

class SelectEvent : public mge::Event {
 public:
  SelectEvent(std::optional<std::reference_wrapper<mge::Entity>> entity)
      : m_selected(entity) {}
  virtual inline const std::string name() const override {
    return "SelectEvent";
  }
  inline std::optional<std::reference_wrapper<mge::Entity>> get_selected()
      const {
    return m_selected;
  }

 protected:
  std::optional<std::reference_wrapper<mge::Entity>> m_selected;
};

class QuerySelectedTagEvent : public mge::Event {
 public:
  QuerySelectedTagEvent(const std::string& tag) : m_tag(tag) {}
  virtual inline const std::string name() const override {
    return "QuerySelectedTagEvent";
  }
  inline const std::string& get_tag() const { return m_tag; }

 protected:
  const std::string& m_tag;
};

class QuerySelectedCursorEvent : public mge::Event {
 public:
  QuerySelectedCursorEvent(glm::vec2 position) : m_position(position) {}
  virtual inline const std::string name() const override {
    return "QuerySelectedCursorEvent";
  }
  inline glm::vec2 get_position() const { return m_position; }

 private:
  glm::vec2 m_position;
};

#endif  // MCAD_EVENTS_SELECT_EVENT_HH